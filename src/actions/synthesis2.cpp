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

#include "synthesis2.h"
#include <clang/AST/Decl.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTContext.h>

#include <Limi/antichain_algo.h>
#include "abstraction/concurrent_automaton.h"
#include "clang_interf/thread_visitor.h"

#include "synthesis/reorderings.h"
#include "synthesis/synchronisation.h"
#include "synthesis/lock.h"
#include "inclusion_test.h"
#include "print.h"
#include "options.h"
#include <z3++.h>
#include <chrono>
#include <fstream>
#include <algorithm>

#include "placement/place_locks.h"
#include "placement/print_program.h"

using namespace clang;
using namespace std;

void actions::synthesis2::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  placement::print_program pprogram(program);
    
  placement::placement_result result;
  bool success = synth_loop(program, result);
  
  if (success) {
    pprogram.print_with_locks(result, output_file_code);
    
  } else {

  }
}

unsigned iteration = 0;
chrono::milliseconds langinc;
chrono::milliseconds verification;
chrono::milliseconds synthesis_time;
chrono::milliseconds placement_time;
void print_time(const chrono::steady_clock::time_point& start) {
  auto stop = chrono::steady_clock::now();
  chrono::milliseconds passed = std::chrono::duration_cast<chrono::milliseconds>(stop - start);
  debug << "TIME - " <<  "Iteration " + to_string(++iteration) << ": " << (double)passed.count()/1000 << "s" << endl;
}

void actions::synthesis2::print_summary(const cfg::program& original_program) {
  debug << "Threads: " << original_program.no_threads() << endl;
  debug << "Iterations: " << iteration << endl;
  debug << "Liss: " << (double)langinc.count()/1000 << "s" << endl;
  debug << "Verification: " << (double)verification.count()/1000 << "s" << endl;
  debug << "Synthesis: " << (double)synthesis_time.count()/1000 << "s" << endl;
  debug << "Placement: " << (double)placement_time.count()/1000 << "s" << endl;
  //cout.precision(1);
  debug << original_program.no_threads() << " | " << iteration << " | " << this->max_bound <<  " | " << (double)langinc.count()/1000 << "s | "  << (double)synthesis_time.count()/1000 << "s | " << (double)verification.count()/1000 << "s | " << (double)placement_time.count()/1000 << "s";
}

bool actions::synthesis2::synth_loop(const cfg::program& program, placement::placement_result& lock_result)
{
  Limi::printer<abstraction::psymbol> symbol_printer;
  if (verbosity>=1) debug << "Building sequential automaton" << endl;
  abstraction::concurrent_automaton sequential(program, false, true);
  sequential.use_cache = false;
  abstraction::compressed_automaton<abstraction::psymbol> compressed_sequential = abstraction::from_concurrent_automaton(sequential);
  
  abstraction::concurrent_automaton concurrent(program, true, false);
  concurrent.use_cache = false; // do not use cache as it interfers with disallowing bad traces

  
  unsigned counter = 0;
  ::synthesis::reorderings reorder(program);
  ::synthesis::synchronisation synch(program);
  ::synthesis::lock_symbols lock_symbols;
  while (true) {
    auto langinc_start = chrono::steady_clock::now();
    auto start = chrono::steady_clock::now();
    // do language inclusion test
    Limi::inclusion_result<abstraction::psymbol> result;
    bool success = test_inclusion(compressed_sequential, concurrent, result);
    this->max_bound = max(result.max_bound,this->max_bound);
    auto langinc_end = chrono::steady_clock::now();
    if (!success) {
      cout << "Found no counter-example up to maximum bound; synthesis cannot proceed." << endl;
      return false;
    }
    vector<abstraction::psymbol>& trace = result.counter_example;
    //::synthesis::blow_up_trace(program, trace);
    if (verbosity >= 1) {
      result.print_long(debug, symbol_printer);
    }
    if (!result.included) {
      auto synth_start = chrono::steady_clock::now();
      
      ::synthesis::dnf_constr bad_cond = reorder.process_trace(trace, lock_symbols);
      if (verbosity>=1)
        debug << "Found constraints to eliminate bad traces" << endl;
      // synthesis of locks for these constraints
      ::synthesis::cnf_constr cnf1 = !bad_cond;
      cnf<::synthesis::lock> new_locks;
      synch.generate_sync(cnf1, new_locks);
      synthesis::lock_symbols new_lock_symbols = synthesis::locks_to_symbols(new_locks, trace);
      //cout << new_lock_symbols << endl;
      ::synthesis::remove_preemption(new_lock_symbols);
      ::synthesis::remove_duplicates(new_lock_symbols);
      cout << new_lock_symbols << endl;
      if (new_lock_symbols.empty()) {
        cout << "Found no more locks." << endl;
        return true;
      }
      concurrent.add_forbidden_traces(new_lock_symbols);
      lock_symbols.insert(lock_symbols.end(), new_lock_symbols.begin(), new_lock_symbols.end());
      auto synth_end = chrono::steady_clock::now();
      ++counter;
      print_time(start);
      langinc += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
      synthesis_time += std::chrono::duration_cast<chrono::milliseconds>(synth_end - synth_start);
      debug << endl;
    } else {
      verification += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
      
      auto placement_start = chrono::steady_clock::now();
      if (!lock_symbols.empty()) {
        // blow up the lock symbols
        ::synthesis::blow_up_lock(program, lock_symbols);
        //cout << lock_symbols << endl;
        placement::place_locks plocks(program);
        if (!plocks.find_locks(lock_symbols, lock_result)) {
          cout << "Found no valid lock placement" << endl;
          return false;
        }
      }
      auto placement_end = chrono::steady_clock::now();
      placement_time = std::chrono::duration_cast<chrono::milliseconds>(placement_end - placement_start);
      
      cout << "Synthesis was successful." << endl;
      print_summary(program);
      return true;
    }
  }
      
  return false;
}
