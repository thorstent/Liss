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
  // we assume the threads are compacted (no inreachable states)
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
  
  // other functions  
  init_locks();   
  init_consistancy();
  init_sameinstr();
}


void place_locks::init_locks()
{
  vector<string> lock_names;
  for (unsigned i = 0; i<3; ++i) {
    string name = "lock_" + to_string(i);
    lock_names.push_back(name);
  }
  z3::func_decl_vector consts(ctx);
  z3::func_decl_vector ts(ctx);
  locks = ctx.enumeration_sort("locks", lock_names, consts, ts);
  
  unsigned index = 0;
  for (unsigned index = 0; index < lock_names.size(); ++index) {
    z3::expr ex = consts[index]();
    lock_vector.push_back(ex);
    lock_map.insert(make_pair(ex,index));
  }
  
  // lock and unlock functions
  lock_a = ctx.function("lock_a", locations, locks, ctx.bool_sort());
  unlock_a = ctx.function("unlock_a", locations, locks, ctx.bool_sort());
  lock_b = ctx.function("lock_b", locations, locks, ctx.bool_sort());
  unlock_b = ctx.function("unlock_b", locations, locks, ctx.bool_sort());
  
  inl = ctx.function("inl", locations, locks, ctx.bool_sort());

}

void place_locks::init_consistancy()
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
      for (z3::expr l : lock_vector) {
        const auto& s = thread->get_state(i);
        if (s.lock_before == clang::SourceLocation()) {
          cons_loc = cons_loc && !lock_b(x,l) && !unlock_b(x,l);
        }
        if (s.lock_after == clang::SourceLocation()) {
          cons_loc = cons_loc && !lock_a(x,l) && !unlock_a(x,l);
        }
        cons_basic = cons_basic && (!lock_a(x,l) || !unlock_a(x,l)) && (!lock_b(x,l) || !unlock_b(x,l));
        if (s.action && s.action->is_preemption_point()) {
          // not allowed to lock a preemption point
          cons_preemption = cons_preemption && !inl(x,l);
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
      for (z3::expr& l : lock_vector) {
        // ensure that function returns hold the same locks as function call positions
        if (thread->get_state(i).return_state != no_state) {
          cons_functions = cons_functions && inl(x,l) == inl(location_vector[t][thread->get_state(i).return_state],l);
        }
        // define inl in terms of the predecessors
        if (predecessors_forward[i].size()>=1) {
          auto it = predecessors_forward[i].begin();
          state_id_type first_pred = *it;
          z3::expr pred_x = location_vector[t][first_pred];
          inl_def = inl_def && inl(x,l) == (lock_b(x,l) || (!unlock_b(x,l) && (inle(pred_x,l))));
          for (; it != predecessors[i].end(); ++it) {
            // all predecessors need to be unlocked to lock in the next step (unlock right before does not count)
            cons_lo = cons_lo && implies(lock_b(x,l), (!inl(location_vector[t][*it],l) && !lock_a(location_vector[t][*it],l)));
            // to unlock before the predecessor needs to be locked (locking right before does not count)
            cons_unl = cons_unl && implies(unlock_b(x,l), (inl(location_vector[t][*it],l) && !unlock_a(location_vector[t][*it],l)));
          }
        } else {
          // there is no predecessor
          inl_def = inl_def && inl(x,l) == lock_b(x,l);
          cons_unl = cons_unl && !unlock_b(x,l);
        }
        // current one must be unlocked to lock after
        cons_lo = cons_lo && implies(lock_a(x,l), (!inl(x,l)));
        // to unlock the current one must have been locked
        cons_unl = cons_unl && implies(unlock_a(x,l), (inl(x,l)));
        
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
          cons_join = cons_join && (all_equal);
        }
        
        // no unlocking if not locked
        cons_unl = cons_unl && implies(unlock_a(x,l), inl(x,l));
      }
    
    }
    ++t;
  }
  
}

// ensure that all positions refering to the same physical location have the same lock and unlock semantics
void place_locks::init_sameinstr()
{
  unordered_map<clang::SourceLocation,vector<z3::expr>> lock_before;
  unordered_map<clang::SourceLocation,vector<z3::expr>> lock_after;
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
  // check which ones have more than one position
  for (const auto& in : lock_after) {
    const vector<z3::expr>& inv = in.second;
    if (inv.size()>1) {
      z3::expr first = inv[0];
      for (z3::expr& l : lock_vector) {
        for (unsigned i = 1; i < inv.size(); ++i) {
          cons_sameinstr = cons_sameinstr && lock_a(first,l) == lock_a(inv[i],l) && unlock_a(first,l) == unlock_a(inv[i],l);
        }
      }
    }
  }
  for (const auto& in : lock_before) {
    const vector<z3::expr>& inv = in.second;
    if (inv.size()>1) {
      z3::expr first = inv[0];
      for (z3::expr& l : lock_vector) {
        for (unsigned i = 1; i < inv.size(); ++i) {
          cons_sameinstr = cons_sameinstr && lock_b(first,l) == lock_b(inv[i],l) && unlock_b(first,l) == unlock_b(inv[i],l);
        }
      }
    }
  }
}


