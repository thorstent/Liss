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

#include "clang_helpers.h"

#include <clang/AST/Stmt.h>
#include <clang/AST/ParentMap.h>
#include <clang/AST/Expr.h>

using namespace clang;

namespace clang_interf {
  bool ends_semicolon(const Stmt* stmt) {
    if (isa<IfStmt>(stmt)) 
      return false;
    if (isa<WhileStmt>(stmt)) 
      return false;
    return true;
  }
  
  parent_result find_stmt_parent(Stmt* stmt, Stmt* function)
  {
    ParentMap map(function);
    while (true) {
      assert(stmt);
      Stmt* parent = map.getParent(stmt);
      if (isa<CompoundStmt>(parent)) {
        return parent_result(stmt, false);
      } else if (IfStmt* ifs = dyn_cast<IfStmt>(parent)) {
        if (stmt!=ifs->getCond())
          return parent_result(stmt, true);
        else
          return parent_result();
      } else if (WhileStmt* whiles = dyn_cast<WhileStmt>(parent)) {
        if (stmt!=whiles->getCond())
          return parent_result(stmt, true);
        else 
          return parent_result();
      } else if (DoStmt* dos = dyn_cast<DoStmt>(parent)) {
        if (stmt!=dos->getCond()) 
          return parent_result(stmt, true);
        else 
          return parent_result();
      }
      stmt = parent;
    }
  }
}