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

#include "inclusion_test.h"

#include "options.h"
#define DEBUG_PRINTING verbosity

#include <Limi/antichain_algo_ind.h>
#include <Limi/list_automaton.h>
#include "abstraction/concurrent_automaton.h"


#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Stmt.h>
#include <llvm/Support/raw_os_ostream.h>

#include <iostream>

using namespace actions;
using namespace std;

Limi::inclusion_result< abstraction::pcsymbol > check_trace(std::list<abstraction::pcsymbol> trace,
                                                                const abstraction::concurrent_automaton& sequential,
                                                                const Limi::printer_base< abstraction::pcsymbol >& symbol_printer
) {
  Limi::list_automaton<abstraction::pcsymbol> check_concurrent(trace.begin(), trace.end(), symbol_printer);
  
  abstraction::concurrent_automaton seq2(sequential);
  seq2.use_cache = false;
  
  unsigned bound = 0;
  for (const abstraction::pcsymbol sy : trace) {
    if (!sy.symbol->is_epsilon()) {
      ++bound;
      seq2.successor_filter.insert(sy);
    }
  }
  if (bound > 10) bound = (bound / 2) + 1; // only half is needed
  Limi::antichain_algo_ind<unsigned,abstraction::pcstate,abstraction::pcsymbol,Limi::list_automaton<abstraction::pcsymbol>,abstraction::concurrent_automaton> algo_check(check_concurrent, seq2, trace.size());
  inclusion_result res = algo_check.run();
  /*cout << "---->" << endl;
  res.print_long(cout, symbol_printer);
  cout << "<----" << endl;*/
  assert (!res.bound_hit);
  return res;
}

bool actions::test_inclusion(const cfg::program& program, inclusion_result& result) {
  return test_inclusion(program, program, result);
}

void actions::filter_result(inclusion_result& result) {
  result.filter_trace([](const abstraction::pcsymbol& sym){return sym.symbol->tag_state!=no_state;});
}

bool actions::test_inclusion(const abstraction::concurrent_automaton& sequential, const cfg::program& concurrent_program, inclusion_result& result)
{
  abstraction::concurrent_automaton concurrent(concurrent_program, true, false);
  
  unsigned bound = 1;
  Limi::antichain_algo_ind<abstraction::pcstate,abstraction::pcstate,abstraction::pcsymbol,abstraction::concurrent_automaton,abstraction::concurrent_automaton> algo(concurrent, sequential, bound);
  while (bound <= max_bound) {
    result = algo.run();
    if (result.included) {
      filter_result(result);
      return true;
    }
    unsigned i = 0;
    while (!result.included && i<1) {
      i++;
      if (!result.bound_hit) {
#ifdef SANITY
        if (verbosity >= 1)
          debug << "Checking if the trace is actually a counter example" << endl;
        Limi::inclusion_result< abstraction::pcsymbol > result_check = check_trace(result.counter_example, sequential, concurrent.symbol_printer());
        if (result_check.included) {
          cerr << endl << endl << "SANITY: The trace found is not a counter example" << endl;
        }
#endif
        filter_result(result);
        return true;
      }
      if (verbosity>=1)
        debug << "Found candidate ... Checking if truely a counter-example(size=" << result.counter_example.size() << ")" << endl;
      // is this result real?
      Limi::inclusion_result< abstraction::pcsymbol > result_check = check_trace(result.counter_example, sequential, concurrent.symbol_printer());
      if (!result_check.included) {
        filter_result(result);
        return true;
      }
      if (verbosity>=1) {
        debug << "Candidate is not a counter-example" << endl;
      }
      if (verbosity>=2) {
        result.print_long(debug, concurrent.symbol_printer());
      }
      //result = algo.run();
    }
    algo.increase_bound(++bound);
    if (verbosity>=1 && bound <= max_bound)
      debug << "Increasing the bound to " << bound << endl;
  }
  
  
  
  return false;
}


bool actions::test_inclusion(const cfg::program& sequential_program, const cfg::program& concurrent_program, inclusion_result& result)
{
  
  abstraction::concurrent_automaton sequential(sequential_program, false, true);
  return test_inclusion(sequential, concurrent_program, result);
}


void inclusion_test::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  Limi::printer<abstraction::pcsymbol> symbol_printer;
  inclusion_result result;
  bool success = test_inclusion(program, result);
  
  if (success) {
    result.print_long(cout, symbol_printer);
  } else {
    cout << "Found no counter-example up to maximum bound." << endl;
  }
  
}
