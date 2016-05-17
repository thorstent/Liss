/*
 * Copyright 2016, IST Austria
 *
 * This file is part of Liss.
 *
 * Liss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Liss.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "place_locks.h"

#include <string.h>
#include <algorithm>
#include <sstream>
#include "options.h"
#include "synthesis/z3_helpers.h"
#include <Limi/internal/helpers.h>

using namespace placement;
using namespace std;

namespace placement {
  std::ostream& operator<<(std::ostream& out, cost_type c) {
    switch (c) {
      case cost_type::absolute_minimum:
        return out << "Absolute minimum of instructions inside a lock";
      case cost_type::small_locks:
        return out << "Smallest locks";
      case cost_type::coarse:
        return out << "Coarse locks";
      case cost_type::unoptimized:
        return out << "No cost function";
      case cost_type::max_pairwise_concurrency:
        return out << "Maximum concurrency";
    }
    return out;
  }
  
  std::string short_name(cost_type c) {
    switch (c) {
      case cost_type::absolute_minimum:
        return "absmin";
      case cost_type::small_locks:
        return "small";
      case cost_type::coarse:
        return "coarse";
      case cost_type::unoptimized:
        return "unopt";
      case cost_type::max_pairwise_concurrency:
        return "maxconc";
    }
    return "";
  }
  
  std::ostream& operator<<(std::ostream& out, const lock_statistics& ls) {
    out << "Number of locks used: " << ls.locks << endl;
    out << "Number of lock operations: " << ls.lock_instr << endl;
    out << "Number of unlock operations: " << ls.unlock_instr << endl;
    out << "Number of abstract instructions inside a lock: " << ls.in_lock << endl;    
    return out;
  }
}

z3::expr func_result_impl(const z3::model& model, const z3::func_decl& func_decl, vector<vector<z3::expr>>& result);

z3::expr func_result_expr(const z3::model& model, const z3::expr& expr, vector<vector<z3::expr>>& result) {
  Z3_decl_kind decl_kind = expr.decl().decl_kind();
  switch (decl_kind) {
    case Z3_OP_UNINTERPRETED: {
      return func_result_impl(model, expr.decl(), result);
      //print_expr(model, expr.arg(0));
    } break;
    case Z3_OP_ITE: {
      assert(false);
      z3::expr cond = expr.arg(0);
      z3::expr then = expr.arg(1);
      z3::expr else1 = expr.arg(2);
      return func_result_expr(model, else1, result);
    } break;
    default:
      return expr;
      break;
  }
}


/**
 * @brief Reads the function assignments from the model
 * 
 * @param model The model
 * @param func_decl The original function declaration
 * @param result The arguments and what was assigned to the arguments (last component of the vector)
 * @return The else case
 */
z3::expr func_result_impl(const z3::model& model, const z3::func_decl& func_decl, vector<vector<z3::expr>>& result) {
  //cout << func_decl.name().str() << endl;
  z3::func_interp interp = model.get_func_interp(func_decl);
  for (unsigned i = 0; i < interp.num_entries(); ++i) {
    z3::func_entry e = interp.entry(i);
    result.push_back(vector<z3::expr>());
    for (unsigned j = 0; j < e.num_args(); ++j) {
      result.back().push_back(e.arg(j));
    }
    result.back().push_back(e.value());
  }
  return func_result_expr(model, interp.else_value(), result);
}

/**
 * @brief Reads the function assignments from the model and also delete all assignments that are the same 
 * as the else case.
 * 
 * @param model The model
 * @param func_decl The original function declaration
 * @param result The arguments and what was assigned to the arguments (last component of the vector)
 * @return The else case
 */
z3::expr func_result(const z3::model& model, const z3::func_decl& func_decl, vector<vector<z3::expr>>& result) {
  z3::expr elsee = func_result_impl(model, func_decl, result);
  result.erase(remove_if(result.begin(), result.end(),[elsee](const vector<z3::expr>& item){ return (Z3_ast)item.back()==elsee;}), result.end());
  return elsee;
}

void print_func_interp(const z3::model& model, const z3::func_decl& func_decl) {
  cout << func_decl.name().str() << endl;
  vector<vector<z3::expr>> result;
  z3::expr elsee = func_result(model, func_decl, result);
  for (vector<z3::expr>& e : result) {
    cout << "  (";
    for (unsigned i = 0; i < e.size()-1; ++i) {
      cout << e[i];
      if (i < e.size()-2) cout << ", ";
    }
    cout << ") -> " << e.back() << endl;
  }
  cout << "  else: " << elsee << endl;
}

