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

#ifndef CLANG_INTERF_STATEMENT_VISITOR_H
#define CLANG_INTERF_STATEMENT_VISITOR_H

#include <string>
#include "pch.h"
#include <unordered_set>

#include "cfg/abstract_cfg.h"
#include "abstraction/identifier_store.h"


namespace clang_interf {

class statement_visitor
: public clang::RecursiveASTVisitor<statement_visitor> {
public:
  /**
   * @brief ...
   * 
   * @param seen_stmt Prevents us from traversing statements we already saw during the 
   * exploration of the cfg block
   * 
   */
  statement_visitor(clang::ASTContext& context, cfg::abstract_cfg& thread,
                    abstraction::identifier_store& identifier_store,
                    std::unordered_set<const clang::Stmt*>& seen_stmt,
                    clang::Stmt* function, bool writer = false) :
                    context(context), thread(thread), identifier_store(identifier_store),
                    access_type(writer ? abstraction::op_class::write : abstraction::op_class::read),
                    function(function), seen_stmt(seen_stmt) {}
  
  inline bool shouldUseDataRecursionFor(clang::Stmt* s) { return false; }
  
  bool VisitDeclRefExpr(clang::DeclRefExpr* s);
  
  bool TraverseBinAssign(clang::BinaryOperator* stmt);
  
  bool TraverseCallExpr(clang::CallExpr* s);
  bool TraverseStmt(clang::Stmt* s);
  
  state_id_type last_state();
  state_id_type first_state();
  inline bool progress() { return start_state != no_state; }
  /**
   * @brief Means that the last instruction referenced non-det
   * 
   */  
  bool last_nondet = false;
private:
  clang::ASTContext& context;
  cfg::abstract_cfg& thread;
  abstraction::identifier_store& identifier_store;
  state_id_type start_state = no_state;
  state_id_type end_state = no_state;
  abstraction::op_class access_type;

  void add_successor(state_id_type successor);
  std::string get_type_name(clang::DeclRefExpr* decl);
  clang::Stmt* function;
  std::unordered_set<const clang::Stmt*>& seen_stmt;
  void check_mainfile(clang::Stmt* stmt, state_id_type state); // disable lock placement if not main file
};
}

#endif // CLANG_INTERF_STATEMENT_VISITOR_H
