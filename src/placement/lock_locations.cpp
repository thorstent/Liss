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

#include "lock_locations.h"

using namespace placement;
using namespace synthesis;
using namespace std;

namespace placement {
lock_symbols locks_to_symbols(const cnf<::synthesis::lock>& locks, const vector<abstraction::psymbol>& trace) {
  lock_symbols result;
  for (const disj<::synthesis::lock>& d : locks) {
    result.emplace_back();
    for (::synthesis::lock l : d) {
      result.back().emplace_back();
      bool started = false;
      for (::synthesis::lock_location loc : l.locations) {
        result.back().back().emplace_back();
        assert (loc.start.instruction_id() <= loc.end.instruction_id());
        for (unsigned i = loc.start.instruction_id(); i <= loc.end.instruction_id(); ++i) {
          if (trace[i]->thread_id()==loc.start.thread_id())
            result.back().back().back().push_back(trace[i]);
        }
      }
    }
  }
  return result;
}

lock_locations symbols_to_locations(const lock_symbols& symbols) {
  lock_locations result;
  for (const auto& disj : symbols) {
    result.emplace_back();
    for (const auto& lock : disj) {
      result.back().emplace_back();
      for (const auto& list : lock) {
        result.back().back().emplace_back();
        for (const abstraction::psymbol& sy : list) {
          result.back().back().back().push_back(sy->loc);
        }
      }
    }
  }
  return result;
}
}