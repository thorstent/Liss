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

#ifndef SYNTHESIS_LOCK_H
#define SYNTHESIS_LOCK_H
#include "constraint.h"
#include <list>
#include <ostream>
#include <unordered_set>
#include <Limi/generics.h>

namespace clang {
  class Stmt;
  class CompoundStmt;
  class VarDecl;
}

namespace synthesis {
  
  struct lock_location {
    location start;
    location end;
    clang::Stmt* starts = nullptr;
    clang::Stmt* ends = nullptr;
    std::unordered_set<clang::Stmt*> locations;
    clang::CompoundStmt* parent;
    clang::Stmt* inserted_start = nullptr;
    clang::Stmt* inserted_end = nullptr;
    lock_location(const location start, const location end) : start(start), end(end) {}
  };
  typedef std::list<lock_location> lock_locations;
  
  struct lock {
    lock(std::string name): name(name){}
    std::string name;
    lock_locations locations;
    clang::VarDecl* declaration = nullptr;
  };
  
  std::ostream& operator<<(std::ostream& out, const lock& lock);
  
}

#endif