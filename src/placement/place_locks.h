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

namespace placement {

class place_locks
{
public:
  place_locks(const cfg::program& program);
  /**
   * @brief Determines the lock placement
   * 
   * @param locks_to_place List of locations that need to share the same lock
   * The innermost is a sequence of location that need to be locked without an unlock in between. The middle layer
   * are the locations that need to be locked by the same lock.
   * @param locks_placed Pair which lock and where
   * @param unlocks_placed ...
   * @return void
   */
  void find_locks(const std::vector< std::vector< std::vector<abstraction::location> > >& locks_to_place, std::vector< std::pair< unsigned, abstraction::location > >& locks_placed, std::vector< std::pair<unsigned, abstraction::location > >& unlocks_placed);
private:
  void init_locks();
  void init_consistancy();
  void init_sameinstr();
  
  void result_to_locklist(const std::vector<std::vector<z3::expr>>& result, std::vector<std::pair<unsigned, abstraction::location >>& locks);

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
  z3::func_decl lock = z3::func_decl(ctx);
  z3::func_decl unlock = z3::func_decl(ctx);
  
  z3::expr cost = z3::expr(ctx);
  z3::expr cost_def = z3::expr(ctx);
  
  z3::func_decl inl = z3::func_decl(ctx);
  z3::expr inl_def = z3::expr(ctx);
  
  z3::expr lock_consistency = z3::expr(ctx);
  z3::expr lock_sameinstr = z3::expr(ctx);
  
  
};
}

#endif // PLACEMENT_PLACE_LOCKS_H