place_locks::place_locks(const cfg::program& program) : threads(program.threads())
{
  // build position array
  // we assume the threads are compacted (no unreachable states)
  vector<string> positions;
  unsigned i = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned j = 1; j <= thread->no_states(); ++j) { 
      string name = to_string(thread->thread_id) + "_" + to_string(j);
      
      // create location
      if (verbosity >= 2) {
        stringstream ss;
        ss << thread->get_state(j);
        name = ss.str() + "_" + name;
      }
      
      positions.push_back(name);
    }
  }
  
  z3::func_decl_vector consts(ctx);
  z3::func_decl_vector ts(ctx);
  locations = ctx.enumeration_sort("locations", positions, consts, ts);
  
  unsigned index = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    location_vector.push_back(std::vector<z3::expr>());
    location_vector.back().push_back(ctx);
    for (unsigned j = 1; j <= thread->no_states(); ++j) { 
      z3::expr ex = consts[index]();
      location_vector.back().push_back(ex);
      location_map.insert(make_pair(ex, abstraction::location(location_vector.size()-1,j)));
      ++index;
    }
  }
  assert(index == consts.size());
  
   
  // build a cache of instructions and their locations
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      z3::expr x = location_vector[t][i];
      const cfg::state& s = threads[t]->get_state(i);
      if (s.lock_after != clang::SourceLocation())
        lock_after[s.lock_after].push_back(x);
      if (s.lock_before != clang::SourceLocation())
        lock_before[s.lock_before].push_back(x);
    }
    ++t;
  }
}

void place_locks::lock_order(locking_constraints& lc, const z3::expr& lock, z3::expr location, z3::expr predecessor) {
  for (auto it = lc.lock_vector.begin(); (Z3_ast)(*it) != lock; ++it) {
    assert (it != lc.lock_vector.end());
    const z3::expr& lb = *it;
    lc.cons_lockorder = lc.cons_lockorder && implies(lc.lock_b(location,lock), (!lc.inl(predecessor,lb) && !lc.lock_a(predecessor,lb)));
  }
  lc.cons_lockorder = lc.cons_lockorder && implies(lc.lock_b(location,lock), (!lc.inl(predecessor,lock) && !lc.lock_a(predecessor,lock)));
}

void place_locks::lock_order(locking_constraints& lc, const z3::expr& lock, z3::expr location) {
  for (auto it = lc.lock_vector.begin(); (Z3_ast)(*it) != lock; ++it) {
    assert (it != lc.lock_vector.end());
    const z3::expr& lb = *it;
    lc.cons_lockorder = lc.cons_lockorder && implies(lc.lock_a(location,lock), (!lc.inl(location,lb)));
  }
  lc.cons_lockorder = lc.cons_lockorder && implies(lc.lock_a(location,lock), (!lc.inl(location,lock)));
}

