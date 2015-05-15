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

using namespace placement;
using namespace std;

#include <string.h>

void print_func_interp1(const z3::model& model, const z3::func_decl& func_decl);

void print_expr(const z3::model& model, const z3::expr& expr) {
  Z3_decl_kind decl_kind = expr.decl().decl_kind();
  switch (decl_kind) {
    case Z3_OP_UNINTERPRETED: {
      print_func_interp1(model, expr.decl());
      //print_expr(model, expr.arg(0));
    } break;
    case Z3_OP_ITE: {
      z3::expr cond = expr.arg(0);
      z3::expr then = expr.arg(1);
      z3::expr else1 = expr.arg(2);
      cout << "  " << cond << " -> " << then << endl;
      print_expr(model, else1);
    } break;
    default:
      cout << "  else: " << expr << endl;
      break;
  }
}

void print_func_interp1(const z3::model& model, const z3::func_decl& func_decl) {
  //cout << func_decl.name().str() << endl;
  z3::func_interp interp = model.get_func_interp(func_decl);
  for (unsigned i = 0; i < interp.num_entries(); ++i) {
    z3::func_entry e = interp.entry(i);
    cout << "  (";
    for (unsigned j = 0; j < e.num_args(); ++j) {
      cout << e.arg(j);
      if (j < e.num_args()-1) cout << ", ";
    }
    cout << ") -> ";
    cout << e.value() << endl;
  }
  print_expr(model, interp.else_value());
}

void print_func_interp(const z3::model& model, const z3::func_decl& func_decl) {
  cout << func_decl.name().str() << endl;
  print_func_interp1(model, func_decl);
}

place_locks::place_locks(const std::vector< const cfg::abstract_cfg* >& threads) : threads(threads)
{
  // build position array
  // we assume the threads are compacted (no inreachable states)
  vector<const char*> positions;
  unsigned i = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    for (unsigned j = 1; j <= thread->no_states(); ++j) { 
      string name = to_string(i) + "_" + to_string(j);
      const char* namec = strdup(name.c_str());
      positions.push_back(namec);      
    }
  }
  
  z3::func_decl_vector consts(ctx);
  z3::func_decl_vector ts(ctx);
  locations = ctx.enumeration_sort("locations", positions.size(), &positions[0], consts, ts);
  // delete the name array once again
  for (const char* p : positions) {
    delete p;
  }
  
  unsigned index = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    location_vector.push_back(std::vector<z3::expr>());
    location_vector.back().push_back(ctx);
    for (unsigned j = 1; j <= thread->no_states(); ++j) { 
      location_vector.back().push_back(consts[index]());
      ++index;
    }
  }
  assert(index == consts.size());
  
  init_successors();
  init_locks();
  init_consistancy();
  
  /*z3::expr x = ctx.fresh_constant("x", locations);
  z3::expr l = ctx.fresh_constant("l", locks);
  // insert a lock for testing
  z3::solver slv(ctx);
  slv.add(succ_def);
  slv.add(inl_def);
  slv.add(lock_consistency);
  slv.add(z3::forall(x,l,lock(x,l)==z3::ite(x==location_vector[0][2]&&l==lock_vector[0],ztrue,zfalse)));
  z3::check_result res = slv.check();
  cout << res << endl;
  if (res == z3::sat) {
    z3::model model = slv.get_model();
    print_func_interp(model, succ);
    print_func_interp(model, lock);
    print_func_interp(model, unlock);
    print_func_interp(model, inls);
    print_func_interp(model, inl);
    print_func_interp(model, inle);
  }*/
  //cout << slv.get_model() << endl;
  //z3::model model = slv.get_model();
  //cout << model.get_func_interp(unlock) << endl;
}

