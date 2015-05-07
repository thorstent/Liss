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

#ifndef SYNTHESIS_REORDERING_H
#define SYNTHESIS_REORDERING_H

#include <string>
#include "lock.h"
#include <ostream>

namespace synthesis {

struct reordering
{
  reordering(std::string name, lock_location location): name(name), location(location) {}
  std::string name;
  lock_location location;
};


void print_reordering(const reordering& reordering, const Limi::printer< abstraction::pcsymbol >& symbol_printer, std::ostream& out);
}

#endif // SYNTHESIS_REORDERING_H
