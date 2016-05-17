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

#include "identifier_store.h"

#include <stdexcept>

using namespace abstraction;
using namespace std;

variable_type identifier_store::insert_variable(std::string variable)
{
  if (variable_names.size() == max_variables)
    throw range_error("Too many variables in program");
  auto it = variable_map.find(variable);
  if (it != variable_map.end()) return it->second;
  variable_type id = variable_names.size();
  variable_names.push_back(variable);
  variable_map.insert(make_pair(variable, id));
  return id;
}

lock_type identifier_store::insert_lock(std::string lock)
{
  if (lock_names.size() == max_locks)
    throw range_error("Too many locks in program");
  auto it = lock_map.find(lock);
  if (it != lock_map.end()) return it->second;
  lock_type id = lock_names.size();
  lock_names.push_back(lock);
  lock_map.insert(make_pair(lock, id));
  return id;
}

conditional_type identifier_store::insert_conditonal(std::string conditional)
{
  if (conditional_names.size() == max_conditionals)
    throw range_error("Too many conditionals in program");
  auto it = conditional_map.find(conditional);
  if (it != conditional_map.end()) return it->second;
  conditional_type id = conditional_names.size();
  conditional_names.push_back(conditional);
  conditional_map.insert(make_pair(conditional, id));
  return id;
}