void place_locks::lock_consistancy(locking_constraints& lc)
{
  // predecessor rule
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    // make a predecessor map
    vector<unordered_set<state_id_type>> predecessors(thread->no_states()+1);
    vector<unordered_set<state_id_type>> predecessors_forward(thread->no_states()+1); // only the forward predecessors (no back edges)
    
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      const z3::expr& x = location_vector[t][i];
      // forbid locking and unlock at non-locations
      for (z3::expr l : lc.lock_vector) {
        const auto& s = thread->get_state(i);
        if (s.lock_before == clang::SourceLocation()) {
          lc.cons_loc = lc.cons_loc && !lc.lock_b(x,l) && !lc.unlock_b(x,l);
        }
        if (s.lock_after == clang::SourceLocation()) {
          lc.cons_loc = lc.cons_loc && !lc.lock_a(x,l) && !lc.unlock_a(x,l);
        }
        lc.cons_basic = lc.cons_basic && (!lc.lock_a(x,l) || !lc.unlock_a(x,l)) && (!lc.lock_b(x,l) || !lc.unlock_b(x,l));
        if (s.action && s.action->is_unlockable_point()) {
          // not allowed to lock a wait or a notify
          lc.cons_preemption = lc.cons_preemption && !lc.inl(x,l);
        }
      }
      // gather predecessor list
      for (const cfg::edge& e : thread->get_successors(i)) {
        predecessors[e.to].insert(i);
        if (!e.back_edge)
          predecessors_forward[e.to].insert(i);
      }
    }
    // use predecessors
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      z3::expr x = location_vector[t][i];
      // for all locks
      for (z3::expr l : lc.lock_vector) {
        // ensure that function returns hold the same locks as function call positions
        if (thread->get_state(i).return_state != no_state) {
          lc.cons_functions = lc.cons_functions && lc.inl(x,l) == lc.inl(location_vector[t][thread->get_state(i).return_state],l);
        }
        // define inl in terms of the predecessors
        if (predecessors_forward[i].size()>=1) {
          auto it = predecessors_forward[i].begin();
          state_id_type first_pred = *it;
          z3::expr pred_x = location_vector[t][first_pred];
          lc.inl_def = lc.inl_def && lc.inl(x,l) == (lc.lock_b(x,l) || (!lc.unlock_b(x,l) && (lc.inle(pred_x,l))));
          for (; it != predecessors[i].end(); ++it) {
            // all predecessors need to be unlocked to lock in the next step (unlock right before does not count) [for all lower locks]
            lock_order(lc, l, x, location_vector[t][*it]);
            // to unlock before the predecessor needs to be locked (locking right before does not count)
            lc.cons_unl = lc.cons_unl && implies(lc.unlock_b(x,l), (lc.inl(location_vector[t][*it],l) && !lc.unlock_a(location_vector[t][*it],l)));
          }
        } else {
          // there is no predecessor
          lc.inl_def = lc.inl_def && lc.inl(x,l) == lc.lock_b(x,l);
          lc.cons_unl = lc.cons_unl && !lc.unlock_b(x,l);
        }
        // current one must be unlocked to lock after
        lock_order(lc, l, x);
        // to unlock the current one must have been locked
        lc.cons_unl = lc.cons_unl && implies(lc.unlock_a(x,l), (lc.inl(x,l)));
        
        // define that at join points locking has to agree
        if (predecessors[i].size()>1) {
          // for all precessors, either they all hold lock l or they don't
          z3::expr all_equal = ztrue;
          auto it = predecessors[i].begin();
          state_id_type first_pred = *it;
          for (++it; it != predecessors[i].end(); ++it) {
            state_id_type pred = *it;
            all_equal = all_equal && (lc.inle(location_vector[t][first_pred],l) == lc.inle(location_vector[t][pred],l));
          }
          lc.cons_join = lc.cons_join && (all_equal);
        }
        
        // no unlocking if not locked
        lc.cons_unl = lc.cons_unl && implies(lc.unlock_a(x,l), lc.inl(x,l));
      }
    
    }
    ++t;
  }
  
}

// ensure that all positions refering to the same physical location have the same lock and unlock semantics
void place_locks::lock_sameinstr(locking_constraints& lc)
{

  // check which ones have more than one position
  for (const auto& in : lock_after) {
    const vector<z3::expr>& inv = in.second;
    if (inv.size()>1) {
      z3::expr first = inv[0];
      for (z3::expr l : lc.lock_vector) {
        for (unsigned i = 1; i < inv.size(); ++i) {
          lc.cons_sameinstr = lc.cons_sameinstr && lc.lock_a(first,l) == lc.lock_a(inv[i],l) && lc.unlock_a(first,l) == lc.unlock_a(inv[i],l);
        }
      }
    }
  }
  for (const auto& in : lock_before) {
    const vector<z3::expr>& inv = in.second;
    if (inv.size()>1) {
      z3::expr first = inv[0];
      for (z3::expr l : lc.lock_vector) {
        for (unsigned i = 1; i < inv.size(); ++i) {
          lc.cons_sameinstr = lc.cons_sameinstr && lc.lock_b(first,l) == lc.lock_b(inv[i],l) && lc.unlock_b(first,l) == lc.unlock_b(inv[i],l);
        }
      }
    }
  }
}



