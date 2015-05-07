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

#ifndef CLANG_INTERF_FUNCTION_VISITOR_H
#define CLANG_INTERF_FUNCTION_VISITOR_H

#include "pch.h"
#include "statement_visitor.h"
#include "cfg/abstract_cfg.h"
#include "cfg/program.h"

namespace clang_interf {

void parse_thread(cfg::abstract_cfg& thread, abstraction::identifier_store& is, clang::ASTContext& context);
  
class thread_visitor: public clang::RecursiveASTVisitor<thread_visitor> {
public:
  thread_visitor(clang::ASTContext& context, cfg::program& program) : context(context), program(program) {}
  bool TraverseFunctionDecl(clang::FunctionDecl *d);
private:
  clang::ASTContext& context;
  cfg::program& program;
};
}

#endif // CLANG_INTERF_FUNCTION_VISITOR_H