void place_locks::init_successors()
{
  // SUCCESSORS
  succ = ctx.function("succ", locations, locations, ctx.bool_sort());
  
  // we need to give a complete definition of this function
  succ_def = ztrue;
  unsigned i = 0;
  for (auto it = threads.begin(); it!=threads.end(); ++it, ++i) {
    const cfg::abstract_cfg& thread = **it;
    for (unsigned j = 1; j <= thread.no_states(); ++j) { 
      unordered_set<state_id> successors = thread.get_forward_successors(j);
      for (unsigned a = 1; a <= thread.no_states(); ++a) { 
        if (successors.find(a) != successors.end())
          succ_def = succ_def && succ(location_vector[i][j], location_vector[i][a]) == ztrue;
        else 
          succ_def = succ_def && succ(location_vector[i][j], location_vector[i][a]) == zfalse;
      }
      // other threads cannot be successors
      unsigned b = 0;
      for (auto it2 = threads.begin(); it2!=threads.end(); ++it2, ++b ) {
        if (it2 != it) {
          for (unsigned a = 1; a <= (*it2)->no_states(); ++a) {
            succ_def = succ_def && succ(location_vector[i][j], location_vector[b][a]) == zfalse;
          }
        }
      }
    }
  }
}

void place_locks::init_locks()
{
  vector<const char*> lock_names;
  unsigned i = 1;
  for (i = 1; i<=3; ++i) {
    string name = "lock_" + to_string(i);
    const char* namec = strdup(name.c_str());
    lock_names.push_back(namec);
  }
  z3::func_decl_vector consts(ctx);
  z3::func_decl_vector ts(ctx);
  locks = ctx.enumeration_sort("locks", lock_names.size(), &lock_names[0], consts, ts);
  // delete the name array once again
  for (const char* p : lock_names) {
    delete p;
  }
  
  unsigned index = 0;
  for (unsigned index = 0; index < lock_names.size(); ++index) {
    lock_vector.push_back(consts[index]());
  }
  
  // lock and unlock functions
  lock = ctx.function("lock", locations, locks, ctx.bool_sort());
  unlock = ctx.function("unlock", locations, locks, ctx.bool_sort());
  
  inl = ctx.function("inl", locations, locks, ctx.bool_sort());
  inle = ctx.function("inle", locations, locks, ctx.bool_sort());
  inls = ctx.function("inls", locations, locks, ctx.bool_sort());
  
  inl_def = ztrue;
  z3::expr x = ctx.fresh_constant("x", locations);
  z3::expr xp = ctx.fresh_constant("xp", locations); // x'
  z3::expr l = ctx.fresh_constant("l", locks);
  inl_def = inl_def && z3::forall(x, l, inls(x,l) == z3::exists(xp, succ(xp,x)==ztrue && inle(xp, l)==ztrue));
  inl_def = inl_def && z3::forall(x, l, inl(x,l) == (lock(x,l) || inls(x, l)));
  inl_def = inl_def && z3::forall(x, l, inle(x,l) == (inl(x, l) && !unlock(x,l)));
}

void place_locks::init_consistancy()
{
  lock_consistency = ztrue;
  // predecessor rule
  unsigned t = 0;
  for (const cfg::abstract_cfg* thread : threads) {
    // make a predecessor map
    vector<unordered_set<state_id>> predecessors(thread->no_states()+1);
    
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      for (const cfg::edge& e : thread->get_successors(i)) {
        predecessors[e.to].insert(i);
      }
    }
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      if (predecessors[i].size()>1) {
        // for all locks
        for (z3::expr& l : lock_vector) {
          // for all precessors, either they hold l or they don't
          z3::expr all_locked = ztrue;
          z3::expr all_unlocked = ztrue;
          for (state_id pred : predecessors[i]) {
            all_locked = all_locked && inle(location_vector[t][pred],l);
            all_unlocked = all_unlocked && !inle(location_vector[t][pred],l);
          }
          lock_consistency = lock_consistency && (all_locked || all_unlocked);
        }
      }
    }
    ++t;
  }
  
  z3::expr x = ctx.fresh_constant("x", locations);
  z3::expr xp = ctx.fresh_constant("xp", locations); // x'
  z3::expr l = ctx.fresh_constant("l", locks);
  lock_consistency = lock_consistency && z3::forall(x, l, implies(unlock(x,l), inl(x,l) ));
  lock_consistency = lock_consistency && z3::forall(x, l, implies(lock(x,l), !inls(x,l) ));
  lock_consistency = lock_consistency && z3::forall(x, l, implies(lock(x,l) , !z3::exists (xp, succ(xp,x) && unlock(xp,l) ) ));
  
}


