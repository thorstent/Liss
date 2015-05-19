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

#ifndef ACTIONS_SYNTHESIS2_H
#define ACTIONS_SYNTHESIS2_H

#include "action_base.h"
#include <Limi/results.h>
#include "abstraction/symbol.h"

namespace clang {
  class Rewriter;
  class Decl;
  class ASTContext;
}

namespace actions {

class synthesis2 : public actions::action_base
{
public:
  virtual void run(const cfg::program& program, clang::CompilerInstance& compiler) override;
private:
  bool synth_loop(const cfg::program& program);
  bool is_local(clang::Decl* d, clang::ASTContext& ast_context);
  Limi::inclusion_result<abstraction::psymbol> result; // so we can access this if synthesis goes wrong
  unsigned max_bound; // maximum bound during the verification run
  void print_summary(const cfg::program& original_program);
};
}

#endif // ACTIONS_SYNTHESIS2_H