void place_locks::locked_together(z3::optimize& slv, locking_constraints& lc, const synthesis::lock_symbols& locks_to_place)
{
  unsigned lock = 0;
  for (const disj<synthesis::lock_lists>& d : locks_to_place) {
    z3::expr lock_id = ctx.fresh_constant("lock" + to_string(lock), lc.locks);
    z3::expr di = zfalse;
    // add lock places
    for (const synthesis::lock_lists& lplaces : d) {
      z3::expr one_lock = zfalse;
      z3::expr e = ztrue;
      // all locations in the vector locked by lock l
      for (const synthesis::lock_list& lplaces2 : lplaces) {
        for (const auto& pl : lplaces2) {
          const abstraction::location& loc = pl->loc;
          z3::expr& loce = location_vector[loc.thread][loc.state];
          e = e && lc.inl(loce,lock_id);
        }
      }
      di = di || e;
    }
    lc.lock_ids.push_back(lock_id);
    slv.add(di);
    ++lock;
  }
}


void place_locks::result_to_locklist(locking_constraints& lc, const vector<vector<z3::expr>>& result, vector<pair<unsigned, abstraction::location >>& locks) {
  for (const vector<z3::expr>& r : result) {
    z3::expr lock = r[1];
    z3::expr loc = r[0];
    assert((Z3_ast)(r[2])==ztrue);
    int llock;
    auto it = lc.lock_map.find(lock);
    assert(it != lc.lock_map.end());
    llock = it->second;
    auto lloc = location_map.find(loc);
    assert (lloc != location_map.end());
    locks.push_back(make_pair(llock,lloc->second));
  }
}

bool same_instructions(const synthesis::lock_lists& a, const synthesis::lock_lists& b, unsigned threads) {
  // first merge the instructions for each thread in a vector
  vector<unordered_set<clang::Stmt*>> a_threads(threads);
  vector<unordered_set<clang::Stmt*>> b_threads(threads);
  for (const synthesis::lock_list& al : a) {
    for (const abstraction::psymbol& s : al) {
      if (s->stmt) a_threads[s->thread_id()].insert(s->stmt);
    }
  }
  for (const synthesis::lock_list& bl : b) {
    for (const abstraction::psymbol& s : bl) {
      if (s->stmt) b_threads[s->thread_id()].insert(s->stmt);
    }
  }
  // at least one thread must differ for it to make sense to ask for different thread
  unsigned violations = 0;
  for (unsigned t1 = 0; t1 < threads; ++t1) {
    for (unsigned t2 = 0; t2 < threads; ++t2) {
      if (!a_threads[t1].empty() && !b_threads[t2].empty()) {
        if (Limi::internal::set_intersection_empty(a_threads[t1], b_threads[t2])) ++violations;
      }
    }
  }
  if (violations > 2) return false; // 2 violations would be normal
  return true;
}

bool same_instructions(const disj<synthesis::lock_lists>& a, const disj<synthesis::lock_lists>& b, unsigned threads) {
  // does any of those disjuncts speak about the same locations
  for (const synthesis::lock_lists& a1 : a) {
    for (const synthesis::lock_lists& b1 : b) {
      if (same_instructions(a1,b1, threads)) return true;
    }
  }
  return false;
}

void place_locks::cost_model_max_concurrency(z3::optimize& slv, locking_constraints& lc, cost_type cost_function, const synthesis::lock_symbols& locks_to_place)
{
  unsigned cons = 0;
  unsigned t1 = 0;
  for (const cfg::abstract_cfg* thread1 : threads) {
    unsigned t2 = 0;
    for (const cfg::abstract_cfg* thread2 : threads) {
      if (t1 <= t2) continue;

      for (unsigned i = 1; i <= thread1->no_states(); ++i) {
        z3::expr x = location_vector[t1][i];

        for (unsigned j = 1; j <= thread2->no_states(); ++j) {
          z3::expr y = location_vector[t2][j];

          z3::expr c = x.ctx().bool_val(true);
          for (z3::expr l : lc.lock_vector) {
            c = c && (!lc.inl(x,l) || !lc.inl(y,l), 100); // return 100 for the expression, implicitly true
          }
          slv.add(c, 1);
          cons++;

        }
      }

      ++t2;
    }
    ++t1;
  }
  //debug << "Finished making " << cons << " constraints for " << threads.size() << " threads!" << endl;
}

