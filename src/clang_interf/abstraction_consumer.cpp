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

#include "abstraction_consumer.h"

#include <clang/AST/ASTContext.h>
#include "cfg/program.h"

#include "options.h"

using namespace clang_interf;

void abstraction_consumer::HandleTranslationUnit(clang::ASTContext &context) {
  cfg::program program(context);
  thread_visitor visitor(context, program);
  
  visitor.TraverseDecl(context.getTranslationUnitDecl());
  
  if (verbosity>=1)
    debug << "Running actions" << std::endl;
  
  for( actions::actionp a : actions) {
    a->run(program, compiler);
  }
}