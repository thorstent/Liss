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

#ifndef ACTIONS_SYNTHESIS2_H
#define ACTIONS_SYNTHESIS2_H

#include "action_base.h"
#include <Limi/results.h>
#include "abstraction/symbol.h"

#include "synthesis/trace_helpers.h"
#include "placement/place_locks.h"

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
  /**
   * @brief Determines the conflicts in the program
   * 
   * @param program The input program
   * @param conflicts a CNF of conflicts found (empty if input program correct)
   * @return true if successful, false if language inclusion check or conflict finding failed
   */
  bool synth_loop(const cfg::program& program, synthesis::lock_symbols& conflicts);
  unsigned max_bound; // maximum bound during the verification run
  void print_summary(const cfg::program& original_program, unsigned int conflicts);
};
}

#endif // ACTIONS_SYNTHESIS2_H
