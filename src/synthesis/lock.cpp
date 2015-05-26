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

#include "lock.h"

#include <iostream>

using namespace synthesis;
using namespace std;

void synthesis::print_lock(const lock& lock, std::ostream& out) {
  out << lock.name << " (";
  for(auto l = lock.locations.begin(); l!=lock.locations.end(); ) {
    out << l->start;
    out << "-";
    out << l->end;
    if (++l != lock.locations.end()) out << ", ";
  }
  out << ")";
}