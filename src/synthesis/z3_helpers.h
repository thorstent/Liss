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

#ifndef SYNTHESIS_Z3_HELPERS_H
#define SYNTHESIS_Z3_HELPERS_H

#include <z3++.h>
#include <vector>
#include <unordered_set>

namespace synthesis {

template <class value>
void min_unsat(z3::solver& sol, std::vector< value >& items, std::function< z3::expr(value) > translate) {
  std::vector<z3::expr> triggers;
  // translate items to expr
  for (auto i: items) {
    z3::expr trigger = sol.ctx().fresh_constant("implied_assumption", sol.ctx().bool_sort());
    triggers.push_back(trigger);
    z3::expr translation = translate(i);
    sol.add(implies(trigger,translation));
  }
  
  assert(sol.check(triggers.size(), &triggers[0])==z3::unsat);
  
  //assert(sol.check(triggers.size(), &triggers[0])==z3::unsat);
  // for many items, first do unsat core
  if (items.size() > 10) {
    z3::check_result r = sol.check(triggers.size(), &triggers[0]);
    if (r==z3::sat) {
      //std::cout << sol.get_model() << std::endl;
      return;
    }
    z3::expr_vector core = sol.unsat_core();
    std::unordered_set<Z3_ast> core_set;
    for (unsigned i = 0; i<core.size(); i++)
      core_set.insert(core[i]);
    auto t = triggers.begin();
    auto i = items.begin();
    for (; t != triggers.end() && i != items.end(); ) {
      auto found = core_set.find(*t);
      if (found == core_set.end()) {
        i = items.erase(i);
        t = triggers.erase(t);
      } else {
        i++;
        t++;
      }
    }
  }
  
  // test which expressions make the solver return unsat
  auto t = triggers.begin();
  auto i = items.begin();
  for (; t != triggers.end() && i != items.end(); ) {
    // assemble triggers to use
    z3::expr_vector assertions(sol.ctx());
    for (auto t1 = triggers.begin(); t1 != triggers.end(); t1++) {
      if (t1 != t) assertions.push_back(*t1);
    }
    if (sol.check(assertions) == z3::unsat) {
      i = items.erase(i);
      t = triggers.erase(t);
    } else {
      //std::cout << sol.get_model() << std::endl;
      i++;
      t++;
    }
  }
}

}

#endif