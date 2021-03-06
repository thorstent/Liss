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

#ifndef SYNTHESIS_LOCATION_H
#define SYNTHESIS_LOCATION_H

#include <z3++.h>
#include "abstraction/symbol.h"
#include <cassert>
#include <ostream>

namespace synthesis {
  
struct location
{
  z3::expr name;
  std::string name_str;
  abstraction::psymbol symbol;
  std::string description;
  unsigned iteration = 0; // the loop iteration of this symbol
  inline thread_id_type thread_id() const { assert(symbol_valid); return symbol->thread_id(); }
  int original_position = -1; // position in the original trace (-1 means not in original trace) (needed only for debug purposes)
  inline unsigned instruction_id() const { assert(original_position>=0); return static_cast<unsigned>(original_position); }
  location(z3::expr name, std::string name_str, const abstraction::psymbol symbol, int original_position) : name(name), name_str(name_str), symbol(symbol), original_position(original_position), symbol_valid(true)  {}
  location(z3::expr name, std::string description) : name(name), description(description)  {}
  operator z3::expr () const { return name; }
  
  z3::expr operator<(const location &other) const { return static_cast<z3::expr>(*this) < other; };
  z3::expr operator>(const location &other) const { return static_cast<z3::expr>(*this) > other; };
  
  bool operator==(const location& other) const { return static_cast<Z3_ast>(name)==static_cast<Z3_ast>(other.name); }
  // TODO: This is a bit dubious
  bool symbol_valid = false;
};

typedef std::pair<const location*, const location*> loc_pair;

std::ostream& operator<<(std::ostream& out, const location& loc);
}

#endif // SYNTHESIS_LOCATION_H