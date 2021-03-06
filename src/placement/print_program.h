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

#ifndef PLACEMENT_PRINT_PROGRAM_H
#define PLACEMENT_PRINT_PROGRAM_H

#include "cfg/program.h"
#include "abstraction/location.h"
#include <vector>
#include <string>
#include <unordered_set>
#include "place_locks.h"

namespace clang {
  class Rewriter;
}

namespace placement {

class print_program
{
public:
  print_program(const cfg::program& program);
  
  void print_original(const std::string& outname);
  /**
   * @brief Prints the program with all locations where locks are allowed
   */  
  void print_test_locations(const std::string& outname);
  void print_with_locks(std::vector<single_placement> locks_to_place, const std::string& outname);
private:
  
  void place_locks(clang::Rewriter& rewriter, const single_placement& lock, std::unordered_set<clang::Stmt*>& added_brace);
  void place_text(clang::Rewriter& rewriter, const cfg::state& state, const std::string& text, bool after, std::unordered_set<clang::Stmt*>& added_brace);
  void place_lock_decl(clang::Rewriter& rewriter, const std::unordered_set< unsigned int >& locks_in_use);
  
  const cfg::program& program;
  
  const std::string lock_name = "synthlock_";
  const std::string unlock_instr = "unlock_s";
  const std::string lock_instr = "lock_s";
  const std::string lock_t = "lock_t";
};
}

#endif // PLACEMENT_PRINT_PROGRAM_H