void place_locks::result_to_locklist(const vector<vector<z3::expr>>& result, vector<pair<unsigned, abstraction::location >>& locks) {
  for (const vector<z3::expr>& r : result) {
    z3::expr lock = r[1];
    z3::expr loc = r[0];
    assert((Z3_ast)(r[2])==ztrue);
    auto llock = lock_map.find(lock);
    auto lloc = location_map.find(loc);
    assert (llock != lock_map.end());
    assert (lloc != location_map.end());
    locks.push_back(make_pair(llock->second,lloc->second));
  }
}

vector<z3::expr> place_locks::locked_together(const synthesis::lock_symbols& locks_to_place)
{
  vector<z3::expr> result;
  for (const disj<synthesis::lock_lists>& d : locks_to_place) {
    z3::expr di = zfalse;
    // add lock places
    for (const synthesis::lock_lists& lplaces : d) {
      z3::expr one_lock = zfalse;
      for (z3::expr& l : lock_vector) {
        z3::expr e = ztrue;
        // all locations in the vector locked by lock l
        for (const synthesis::lock_list& lplaces2 : lplaces) {
          for (const auto& pl : lplaces2) {
            const abstraction::location& loc = pl->loc;
            z3::expr& loce = location_vector[loc.thread][loc.state];
            e = e && inl(loce,l);
          }
        }
        one_lock = one_lock || e;
      }
      di = di || one_lock;
    }
    result.push_back(di);
  }
  return result;
}

vector< std::pair< z3::expr, unsigned int > > place_locks::cost_model(cost_type cost_function)
{
  vector< std::pair< z3::expr, unsigned int > > result;
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      z3::expr x = location_vector[t][i];
      // for all locks
      for (z3::expr& l : lock_vector) {
        result.emplace_back(make_pair(!inl(x,l), 1));
      }
    }
    ++t;
  }
  return result;
}


bool place_locks::find_locks(const synthesis::lock_symbols& locks_to_place, std::vector<placement_result>& to_place, cost_type cost_function)
{
  // insert a lock for testing
  z3::optimize slv(ctx);
  
  // hard constraints
  slv.add(inl_def);
  
  slv.add(cons_loc);
  slv.add(cons_basic);
  slv.add(cons_join);
  slv.add(cons_unl);
  slv.add(cons_lo);
  slv.add(cons_functions);
  slv.add(cons_sameinstr);
  slv.add(cons_preemption);
  
  vector<z3::expr> locking = locked_together(locks_to_place);
  if (verbosity>=3) {
    debug << "Locking:" << endl;
    Limi::internal::print_vector(locking, debug);
    debug << endl;
  }
  
  for (z3::expr& e : locking)
    slv.add(e);
  
  // get cost model model
  std::vector<std::pair<z3::expr, unsigned>> costs = cost_model(cost_function);
  for (const std::pair<z3::expr, unsigned>& c : costs)
    slv.add(c.first, c.second);
  
  if (verbosity > 1)
    debug << "Starting lock placement" << endl;
    
  z3::check_result res = slv.check();
  if (res != z3::sat) {
    return false;
  }
    
  z3::model model = slv.get_model();
  
  //z3::check_result last_res = slv.check();
  /*if (last_res != z3::sat)  {
    if (verbosity>=1) {
      slv.pop();
      synthesis::min_unsat(slv, locking);
      debug << "Problem locks:" << endl;
      Limi::internal::print_vector(locking, debug);
      debug << endl;
    }
    return false;
  }*/
  
  
  
  if (verbosity>=2) {
    print_func_interp(model, lock_b);
    print_func_interp(model, lock_a);
    print_func_interp(model, unlock_a);
    print_func_interp(model, unlock_b);
    print_func_interp(model, inl);
  }
  /*if (verbosity>=1)
    debug << "Final cost: " << model.eval(cost) << endl;*/

  vector<pair<unsigned, abstraction::location>> locklist;
  vector<vector<z3::expr>> result;
  z3::expr elsee = func_result(model, lock_a, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, locklist);
  for (const pair<unsigned, abstraction::location>& lock : locklist)
    to_place.emplace_back(lock.first, lock_type::lock, position_type::after, lock.second);
  
  result.clear(); locklist.clear();
  elsee = func_result(model, lock_b, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, locklist);
  for (const pair<unsigned, abstraction::location>& lock : locklist)
    to_place.emplace_back(lock.first, lock_type::lock, position_type::before, lock.second);
  
  result.clear(); locklist.clear();
  elsee = func_result(model, unlock_a, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, locklist);
  for (const pair<unsigned, abstraction::location>& lock : locklist)
    to_place.emplace_back(lock.first, lock_type::unlock, position_type::after, lock.second);
  
  result.clear(); locklist.clear();
  elsee = func_result(model, unlock_b, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, locklist);
  for (const pair<unsigned, abstraction::location>& lock : locklist)
    to_place.emplace_back(lock.first, lock_type::unlock, position_type::before, lock.second);
  return true;
}

z3::expr place_locks::inle(const z3::expr& x, const z3::expr& l)
{
  return inl(x,l) && !unlock_a(x,l) || lock_a(x,l);
}
