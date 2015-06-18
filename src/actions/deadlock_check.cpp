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

#include "deadlock_check.h"

#define DEBUG_PRINTING verbosity

#include "options.h"
#include "automata/deadlock_algo.h"
#include "abstraction/concurrent_automaton.h"
#include <iostream>
#include <chrono>


using namespace actions;
using namespace std;

bool actions::test_deadlock(const cfg::program& program, deadlock_result& result, bool concurrent)
{
  abstraction::concurrent_automaton automaton(program, concurrent, false, true);
  return test_deadlock(automaton, result);
}

bool actions::test_deadlock(const abstraction::concurrent_automaton& automaton, deadlock_result& result)
{
  automata::deadlock_algo<abstraction::pcstate,abstraction::psymbol,abstraction::concurrent_automaton> algo;
  result = algo.run(automaton);
  return true;
}

void print_summary(chrono::milliseconds time) {
  debug << "Deadlock check " << (double)time.count()/1000 << "s" << endl;
  //cout.precision(1);
  debug << " | " << (double)time.count()/1000 << "s | ";
}

void deadlock_check::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  auto start = chrono::steady_clock::now();
  test(program, false);
  test(program, true);  
  
  auto end = chrono::steady_clock::now();
  print_summary(std::chrono::duration_cast<chrono::milliseconds>(end - start));  
}


void deadlock_check::test(const cfg::program& program, bool concurrent)
{
  deadlock_result result;
  if (verbosity>=1) { debug << "Testing ";
  if (concurrent) debug << "concurrent"; else debug << "sequential";
  debug << " automaton" << endl; }
  abstraction::concurrent_automaton automaton(program, concurrent, false, true);
  test_deadlock(automaton, result);
  if (concurrent) cout << "Concurrent"; else cout << "Sequential";
  cout << " automaton: ";
  result.print_long(cout, automaton.symbol_printer(), automaton.state_printer());
  cout << endl << endl;
}

