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

#include "thread_visitor.h"

#include "statement_visitor.h"

#include "cfg_visitor.h"
#include <clang/AST/ASTContext.h>

using namespace clang_interf;

using namespace clang;
using namespace std;

void clang_interf::parse_thread(cfg::abstract_cfg& thread, abstraction::identifier_store& is, ASTContext& context)
{
  std::unique_ptr<clang::CFG> cfg = CFG::buildCFG(thread.declaration, thread.declaration->getBody(), &context, clang::CFG::BuildOptions());
  cfg_visitor visitor = cfg_visitor::process_function(context, thread, is, thread.declaration);
  thread.mark_final(visitor.exit_state());
  thread.get_state(visitor.entry_state()).return_state = visitor.exit_state();
}


bool thread_visitor::TraverseFunctionDecl(FunctionDecl* fd)
{
  if (fd->hasBody()) {
    if (program.first_function.isInvalid())
      program.first_function = fd->getSourceRange().getBegin();
    std::string name = fd->getNameInfo().getAsString();
    
    if (cfg::program::is_thread_name(name)) {
      cfg::abstract_cfg* thread = new cfg::abstract_cfg(fd, program.no_threads());
      parse_thread(*thread, program.identifiers(), context);
      
      program.add_thread(thread);
    }
  }
      
  return true;
}
