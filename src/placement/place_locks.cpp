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
#include "options.h"

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
      cout << "  " << cond << " -> " << then << endl;
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

place_locks::place_locks(const std::vector< const cfg::abstract_cfg* >& threads) : threads(threads)
{
  // build position array
  // we assume the threads are compacted (no inreachable states)
  vector<string> positions;
  unsigned i = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned j = 1; j <= thread->no_states(); ++j) { 
      string name = to_string(thread->thread_id) + "_" + to_string(j);
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
      location_map.insert(make_pair(ex, location(location_vector.size()-1,j)));
      ++index;
    }
  }
  assert(index == consts.size());
  
  // cost
  cost = ctx.int_const("cost");
  
  // other functions  
  init_locks();   
  init_consistancy();
  
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
  lock = ctx.function("lock", locations, locks, ctx.bool_sort());
  unlock = ctx.function("unlock", locations, locks, ctx.bool_sort());
  
  inl = ctx.function("inl", locations, locks, ctx.bool_sort());
  /*inle = ctx.function("inle", locations, locks, ctx.bool_sort());
  inls = ctx.function("inls", locations, locks, ctx.bool_sort());
  
  inl_def = ztrue;
  z3::expr x = ctx.fresh_constant("x", locations);
  z3::expr xp = ctx.fresh_constant("xp", locations); // x'
  z3::expr l = ctx.fresh_constant("l", locks);
  inl_def = inl_def && z3::forall(x, l, inls(x,l) == z3::exists(xp, succ(xp,x)==ztrue && inle(xp, l)==ztrue));
  inl_def = inl_def && z3::forall(x, l, inl(x,l) == (lock(x,l) || inls(x, l)));
  inl_def = inl_def && z3::forall(x, l, inle(x,l) == (inl(x, l) && !unlock(x,l)));*/
}

void place_locks::init_consistancy()
{
  /*z3::expr x = ctx.fresh_constant("x", locations);
  z3::expr xp = ctx.fresh_constant("xp", locations); // x'
  z3::expr l = ctx.fresh_constant("l", locks);*/
  lock_consistency = ztrue;
  inl_def = ztrue;
  z3::expr cost_sum = ctx.int_val(0);
  // predecessor rule
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    // make a predecessor map
    vector<unordered_set<state_id>> predecessors(thread->no_states()+1);
    vector<unordered_set<state_id>> predecessors_forward(thread->no_states()+1); // only the forward predecessors ()
    
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      // forbid locking and unlock at non-locations
      if (!thread->get_state(i).action) {
        for (z3::expr l : lock_vector)
          lock_consistency = lock_consistency && !lock(location_vector[t][i],l) && !unlock(location_vector[t][i],l);
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
        // define inl in terms of the predecessors
        if (predecessors_forward[i].size()>=1) {
          auto it = predecessors_forward[i].begin();
          state_id first_pred = *it;
          z3::expr pred_x = location_vector[t][first_pred];
          inl_def = inl_def && inl(x,l) == (lock(x,l) || (inl(pred_x,l) && !unlock(pred_x,l)));
          for (; it != predecessors[i].end(); ++it) {
            // all predecessors need to be unlocked to lock in the next step
            lock_consistency = lock_consistency && implies(lock(x,l), !inl(location_vector[t][*it],l));
          }
        } else {
          // there is on predecessor
          inl_def = inl_def && inl(x,l) == lock(x,l);
        }
        // define that at join points locking has to agree
        if (predecessors[i].size()>1) {
          // for all precessors, either they hold l or they don't
          z3::expr all_equal = ztrue;
          auto it = predecessors[i].begin();
          state_id first_pred = *it;
          for (++it; it != predecessors[i].end(); ++it) {
            state_id pred = *it;
            all_equal = all_equal && ((inl(location_vector[t][first_pred],l)&& !unlock(location_vector[t][first_pred],l)) == (inl(location_vector[t][pred],l)&& !unlock(location_vector[t][pred],l)));
          }
          lock_consistency = lock_consistency && (all_equal);
        }
        
        // no unlocking if not locked
        lock_consistency = lock_consistency && implies(unlock(x,l), inl(x,l));
        // final position must not be locked
        if (thread->get_state(i).final)
          lock_consistency = lock_consistency && (unlock(x,l) || !inl(x,l));
        // define cost
        cost_sum = cost_sum + z3::ite(inl(x,l),ctx.int_val(1),ctx.int_val(0));
      }
    
    }
    ++t;
  }

  cost_def = cost == cost_sum;
  

  /*lock_consistency = lock_consistency && z3::forall(x, l, implies(unlock(x,l), inl(x,l) ));
  lock_consistency = lock_consistency && z3::forall(x, l, implies(lock(x,l), !inls(x,l) ));
  lock_consistency = lock_consistency && z3::forall(x, l, implies(lock(x,l) , !z3::exists (xp, succ(xp,x) && unlock(xp,l) ) ));*/
  
}

void place_locks::result_to_locklist(const vector<vector<z3::expr>>& result, vector<pair<unsigned, location >>& locks) {
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

void place_locks::find_locks(const vector< vector< location > >& locks_to_place, vector<pair<unsigned, location >>& locks_placed, vector<pair<unsigned, location >>& unlocks_placed)
{
  /*z3::expr x = ctx.fresh_constant("x", locations);
  z3::expr l = ctx.fresh_constant("l", locks);*/
  
  // insert a lock for testing
  z3::solver slv(ctx);
  slv.add(inl_def);
  slv.add(cost_def);
  slv.add(lock_consistency);
  //slv.add(z3::forall(x,l,lock(x,l)==z3::ite(x==location_vector[0][2]&&l==lock_vector[0],ztrue,zfalse)));
  /*slv.add(z3::forall(x,l,inl(x,l)==zfalse));
  slv.add(z3::forall(x,l,inls(x,l)==zfalse));
  slv.add(z3::forall(x,l,inle(x,l)==zfalse));
  slv.add(z3::forall(x,l,lock(x,l)==zfalse));
  slv.add(z3::forall(x,l,unlock(x,l)==zfalse));*/
  
  // add lock places
  for (const vector< location >& lplaces : locks_to_place) {
    z3::expr one_lock = zfalse;
    for (z3::expr& l : lock_vector) {
      z3::expr e = ztrue;
      for (const location& loc : lplaces) {
        e = e && inl(location_vector[loc.thread][loc.state],l);
      }
      one_lock = one_lock || e;
    }
    cout << one_lock << endl;
    slv.add(one_lock);
  }
  
  if (verbosity > 1)
    debug << "Starting lock placement" << endl;
  
  z3::check_result last_res = slv.check();
  cout << last_res << endl;
  assert (last_res == z3::sat);
  z3::model last_model = slv.get_model();
  // try and reduce cost
  while (last_res==z3::sat) {
    last_model = slv.get_model();
    z3::expr c = last_model.eval(cost);
    slv.add(cost < c);
    last_res = slv.check();
  }
  print_func_interp(last_model, lock);
  print_func_interp(last_model, unlock);
  print_func_interp(last_model, inl);
  cout << last_model.eval(cost) << endl;

  vector<vector<z3::expr>> result;
  z3::expr elsee = func_result(last_model, lock, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, locks_placed);
  
  result.clear();
  elsee = func_result(last_model, unlock, result);
  assert ((Z3_ast)elsee == zfalse);
  result_to_locklist(result, unlocks_placed);
  
}

