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

#ifndef PLACEMENT_LOCK_LOCATIONS_H
#define PLACEMENT_LOCK_LOCATIONS_H

#include "types.h"
#include <vector>
#include <ostream>
#include "abstraction/location.h"
#include "abstraction/symbol.h"
#include "synthesis/lock.h"
#include "cfg/program.h"

namespace synthesis {

  /**
   * @brief This type describes a CNF of locks needed.
   *
   * Every atom in the cnf is a vector of places to lock
   * and for each place is a vector of locations inside that lock
   * 
   */  
  using lock_list = std::vector<abstraction::psymbol>;
  using lock_lists = std::vector<lock_list>;
  using lock_symbols = cnf<lock_lists>;
  
  
  lock_symbols locks_to_symbols(const cnf<::synthesis::lock>& locks, const std::vector<abstraction::psymbol>& trace);
  
  /**
   * @brief Add symbols (function call/return) to the trace
   * 
   * The trace is based on the minimized automaton. This function adds the in-between states that are not in the minimized automaton.
   */
  void blow_up_trace(const cfg::program& program, std::vector< abstraction::psymbol >& trace);
  
  /**
   * @brief Add symbols (function call/return) to the locks
   * 
   * The trace is based on the minimized automaton. This function adds the in-between states that are not in the minimized automaton.
   */
  void blow_up_lock(const cfg::program& program, lock_symbols& locks);
  
  /**
   * @brief Remove preemption points from the lock.
   * 
   * This will also split locks with preemption points in between
   */
  void remove_preemption(lock_symbols& locks);
  
  /**
   * @brief A very simple algorithm that removes duplicates between locks
   */
  void remove_duplicates(lock_symbols& locks);
}


std::ostream& operator<<(std::ostream& out, const synthesis::lock_lists& lock);

#endif // PLACEMENT_LOCK_LOCATIONS_H
