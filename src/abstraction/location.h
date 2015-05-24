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

#ifndef PLACEMENT_LOCATION_H
#define PLACEMENT_LOCATION_H

#include "types.h"
#include <Limi/internal/hash.h>

namespace abstraction {
  struct location {
    thread_id_type thread;
    state_id_type state;
    location(thread_id_type thread, state_id_type state) : thread(thread), state(state) {      
    }
    location() : thread(no_thread), state(no_state) {}
    bool operator==(const location& other) const {
      return state == other.state && thread == other.thread;
    }
    bool operator!=(const location& other) const {
      return !(*this==other);
    }
  };
}

namespace std {
  template<> struct hash<abstraction::location> {
    size_t operator()(const abstraction::location& val) const {
      size_t seed = val.state;
      Limi::internal::hash_combine(seed, val.thread);
      return seed;
    }
  };
}

#endif