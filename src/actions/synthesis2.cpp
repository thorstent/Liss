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
#include "synthesis/insertion.h"
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
  
  bool success = synth_loop(program);
  
  file_name = main_filename;
  file_name.replace(file_name.length()-2,2, ".end.c");
  //print_code(program, debug_folder, file_name);
  
  if (success) {
    //print_code(program, output_file_code);
  } else {
    Limi::printer<abstraction::psymbol> symbol_printer;
    ofstream file_out(output_file_log);
    result.print_long(file_out, symbol_printer);
    file_out.close();
  }
}


bool actions::synthesis2::is_local(Decl* d, ASTContext& ast_context)
{
  FileID id = ast_context.getSourceManager().getFileID(d->getLocStart());
  FileID main = ast_context.getSourceManager().getMainFileID();
  return id == main || (isa<VarDecl>(d) && d->getLocStart() == SourceLocation());
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

vector<vector<placement::location>> locks_to_locations(list<::synthesis::lock> locks, const vector<abstraction::psymbol>& trace) {
  vector<vector<placement::location>> result;
  for (::synthesis::lock l : locks) {
    result.push_back(vector<placement::location>());
    bool started = false;
    for (::synthesis::lock_location loc : l.locations) {
      assert (loc.start.instruction_id() <= loc.end.instruction_id());
      for (unsigned i = loc.start.instruction_id(); i <= loc.end.instruction_id(); ++i) {
        if (trace[i]->thread_id==loc.start.thread_id())
          result.back().push_back(placement::location(trace[i]->thread_id, trace[i]->state));
      }
    }
  }
  return result;
}

bool actions::synthesis2::synth_loop(const cfg::program& program)
{
  z3::context ctx;

  Limi::printer<abstraction::psymbol> symbol_printer;
  abstraction::concurrent_automaton sequential(program, false, true);
  
  placement::place_locks plocks(program.minimised_threads());
  placement::print_program pprogram(program);
  
  /*vector<vector<placement::location>> lock_locations;
  vector<pair<unsigned,placement::location>> locks, unlocks;
  plocks.find_locks(lock_locations, locks, unlocks);
  exit(2);*/
  
  unsigned counter = 0;
  while (true) {
    print_program(program, false, true, to_string(counter) + ".");
    
    auto langinc_start = chrono::steady_clock::now();
    auto start = chrono::steady_clock::now();
    // compare program to itself (context switches on synthesised locks are not allowed)
    bool success = test_inclusion(program, program, result);
    this->max_bound = max(result.max_bound,this->max_bound);
    auto langinc_end = chrono::steady_clock::now();
    if (!success)
      cout << "Found no counter-example up to maximum bound; synthesis cannot proceed." << endl;
    if (verbosity >= 1) {
      result.print_long(debug, symbol_printer);
    }
    if (!result.included) {
      auto synth_start = chrono::steady_clock::now();
      ::synthesis::concurrent_trace trace = ::synthesis::make_trace(ctx, program, result.counter_example);
      ::synthesis::reorderings reorder(ctx, program);
      pair<::synthesis::dnf,::synthesis::dnf> dnf = reorder.process_trace(trace);
      ::synthesis::synchronisation synch(program, trace);
      ::synthesis::cnf cnf = negate_dnf(dnf.first);
      ::synthesis::cnf cnf_weak = negate_dnf(dnf.second);
      list<::synthesis::lock> new_locks;
      list<::synthesis::reordering> reorderings;
      synch.generate_sync(cnf, new_locks, reorderings, false);
      vector<vector<placement::location>> lock_locations = locks_to_locations(new_locks, result.counter_example);
      vector<pair<unsigned,placement::location>> locks, unlocks;
      plocks.find_locks(lock_locations, locks, unlocks);
      string file_name = main_filename;
      file_name.replace(file_name.length()-2,2, "." + to_string(counter) + ".c");
      pprogram.print_with_locks(locks, unlocks, debug_folder + file_name);
      exit(1);
      
    } else {
      verification += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
      cout << "Synthesis was successful." << endl;
      print_summary(program);
      return true;
    }
    ++counter;
    print_time(start);
    langinc += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
    debug << endl;
    string file_name = main_filename;
    file_name.replace(file_name.length()-2,2, "." + to_string(counter) + ".c");
    //print_code(program, debug_folder, file_name);
    // reparse threads
    std::list<const clang::FunctionDecl*> functions;
    for (const cfg::abstract_cfg* t : program.threads()) {
      functions.push_back(t->declaration);
    }

  }
}