void place_locks::cost_model(z3::optimize& slv, locking_constraints& lc, cost_type cost_function, const synthesis::lock_symbols& locks_to_place)
{
  if (cost_function==cost_type::unoptimized) return;
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      z3::expr x = location_vector[t][i];
      // for all locks
      for (z3::expr l : lc.lock_vector) {
        slv.add(!lc.inl(x,l), 1);
      }
    }
    ++t;
  }
  assert (lc.lock_ids.size() == locks_to_place.size());
  if (cost_function==cost_type::max_pairwise_concurrency) {
    place_locks::cost_model_max_concurrency(slv, lc, cost_function, locks_to_place);
    return;
  }
  switch (cost_function) {
    case cost_type::absolute_minimum:
      // the condition above is enough
      break;
    case cost_type::small_locks: 
      // ask it to keep different locks for different conflicts
      // for preformance reasons we only seperate some of the conflicts
      for (unsigned i = 0; i < locks_to_place.size(); ++i) {
        for (unsigned j = 0; j < locks_to_place.size(); ++j) {
          if (j!=i) {
            const disj<synthesis::lock_lists>& a = locks_to_place[i];
            const disj<synthesis::lock_lists>& b = locks_to_place[j];
            if (!same_instructions(a,b, threads.size())) {
              // we prefer different locks
              slv.add(lc.lock_ids[i]!=lc.lock_ids[j], 100);
            }
          } // end if (j!=i)
        }
      }
      break;
    case cost_type::coarse:
      t = 0;
      for (const cfg::abstract_cfg* thread : threads) {
        for (unsigned i = 1; i <= thread->no_states(); ++i) {
          z3::expr x = location_vector[t][i];
          // for all locks
          for (z3::expr l : lc.lock_vector) {
            slv.add(!lc.lock_b(x,l), 20);
            slv.add(!lc.lock_a(x,l), 20);
          }
        }
        ++t;
      }
      break;
  }
  if (cost_function == cost_type::small_locks) {
    // find conflicts that do not talk about the same instructions
    for (unsigned i = 0; i < locks_to_place.size(); ++i) {
      for (unsigned j = 0; j < locks_to_place.size(); ++j) {
        if (j!=i) {
          const disj<synthesis::lock_lists>& a = locks_to_place[i];
          const disj<synthesis::lock_lists>& b = locks_to_place[j];
          if (!same_instructions(a,b, threads.size())) {
            // we prefer different locks
            slv.add(lc.lock_ids[i]!=lc.lock_ids[j], 100);
            //cout << (lc.lock_ids[i]!=lc.lock_ids[j]) << endl;
          }
        } // end if (j!=i)
      }
    }
  }
}

void place_locks::init_locks(place_locks::locking_constraints& lc, unsigned max_lock)
{
  vector<string> lock_names;
  for (unsigned i = 0; i<max_lock; ++i) {
    string name = "lock_" + to_string(i);
    lock_names.push_back(name);
  }
  z3::func_decl_vector consts(ctx);
  z3::func_decl_vector ts(ctx);
  lc.locks = ctx.enumeration_sort("locks", lock_names, consts, ts);
  
  for (unsigned index = 0; index < lock_names.size(); ++index) {
    z3::expr ex = consts[index]();
    lc.lock_vector.push_back(ex);
    lc.lock_map.insert(make_pair(ex,index));
  }
  
  // lock and unlock functions
  lc.lock_a = ctx.function("lock_a", locations, lc.locks, ctx.bool_sort());
  lc.unlock_a = ctx.function("unlock_a", locations, lc.locks, ctx.bool_sort());
  lc.lock_b = ctx.function("lock_b", locations, lc.locks, ctx.bool_sort());
  lc.unlock_b = ctx.function("unlock_b", locations, lc.locks, ctx.bool_sort());
  
  lc.inl = ctx.function("inl", locations, lc.locks, ctx.bool_sort());
  
}

void place_locks::add_constraints(z3::optimize& slv, locking_constraints& lc)
{  
  lock_consistancy(lc);
  lock_sameinstr(lc);
  
  // hard constraints
  slv.add(lc.inl_def);
  
  slv.add(lc.cons_loc);
  slv.add(lc.cons_basic);
  slv.add(lc.cons_join);
  slv.add(lc.cons_unl);
  slv.add(lc.cons_lo);
  slv.add(lc.cons_functions);
  slv.add(lc.cons_sameinstr);
  slv.add(lc.cons_preemption);
  slv.add(lc.cons_lockorder);
  
}

