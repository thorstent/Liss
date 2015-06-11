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

#ifndef CLANG_INTERF_CLANG_HELPERS_H
#define CLANG_INTERF_CLANG_HELPERS_H

namespace clang {
  class Stmt;
}

namespace clang_interf {
  struct parent_result {
    clang::Stmt* stmt_to_lock;
    bool braces_needed;
    parent_result(clang::Stmt* stmt_to_lock, bool braces_needed) : stmt_to_lock(stmt_to_lock),
    braces_needed(braces_needed)  {}
    parent_result() : stmt_to_lock(nullptr), braces_needed(false) {}
  };
  
  /**
   * @brief Finds the parent and if additional braces are needed
   * 
   * @returns stmt_to_lock may be null if lock placement is not allowed here
   */
  parent_result find_stmt_parent(clang::Stmt* stmt, clang::Stmt* function);
  
  bool ends_semicolon(const clang::Stmt* stmt);
}

#endif