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

#ifndef ACTIONS_INCLUDED_H
#define ACTIONS_INCLUDED_H

#include "action_base.h"
#include <Limi/results.h>
#include "abstraction/symbol.h"
#include "abstraction/concurrent_automaton.h"

namespace actions {

typedef Limi::inclusion_result< abstraction::psymbol > inclusion_result;
bool test_inclusion(const cfg::program& program, inclusion_result& result);
bool test_inclusion(const cfg::program& sequential_program, const cfg::program& concurrent_program, inclusion_result& result);

/**
 * @brief Runs the language inclusion test
 * 
 * @param sequential The sequential automaton (can be reused, caching!)
 * @param concurrent The concurrent automaton
 * @param result The result will be found in here if the return value is true
 * @return True if a result was found
 */
bool test_inclusion(const abstraction::concurrent_automaton& sequential, const abstraction::concurrent_automaton& concurrent, inclusion_result& result);

void filter_result(inclusion_result& result);
  
class inclusion_test : public actions::action_base
{
public:
  virtual void run(const cfg::program& program, clang::CompilerInstance& compiler) override;
};
}

#endif // ACTIONS_INCLUDED_H
