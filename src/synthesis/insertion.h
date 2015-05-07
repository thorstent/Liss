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

#ifndef SYNTHESIS_INSERTION_H
#define SYNTHESIS_INSERTION_H

#include "lock.h"
#include "reordering.h"
#include <list>
#include <unordered_set>
#include "abstraction/concurrent_automaton.h"

namespace clang {
  class Rewriter;
  class CompoundStmt;
  class Stmt;
  class Decl;
  class VarDecl;
}

namespace synthesis {

class insertion
{
public:
  insertion(cfg::program& program);
  void add_locks(std::list<lock>& locks);
  void remove_locks(std::list<lock>& locks);
  void apply_reorderings(std::list<reordering>& reorderings);
private:
  cfg::program& program;
  const Limi::printer<abstraction::pcsymbol> symbol_printer;
  /**
   * @brief Find the position in the code where to place a lock
   * 
   * @param stack1 The start location of the lock
   * @param stack2 The end location of the lock
   * @param locked_list The list of locations that should be locked (if found)
   * @param function The common function of the above locations
   * @return bool if a lock was found
   */
  bool find_parent(call_stack& stack1, call_stack& stack2, std::unordered_set< clang::Stmt* >& locked_list, clang::CompoundStmt*& common_parent, clang::Stmt*& start, clang::Stmt*& end);
    bool find_parent(synthesis::lock_location& lp);
  clang::Decl* find_declaration(std::string name);
  clang::VarDecl* add_declaration(std::string name, std::string type);
  clang::Stmt* insert_call(std::string function, clang::VarDecl* var_decl, clang::CompoundStmt* parent, bool after, clang::Stmt* ref_point);
  void move_down(clang::CompoundStmt* parent, clang::Stmt* first, clang::Stmt* second);
  void remove_stmt(clang::CompoundStmt* parent, clang::Stmt* stmt);
  void merge_locks(std::list<lock>& locks);
  bool merge_overlap(lock_location& l1, const lock_location& l2);
  void merge_reorderings(std::list<reordering>& reorderings);
  void print_lp(const lock_location& lp, std::ostream& out);
  void print_locks(const std::list<lock>& locks, std::ostream& out);
};
}

#endif // SYNTHESIS_INSERTION_H
