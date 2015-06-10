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

#ifndef PLACEMENT_PLACE_LOCKS_H
#define PLACEMENT_PLACE_LOCKS_H

#include "cfg/program.h"
#include <z3++.h>
#include <vector>
#include "abstraction/location.h"
#include "types.h"
#include "synthesis/trace_helpers.h"

namespace placement {

  struct placement_result {
    std::vector<std::pair<unsigned, abstraction::location >> locks_b; // before the instruction
    std::vector<std::pair<unsigned, abstraction::location >> locks_a; // after the instruction
    std::vector<std::pair<unsigned, abstraction::location >> unlocks_b; // before the instruction
    std::vector<std::pair<unsigned, abstraction::location >> unlocks_a; // after the instruction
  };
  
class place_locks
{
public:
  place_locks(const cfg::program& program);
  /**
   * @brief Determines the lock placement
   * 
   * @param locks_to_place List of locations that need to share the same lock
   * The innermost is a sequence of location that need to be locked without an unlock in between. The middle layer
   * are the locations that need to be locked by the same lock. There is a cnf over that
   * @param locks_placed Pair which lock and where
   * @param unlocks_placed ...
   * @return void
   */
  bool find_locks(const synthesis::lock_symbols& locks_to_place, placement_result& to_place);
private:
  void init_locks();
  void init_consistancy();
  void init_sameinstr();
  
  void result_to_locklist(const std::vector<std::vector<z3::expr>>& result, std::vector<std::pair<unsigned, abstraction::location >>& locks);
  
  // calculates the places to that need to be locked together
  std::vector< z3::expr > locked_together(const synthesis::lock_symbols& locks_to_place);

  z3::context ctx;  
  z3::expr ztrue = ctx.bool_val(true);
  z3::expr zfalse = ctx.bool_val(false);
  const std::vector<const cfg::abstract_cfg*>& threads;
  std::vector<std::vector<z3::expr>> location_vector;
  std::unordered_map<z3::expr,abstraction::location> location_map;
  z3::sort locations = z3::sort(ctx);
  
  z3::sort locks = z3::sort(ctx);
  std::vector<z3::expr> lock_vector;
  std::unordered_map<z3::expr, unsigned> lock_map;
  z3::func_decl lock_b = z3::func_decl(ctx); // lock before
  z3::func_decl unlock_b = z3::func_decl(ctx); // unlock before
  z3::func_decl lock_a = z3::func_decl(ctx); // lock after
  z3::func_decl unlock_a = z3::func_decl(ctx); // lock before
  
  z3::expr cost = ctx.int_const("cost");
  z3::expr cost_def = ztrue;
  
  z3::func_decl inl = z3::func_decl(ctx);
  z3::expr inl_def = ztrue;
  
  z3::expr cons_loc = ztrue; // what locations may be locked or unlocked respectively
  z3::expr cons_preemption = ztrue; // do not lock preemption points
  z3::expr cons_basic = ztrue; // basic rule that nothing can be locked and unlocked at the same time
  z3::expr cons_join = ztrue; // predecessors need the same locking on join
  z3::expr cons_unl = ztrue; // to be unlocked it needs to be locked first
  z3::expr cons_lo = ztrue; // to be locked it needs to be unlocked first
  z3::expr cons_functions = ztrue; // ensure functions start and end with the same locking configuration
  z3::expr cons_sameinstr = ztrue; // same instructions need to be in the same lock
  
  z3::expr inle(const z3::expr& x, const z3::expr& l); // lock status after the instruction executed
};
}

#endif // PLACEMENT_PLACE_LOCKS_H
