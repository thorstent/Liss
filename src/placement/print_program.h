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

#ifndef PLACEMENT_PRINT_PROGRAM_H
#define PLACEMENT_PRINT_PROGRAM_H

#include "cfg/program.h"
#include "location.h"
#include <vector>
#include <unordered_set>

namespace clang {
  class Rewriter;
}

namespace placement {

class print_program
{
public:
  print_program(const cfg::program& program);
  
  void print_with_locks(const std::vector<std::pair<unsigned,placement::location>>& locks, const std::vector<std::pair<unsigned,placement::location>>& unlocks, const std::string& outname);
private:
  void place_locks(clang::Rewriter& rewriter, const std::vector< std::pair< unsigned, placement::location > >& locks, const std::string name, bool after, std::unordered_set<clang::Stmt*>& added_brace);
  /**
   * @brief Finds the parent and if additional braces are needed
   */
  std::pair<clang::Stmt*,bool> find_stmt_parent(clang::Stmt* stmt,clang::Stmt* function);
  
  const cfg::program& program;
};
}

#endif // PLACEMENT_PRINT_PROGRAM_H
