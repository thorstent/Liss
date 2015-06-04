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

#ifndef PLACEMENT_LOCK_LOCATIONS_H
#define PLACEMENT_LOCK_LOCATIONS_H

#include "types.h"
#include <vector>
#include "abstraction/location.h"
#include "abstraction/symbol.h"
#include "synthesis/lock.h"

namespace placement {

  /**
   * @brief This type describes a CNF of locks needed.
   *
   * Every atom in the cnf is a vector of places to lock
   * and for each place is a vector of locations inside that lock
   * 
   */  
  using lock_locations = cnf<std::vector<std::vector<abstraction::location>>>;
  using lock_symbols = cnf<std::vector<std::vector<abstraction::psymbol>>>;
  
  lock_symbols locks_to_symbols(const cnf<::synthesis::lock>& locks, const std::vector<abstraction::psymbol>& trace);
  lock_locations symbols_to_locations(const lock_symbols& symbols);
  
}

#endif // PLACEMENT_LOCK_LOCATIONS_H
