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

#include "synthesis.h"
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

using namespace clang;
using namespace std;

void actions::synthesis::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  cfg::program new_program(program);
  Rewriter rewriter;
  
  string file_name = main_filename;
  file_name.replace(file_name.length()-2,2, ".start.c");
  print_code(program, debug_folder, file_name);
  print_code(program, start_file_code);
  
  bool success = synth_loop(program, new_program);
  
  file_name = main_filename;
  file_name.replace(file_name.length()-2,2, ".end.c");
  print_code(program, debug_folder, file_name);
  
  if (success) {
    print_code(program, output_file_code);
  } else {
    Limi::printer<abstraction::psymbol> symbol_printer;
    ofstream file_out(output_file_log);
    result.print_long(file_out, symbol_printer);
    file_out.close();
  }
}

void actions::synthesis::print_code(const cfg::program& program, string directory, string filename)
{ 
  print_code(program, directory + filename);
}

void actions::synthesis::print_code(const cfg::program& program, string filename)
{  
  std::error_code ec;
  llvm::raw_fd_ostream file_out(filename, ec, llvm::sys::fs::F_Text | llvm::sys::fs::F_RW);
  file_out << "#include \"langinc.h\"\n\n";
  for (Decl* d : program.translation_unit->decls()) {
    
    if (is_local(d, program.ast_context)) {
      if (isa<FunctionDecl>(d) && cast<FunctionDecl>(d)->hasBody())
        continue;
      d->print(file_out);
      file_out << ";\n";
    }
  }
  file_out << "\n";
  
  for (Decl* d : program.translation_unit->decls()) {
    if (is_local(d, program.ast_context) && isa<FunctionDecl>(d) && cast<FunctionDecl>(d)->hasBody()) {
      d->print(file_out);
      file_out << "\n";
    }
  }
  
  file_out.close();
}

bool actions::synthesis::is_local(Decl* d, ASTContext& ast_context)
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

void actions::synthesis::print_summary(const cfg::program& original_program) {
  debug << "Threads: " << original_program.no_threads() << endl;
  debug << "Iterations: " << iteration << endl;
  debug << "Liss: " << (double)langinc.count()/1000 << "s" << endl;
  debug << "Verification: " << (double)verification.count()/1000 << "s" << endl;
  debug << "Synthesis: " << (double)synthesis_time.count()/1000 << "s" << endl;
  //cout.precision(1);
  debug << "| " << original_program.no_threads() << " | " << iteration << " | " << this->max_bound <<  " | " << (double)langinc.count()/1000 << "s | "  << (double)synthesis_time.count()/1000 << "s | " << (double)verification.count()/1000 << "s |";
}


bool actions::synthesis::synth_loop(const cfg::program& original_program, cfg::program& program)
{
  z3::context ctx;

  Limi::printer<abstraction::psymbol> symbol_printer;
  abstraction::concurrent_automaton sequential(original_program, false, true);
  
  unsigned counter = 0;
  list<::synthesis::lock> locks;
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
      synch.generate_sync(cnf, new_locks, reorderings, true);
      if (new_locks.empty() && reorderings.empty()) {
        synch.generate_sync(cnf_weak, new_locks, reorderings, false);
        if (new_locks.empty() && reorderings.empty()) {
          langinc += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
          cerr << "Inference failed, program incomplete." << endl;
          print_summary(original_program);
          return false;
        }
      }
      ::synthesis::insertion insert(program);
      insert.remove_locks(locks);
      locks.insert(locks.end(), new_locks.begin(), new_locks.end());
      //locks = new_locks;
      insert.add_locks(locks);
      insert.apply_reorderings(reorderings);
      auto synth_end = chrono::steady_clock::now();
      synthesis_time += std::chrono::duration_cast<chrono::milliseconds>(synth_end - synth_start);
      if (verbosity>=1)
        debug << "=== Added synchronisation, testing for more errors ===" << endl;
    } else {
      verification += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
      cout << "Synthesis was successful." << endl;
      print_summary(original_program);
      return true;
    }
    ++counter;
    print_time(start);
    langinc += std::chrono::duration_cast<chrono::milliseconds>(langinc_end - langinc_start);
    debug << endl;
    string file_name = main_filename;
    file_name.replace(file_name.length()-2,2, "." + to_string(counter) + ".c");
    print_code(program, debug_folder, file_name);
    // reparse threads
    std::list<const clang::FunctionDecl*> functions;
    for (const cfg::abstract_cfg* t : program.threads()) {
      functions.push_back(t->declaration);
    }
    program.clear_threads();
    for (const clang::FunctionDecl* f : functions) {
      cfg::abstract_cfg* cfg = new cfg::abstract_cfg(f,0);
      clang_interf::parse_thread(*cfg, program.identifiers(), program.ast_context);
      program.add_thread(cfg);
    }
    
    /*abstraction::concurrent_automaton sequential_new(program, false, true);
    Limi::antichain_algo<abstraction::pcstate,abstraction::pcstate,abstraction::psymbol,abstraction::concurrent_automaton,abstraction::concurrent_automaton> algo2(sequential_new, sequential);
    Limi::inclusion_result<abstraction::psymbol> result2 = algo2.run();
    result2.print_long(debug, sequential_new.symbol_printer());
    
    Limi::antichain_algo<abstraction::pcstate,abstraction::pcstate,abstraction::psymbol,abstraction::concurrent_automaton,abstraction::concurrent_automaton> algo3(sequential, sequential_new);
    Limi::inclusion_result<abstraction::psymbol> result3 = algo3.run();
    result3.print_long(debug, sequential.symbol_printer());*/
  }
}
