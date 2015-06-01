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
#include "synthesis/find_locks.h"
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
  
  string file_name = main_filename;
  file_name.replace(file_name.length()-2,2, ".start.c");
  pprogram.print_original(debug_folder + file_name);
  pprogram.print_original(start_file_code);
  
  vector<pair<unsigned,abstraction::location>> locks, unlocks;
  bool success = synth_loop(program, locks, unlocks);
  

  
  if (success) {
    file_name = main_filename;
    file_name.replace(file_name.length()-2,2, ".end.c");
    pprogram.print_with_locks(locks, unlocks, debug_folder + file_name);
    
  } else {
    Limi::printer<abstraction::psymbol> symbol_printer;
    ofstream file_out(output_file_log);
    result.print_long(file_out, symbol_printer);
    file_out.close();
  }
}

unsigned iteration = 0;
chrono::milliseconds langinc;
chrono::milliseconds verification;
chrono::milliseconds synthesis_time;
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
  //cout.precision(1);
  debug << "| " << original_program.no_threads() << " | " << iteration << " | " << this->max_bound <<  " | " << (double)langinc.count()/1000 << "s | "  << (double)synthesis_time.count()/1000 << "s | " << (double)verification.count()/1000 << "s |";
}

cnf<vector<vector<abstraction::location>>> locks_to_locations(const cnf<::synthesis::lock>& locks, const vector<abstraction::psymbol>& trace) {
  cnf<vector<vector<abstraction::location>>> result;
  for (const disj<::synthesis::lock>& d : locks) {
    result.emplace_back();
    for (::synthesis::lock l : d) {
      result.back().emplace_back();
      bool started = false;
      for (::synthesis::lock_location loc : l.locations) {
        result.back().back().emplace_back();
        assert (loc.start.instruction_id() <= loc.end.instruction_id());
        for (unsigned i = loc.start.instruction_id(); i <= loc.end.instruction_id(); ++i) {
          if (trace[i]->thread_id()==loc.start.thread_id())
            result.back().back().back().push_back(abstraction::location(trace[i]->thread_id(), trace[i]->state_id()));
        }
      }
    }
  }
  return result;
}

bool actions::synthesis2::synth_loop(const cfg::program& program, vector<pair<unsigned,abstraction::location>>& locks, vector<pair<unsigned,abstraction::location>>& unlocks)
{
  z3::context ctx,ctx2;

  Limi::printer<abstraction::psymbol> symbol_printer;
  abstraction::concurrent_automaton sequential(program, false, true);
  abstraction::concurrent_automaton concurrent(program, true, false);
  concurrent.use_cache = false; // do not use cache as it interfers with disallowing bad traces

  
  unsigned counter = 0;
  ::synthesis::reorderings reorder(ctx, program);
  ::synthesis::synchronisation synch(program);
  cnf<vector<vector<abstraction::location>>> lock_locations;
  while (true) {
    auto langinc_start = chrono::steady_clock::now();
    auto start = chrono::steady_clock::now();
    // do language inclusion test
    bool success = test_inclusion(sequential, concurrent, result);
    this->max_bound = max(result.max_bound,this->max_bound);
    auto langinc_end = chrono::steady_clock::now();
    if (!success) {
      cout << "Found no counter-example up to maximum bound; synthesis cannot proceed." << endl;
      return false;
    }
    if (verbosity >= 1) {
      result.print_long(debug, symbol_printer);
    }
    if (!result.included) {
      auto synth_start = chrono::steady_clock::now();
      ::synthesis::concurrent_trace trace = ::synthesis::make_trace(ctx, program, result.counter_example);
      
      pair<::synthesis::dnf_constr,::synthesis::dnf_constr> dnf = reorder.process_trace(trace);
      //::synthesis::find_locks find_locks(ctx, program);
      //find_locks.process_trace(trace);
      ::synthesis::dnf_constr bad_cond = dnf.first; // these conditions make the trace bad
      if (verbosity>=1)
        debug << "Found constraints to eliminate bad traces" << endl;
      concurrent.add_forbidden_traces(bad_cond);
      // synthesis of locks for these constraints
      ::synthesis::cnf_constr cnf_weak = negate_dnf(dnf.second);
      cnf<::synthesis::lock> new_locks;
      synch.generate_sync(cnf_weak, new_locks);
      cnf<vector<vector<abstraction::location>>> new_lock_locations = locks_to_locations(new_locks, result.counter_example);
      lock_locations.insert(lock_locations.end(), new_lock_locations.begin(), new_lock_locations.end());
      ++counter;
      print_time(start);
      langinc += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
      debug << endl;
    } else {
      verification += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
      
      if (!lock_locations.empty()) {
        placement::place_locks plocks(program);
        if (!plocks.find_locks(lock_locations, locks, unlocks)) {
          cout << "Found no valid lock placement" << endl;
          return false;
        }
      }
      
      cout << "Synthesis was successful." << endl;
      print_summary(program);
      return true;
    }
  }
      
  return false;
}
