/*
 * Copyright 2015, IST Austria
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
    }
    return out;
  }
  
  std::string short_name(cost_type c) {
    switch (c) {
      case cost_type::absolute_minimum:
        return "absmin";
      case cost_type::small_locks:
        return "small";
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
  
  // lock and unlock functions
  lock_a = ctx.function("lock_a", locations, locks, ctx.bool_sort());
  unlock_a = ctx.function("unlock_a", locations, locks, ctx.bool_sort());
  lock_b = ctx.function("lock_b", locations, locks, ctx.bool_sort());
  unlock_b = ctx.function("unlock_b", locations, locks, ctx.bool_sort());
  
  inl = ctx.function("inl", locations, locks, ctx.bool_sort());
  
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

void place_locks::lock_order(locking_constraints& lc, unsigned lock, z3::expr location, z3::expr predecessor) {
  z3::expr l = ctx.int_val(lock);
  for (unsigned il = 0; il <= lock; ++il) {
    z3::expr lb = ctx.int_val(il);
    lc.cons_lockorder = lc.cons_lockorder && implies(lock_b(location,l), (!inl(predecessor,lb) && !lock_a(predecessor,lb)));
  }
}

void place_locks::lock_order(locking_constraints& lc, unsigned lock, z3::expr location) {
  z3::expr l = ctx.int_val(lock);
  for (unsigned il = 0; il <= lock; ++il) {
    z3::expr lb = ctx.int_val(il);
    lc.cons_lockorder = lc.cons_lockorder && implies(lock_a(location,l), (!inl(location,lb)));
  }
}

void place_locks::lock_consistancy(locking_constraints& lc, unsigned from_lock, unsigned to_lock)
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
      for (unsigned il = from_lock; il <= to_lock; ++il) {
        z3::expr l = ctx.int_val(il);
        const auto& s = thread->get_state(i);
        if (s.lock_before == clang::SourceLocation()) {
          lc.cons_loc = lc.cons_loc && !lock_b(x,l) && !unlock_b(x,l);
        }
        if (s.lock_after == clang::SourceLocation()) {
          lc.cons_loc = lc.cons_loc && !lock_a(x,l) && !unlock_a(x,l);
        }
        lc.cons_basic = lc.cons_basic && (!lock_a(x,l) || !unlock_a(x,l)) && (!lock_b(x,l) || !unlock_b(x,l));
        if (s.action && s.action->is_preemption_point()) {
          // not allowed to lock a preemption point
          lc.cons_preemption = lc.cons_preemption && !inl(x,l);
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
      for (unsigned il = from_lock; il <= to_lock; ++il) {
        z3::expr l = ctx.int_val(il);
        // ensure that function returns hold the same locks as function call positions
        if (thread->get_state(i).return_state != no_state) {
          lc.cons_functions = lc.cons_functions && inl(x,l) == inl(location_vector[t][thread->get_state(i).return_state],l);
        }
        // define inl in terms of the predecessors
        if (predecessors_forward[i].size()>=1) {
          auto it = predecessors_forward[i].begin();
          state_id_type first_pred = *it;
          z3::expr pred_x = location_vector[t][first_pred];
          lc.inl_def = lc.inl_def && inl(x,l) == (lock_b(x,l) || (!unlock_b(x,l) && (inle(pred_x,l))));
          for (; it != predecessors[i].end(); ++it) {
            // all predecessors need to be unlocked to lock in the next step (unlock right before does not count) [for all lower locks]
            lock_order(lc, il, x, location_vector[t][*it]);
            // to unlock before the predecessor needs to be locked (locking right before does not count)
            lc.cons_unl = lc.cons_unl && implies(unlock_b(x,l), (inl(location_vector[t][*it],l) && !unlock_a(location_vector[t][*it],l)));
          }
        } else {
          // there is no predecessor
          lc.inl_def = lc.inl_def && inl(x,l) == lock_b(x,l);
          lc.cons_unl = lc.cons_unl && !unlock_b(x,l);
        }
        // current one must be unlocked to lock after
        lock_order(lc, il, x);
        // to unlock the current one must have been locked
        lc.cons_unl = lc.cons_unl && implies(unlock_a(x,l), (inl(x,l)));
        
        // define that at join points locking has to agree
        if (predecessors[i].size()>1) {
          // for all precessors, either they all hold lock l or they don't
          z3::expr all_equal = ztrue;
          auto it = predecessors[i].begin();
          state_id_type first_pred = *it;
          for (++it; it != predecessors[i].end(); ++it) {
            state_id_type pred = *it;
            all_equal = all_equal && (inle(location_vector[t][first_pred],l) == inle(location_vector[t][pred],l));
          }
          lc.cons_join = lc.cons_join && (all_equal);
        }
        
        // no unlocking if not locked
        lc.cons_unl = lc.cons_unl && implies(unlock_a(x,l), inl(x,l));
      }
    
    }
    ++t;
  }
  
}

// ensure that all positions refering to the same physical location have the same lock and unlock semantics
void place_locks::lock_sameinstr(locking_constraints& lc, unsigned from_lock, unsigned to_lock)
{

  // check which ones have more than one position
  for (const auto& in : lock_after) {
    const vector<z3::expr>& inv = in.second;
    if (inv.size()>1) {
      z3::expr first = inv[0];
      for (unsigned il = from_lock; il <= to_lock; ++il) {
        z3::expr l = ctx.int_val(il);
        for (unsigned i = 1; i < inv.size(); ++i) {
          lc.cons_sameinstr = lc.cons_sameinstr && lock_a(first,l) == lock_a(inv[i],l) && unlock_a(first,l) == unlock_a(inv[i],l);
        }
      }
    }
  }
  for (const auto& in : lock_before) {
    const vector<z3::expr>& inv = in.second;
    if (inv.size()>1) {
      z3::expr first = inv[0];
      for (unsigned il = from_lock; il <= to_lock; ++il) {
        z3::expr l = ctx.int_val(il);
        for (unsigned i = 1; i < inv.size(); ++i) {
          lc.cons_sameinstr = lc.cons_sameinstr && lock_b(first,l) == lock_b(inv[i],l) && unlock_b(first,l) == unlock_b(inv[i],l);
        }
      }
    }
  }
}



vector<pair<z3::expr,z3::expr>> place_locks::locked_together(const synthesis::lock_symbols& locks_to_place)
{
  vector<pair<z3::expr,z3::expr>> result;
  unsigned lock = 0;
  for (const disj<synthesis::lock_lists>& d : locks_to_place) {
    z3::expr lock_id = ctx.fresh_constant("lock" + to_string(lock), ctx.int_sort());
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
          e = e && inl(loce,lock_id);
        }
      }
      di = di || e;
    }
    result.push_back(make_pair(lock_id, di));
    ++lock;
  }
  return result;
}


void place_locks::result_to_locklist(const vector<vector<z3::expr>>& result, vector<pair<unsigned, abstraction::location >>& locks) {
  for (const vector<z3::expr>& r : result) {
    z3::expr lock = r[1];
    z3::expr loc = r[0];
    assert((Z3_ast)(r[2])==ztrue);
    int llock;
    bool res = lock.get_int(llock);
    assert(res);
    auto lloc = location_map.find(loc);
    assert (lloc != location_map.end());
    locks.push_back(make_pair(llock,lloc->second));
  }
}

void place_locks::cost_model(z3::optimize& slv, cost_type cost_function, unsigned from_lock, unsigned to_lock)
{
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      z3::expr x = location_vector[t][i];
      // for all locks
      for (unsigned il = from_lock; il <= to_lock; ++il) {
        z3::expr l = ctx.int_val(il);
        slv.add(!inl(x,l), 1);
      }
    }
    ++t;
  }
}

bool same_instructions(const synthesis::lock_lists& a, const synthesis::lock_lists& b, unsigned threads) {
  // first merge the instructions for each thread in a vector
  vector<unordered_set<abstraction::psymbol>> a_threads(threads);
  vector<unordered_set<abstraction::psymbol>> b_threads(threads);
  for (const synthesis::lock_list& al : a) {
    for (const abstraction::psymbol& s : al) {
      a_threads[s->thread_id()].insert(s);
    }
  }
  for (const synthesis::lock_list& bl : b) {
    for (const abstraction::psymbol& s : bl) {
      b_threads[s->thread_id()].insert(s);
    }
  }
  // at least one thread must differ for it to make sense to ask for different thread
  for (unsigned t = 0; t < threads; ++t) {
    if (a_threads[t].empty() != b_threads.empty())
      return false;
    if (!a_threads[t].empty() && !b_threads.empty() && Limi::internal::set_intersection_empty(a_threads[t], b_threads[t]))
      return false;
  }
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

void place_locks::cost_model_locks(z3::optimize& slv, cost_type cost_function, const synthesis::lock_symbols& locks_to_place, const vector< z3::expr > lock_ids)
{
  assert (lock_ids.size() == locks_to_place.size());
  if (cost_function == cost_type::small_locks) {
    // find conflicts that do not talk about the same instructions
    for (unsigned i = 0; i < locks_to_place.size(); ++i) {
      for (unsigned j = 0; j < locks_to_place.size(); ++j) {
        if (j!=i) {
          const disj<synthesis::lock_lists>& a = locks_to_place[i];
          const disj<synthesis::lock_lists>& b = locks_to_place[j];
          if (!same_instructions(a,b, threads.size())) {
            // we prefer different locks
            slv.add(lock_ids[i]!=lock_ids[j], 100);
          }
        } // end if (j!=i)
      }
    }
  }
}


void place_locks::add_constraints(z3::optimize& slv, cost_type cost_function, unsigned int from_lock, unsigned int to_lock)
{
  locking_constraints lc(ctx);
  
  lock_consistancy(lc, from_lock, to_lock);
  lock_sameinstr(lc, from_lock, to_lock);
  
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
  
  cost_model(slv, cost_function, from_lock, to_lock);
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

bool place_locks::minsmt(z3::optimize& slv, const synthesis::lock_symbols& locks_to_place, cost_type cost_function, placement_result& presult) {
  // get cost model model
  vector<single_placement>& to_place = presult.locks;
  
  
  if (verbosity >= 1)
    debug << "Starting lock placement for " << short_name(cost_function) << endl;
  
  slv.push();
 
  add_constraints(slv, cost_function, 0, 2);
  
  vector<pair<z3::expr,z3::expr>> locking = locked_together(locks_to_place);
  /*if (verbosity>=3) {
    debug << "Locking:" << endl;
    Limi::internal::print_vector(locking, debug);
    debug << endl;
  }*/
  
  vector<z3::expr> lock_ids;
  for (const pair<z3::expr,z3::expr>& e : locking) {
    slv.add(e.second);
    lock_ids.push_back(e.first);
  }
  
  cost_model_locks(slv, cost_function, locks_to_place, lock_ids);
  
  // restrict locks (only positive)
  for (z3::expr& e : lock_ids) {
    slv.add(e >= ctx.int_val(0));
  }
  
  slv.push();
  
  // restrict locks
  for (z3::expr& e : lock_ids) {
    slv.add(e <= ctx.int_val(2));
  }
  
  z3::check_result res = slv.check();
  if (res != z3::sat) {
    return false;
  }
  
  z3::model model = slv.get_model();
  
  if (verbosity>=2) {
    print_func_interp(model, lock_b);
    print_func_interp(model, lock_a);
    print_func_interp(model, unlock_a);
    print_func_interp(model, unlock_b);
    print_func_interp(model, inl);
  }
  
  vector<pair<unsigned, abstraction::location>> lock_before;
  vector<pair<unsigned, abstraction::location>> lock_after;
  vector<pair<unsigned, abstraction::location>> unlock_before;
  vector<pair<unsigned, abstraction::location>> unlock_after;
  vector<vector<z3::expr>> result;
  z3::expr elsee = func_result(model, lock_a, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, lock_after);
  for (const pair<unsigned, abstraction::location>& lock : lock_after)
    to_place.emplace_back(lock.first, lock_type::lock, position_type::after, lock.second);
  
  result.clear();
  elsee = func_result(model, lock_b, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, lock_before);
  for (const pair<unsigned, abstraction::location>& lock : lock_before)
    to_place.emplace_back(lock.first, lock_type::lock, position_type::before, lock.second);
  
  result.clear();
  elsee = func_result(model, unlock_a, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, unlock_after);
  for (const pair<unsigned, abstraction::location>& lock : unlock_after)
    to_place.emplace_back(lock.first, lock_type::unlock, position_type::after, lock.second);
  
  result.clear();
  elsee = func_result(model, unlock_b, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, unlock_before);
  for (const pair<unsigned, abstraction::location>& lock : unlock_before)
    to_place.emplace_back(lock.first, lock_type::unlock, position_type::before, lock.second);
  
  remove_duplicates(to_place);
  
  presult.statistics = get_statistics(to_place, model);
  
  slv.pop();
  slv.pop();
  return true;
}

bool place_locks::find_locks(const synthesis::lock_symbols& locks_to_place, const vector< cost_type >& cost_function, vector< placement_result >& to_place)
{
  // insert a lock for testing
  z3::optimize slv(ctx);
  
  for (cost_type cost : cost_function) {
    to_place.emplace_back();
    if (!minsmt(slv, locks_to_place, cost, to_place.back())) {
      return false;
    }
  }
  return true;
}

z3::expr place_locks::inle(const z3::expr& x, const z3::expr& l)
{
  return inl(x,l) && !unlock_a(x,l) || lock_a(x,l);
}

lock_statistics place_locks::get_statistics(vector<single_placement>& to_lock, z3::model& model)
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
        for (unsigned il : locks) {
          z3::expr l = ctx.int_val(il);
          if (model.eval(inl(location_vector[t][j], l)).get_bool()) {
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
