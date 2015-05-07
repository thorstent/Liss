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

#ifndef ABSTRACTION_IDENTIFIER_STORE_H
#define ABSTRACTION_IDENTIFIER_STORE_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "types.h"

namespace clang {
  class SourceManager;
}

namespace abstraction {
  
class identifier_store
{
  std::vector<std::string> variable_names;
  std::unordered_map<std::string, variable_type> variable_map;
  
  std::vector<std::string> lock_names;
  std::unordered_map<std::string, lock_type> lock_map;
  
  std::vector<std::string> conditional_names;
  std::unordered_map<std::string, conditional_type> conditional_map;
  
public:
  identifier_store(const clang::SourceManager& source_manager) : source_manager(source_manager) {}
  inline std::string lookup_variable(variable_type var) const { return variable_names[var]; }
  inline std::string lookup_lock(lock_type l) const { return lock_names[l]; }
  inline std::string lookup_conditional(conditional_type c) const { return conditional_names[c]; }
  
  inline unsigned no_variables() const { return variable_names.size(); }
  inline unsigned no_locks() const { return lock_names.size(); }
  inline unsigned no_conditionals() const { return conditional_names.size(); }
  
  variable_type insert_variable(std::string variable);
  lock_type insert_lock(std::string lock);
  conditional_type insert_conditonal(std::string conditional);
  
  const clang::SourceManager& source_manager;
};
}

#endif // ABSTRACTION_IDENTIFIER_STORE_H
