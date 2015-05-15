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

#ifndef SYNTHESIS_SYNCHRONISATION_H
#define SYNTHESIS_SYNCHRONISATION_H
#include "constraint.h"
#include <list>
#include "cfg/program.h"
#include "abstraction/concurrent_state.h"

#include "lock.h"
#include "reordering.h"
#include "trace.h"

namespace synthesis {
  
class synchronisation
{
public:
  synchronisation(const cfg::program& program, const concurrent_trace& trace);
  void generate_sync(const synthesis::cnf& cnf, std::list< synthesis::lock >& locks, std::list< synthesis::reordering >& reorderings, bool prefer_reorder);
private:
  //const abstraction::program& program;
  const Limi::printer<abstraction::psymbol> symbol_printer;
  const concurrent_trace& trace;
  bool find_lock(const disj& disjunct, std::list<lock>& locks);
  bool find_reordering(const synthesis::disj& disjunct, std::list< synthesis::reordering >& reorderings);
  void merge_locks(std::list<lock>& locks);
  //void merge_locks_multithread(std::list<lock>& locks);
  
  /**
   * @brief Checks if two pairs of locations overlap
   */
  bool check_overlap(const lock_location& locs1, const lock_location& locs2);
  
  /**
   * @brief Checks if one pair is contained in the other
   */
  bool check_contained(const lock_location& locs1, const lock_location& locs2);
  
  /**
   * @brief Merge into the existing locs1 the second locs, so that they cover both intervals
   */
  void merge_overlap(lock_location& locs1, const lock_location& locs2);
};
}

#endif // SYNTHESIS_SYNCHRONISATION_H