void place_locks::remove_duplicates(std::vector<single_placement>& locks) {
  for (unsigned i = 0; i < locks.size(); ++i) {
    for (unsigned j = i+1; j < locks.size(); ++j) {
      if (locks[i].lock == locks[j].lock && locks[i].lock_t == locks[j].lock_t && locks[i].position == locks[j].position) { // same lock
        const auto& statei = threads[locks[i].location.thread]->get_state(locks[i].location.state);
        const auto& statej = threads[locks[j].location.thread]->get_state(locks[j].location.state);
        if (locks[i].position == position_type::before && statei.lock_before == statej.lock_before || locks[i].position == position_type::after && statei.lock_after == statej.lock_after) {
          // these are actually refering to the same instruction
          locks.erase(locks.begin()+j);--j;
        }
      }
    }
  }
}

bool place_locks::find_locks(const synthesis::lock_symbols& locks_to_place, cost_type cost_function, placement_result& presult) {
  // get cost model model
  vector<single_placement>& to_place = presult.locks;
  
  // TODO: make this a user input
  unsigned max_locks = min<unsigned>(lock_limit, locks_to_place.size()); // don't allow more locks than conflicts though
  
  if (verbosity >= 1)
    debug << "Starting lock placement for " << short_name(cost_function) << endl;
  
  locking_constraints lc(ctx);
  init_locks(lc, max_locks);

  z3::optimize slv(ctx);

  locked_together(slv, lc, locks_to_place);
  add_constraints(slv, lc);
  
  cost_model(slv, lc, cost_function, locks_to_place);
  
  z3::check_result res = slv.check();
  if (res != z3::sat) {
    debug << "Lock placement failed!" << endl;
    return false;
  }
  
  debug << "Lock placement succeeded!" << endl;
  z3::model model = slv.get_model();
  
  if (verbosity>=2) {
    print_func_interp(model, lc.lock_b);
    print_func_interp(model, lc.lock_a);
    print_func_interp(model, lc.unlock_a);
    print_func_interp(model, lc.unlock_b);
    print_func_interp(model, lc.inl);
  }
  
  vector<pair<unsigned, abstraction::location>> lock_before;
  vector<pair<unsigned, abstraction::location>> lock_after;
  vector<pair<unsigned, abstraction::location>> unlock_before;
  vector<pair<unsigned, abstraction::location>> unlock_after;
  vector<vector<z3::expr>> result;
  z3::expr elsee = func_result(model, lc.lock_a, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(lc, result, lock_after);
  for (const pair<unsigned, abstraction::location>& lock : lock_after)
    to_place.emplace_back(lock.first, lock_type::lock, position_type::after, lock.second);
  
  result.clear();
  elsee = func_result(model, lc.lock_b, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(lc, result, lock_before);
  for (const pair<unsigned, abstraction::location>& lock : lock_before)
    to_place.emplace_back(lock.first, lock_type::lock, position_type::before, lock.second);
  
  result.clear();
  elsee = func_result(model, lc.unlock_a, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(lc, result, unlock_after);
  for (const pair<unsigned, abstraction::location>& lock : unlock_after)
    to_place.emplace_back(lock.first, lock_type::unlock, position_type::after, lock.second);
  
  result.clear();
  elsee = func_result(model, lc.unlock_b, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(lc, result, unlock_before);
  for (const pair<unsigned, abstraction::location>& lock : unlock_before)
    to_place.emplace_back(lock.first, lock_type::unlock, position_type::before, lock.second);
  
  remove_duplicates(to_place);
  
  presult.statistics = get_statistics(to_place, model, lc);
  
  return true;
}

z3::expr place_locks::locking_constraints::inle(const z3::expr& x, const z3::expr& l)
{
  return inl(x,l) && !unlock_a(x,l) || lock_a(x,l);
}

lock_statistics place_locks::get_statistics(vector<single_placement>& to_lock, z3::model& model, const locking_constraints& lc)
{
  lock_statistics result;
  
  // find number of locks in use
  unordered_set<unsigned> locks;
  
  for (const single_placement& lock : to_lock) {
    if (lock.lock_t == lock_type::lock) result.lock_instr++;
    if (lock.lock_t == lock_type::unlock) result.unlock_instr++;
    locks.insert(lock.lock);
  }
  
  result.locks = locks.size();
  
  // finally find the number of instructions inside a lock
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned j = 1; j <= thread->no_states(); ++j) {
      if (!thread->get_state(j).is_dummy()) {
        bool locked = false;
        for (z3::expr l : lc.lock_vector) {
          if (model.eval(lc.inl(location_vector[t][j], l)).get_bool()) {
            locked = true;
            break;
          }            
        }
        if (locked) result.in_lock++;
      }
    }
    ++t;
  }
  
  return result;
}
