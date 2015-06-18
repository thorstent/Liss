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

#include "print.h"

#include <Limi/automaton.h>
#include <Limi/dot_printer.h>
#include <Limi/timbuk_printer.h>
#include "abstraction/concurrent_automaton.h"
#include "cfg/automaton.h"
#include "options.h"

using namespace actions;
using namespace std;

struct short_symbol_printer : public Limi::printer_base<abstraction::psymbol> {
  virtual void print(const abstraction::psymbol& symbol, std::ostream& out) const {
    out << std::to_string(symbol->thread_id());
    out << symbol->operation << symbol->variable << symbol->loc;
  }
};



void actions::print_program(const cfg::program& program, bool timbuk, bool only_threads, std::string prefix)
{
  create_debug_folder();
  short_symbol_printer symbol_printer;
  if (!timbuk) {
    Limi::dot_printer<state_id_type, abstraction::psymbol, cfg::automaton> pr;
    unsigned t = 0;
    for (const cfg::abstract_cfg* thread : program.minimised_threads()) {
      cfg::automaton automaton(*thread);
      pr.print_dot(automaton, debug_folder + prefix + std::to_string(t) + "-" + thread->name + ".dot");
      cfg::automaton automaton_ef(*thread, true);
      pr.print_dot(automaton_ef, debug_folder + prefix + std::to_string(t) + "-" + thread->name + "_ef.dot");
      ++t;
    }
    
    if (!only_threads) {
      abstraction::concurrent_automaton automaton(program, true, false);
      Limi::dot_printer<abstraction::pcstate, abstraction::psymbol, abstraction::concurrent_automaton> pr;
      pr.print_dot(automaton, debug_folder + prefix + "program_con.dot");
      abstraction::concurrent_automaton automaton2(program, false);
      pr.print_dot(automaton2, debug_folder + prefix + "program_seq.dot");
      abstraction::concurrent_automaton automaton_ef(program, true, true);
      pr.print_dot(automaton_ef, debug_folder + prefix + "program_con_ef.dot");
      abstraction::concurrent_automaton automaton2_ef(program, false, true);
      pr.print_dot(automaton2_ef, debug_folder + prefix + "program_seq_ef.dot");
    }
  } else {
    Limi::timbuk_printer<abstraction::pcstate, abstraction::psymbol, abstraction::concurrent_automaton, Limi::no_independence<abstraction::psymbol>> pr_noind;
    Limi::timbuk_printer<abstraction::pcstate, abstraction::psymbol, abstraction::concurrent_automaton> pr;
    abstraction::concurrent_automaton concurrent(program, true, true);
    pr_noind.print_timbuk(concurrent, debug_folder + prefix + "program_con.timbuk", symbol_printer, "C");
    abstraction::concurrent_automaton sequential(program, false, true);
    pr.print_timbuk(sequential, debug_folder + prefix + "program_seq.timbuk", symbol_printer, "S");
  }
}


void print::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  print_program(program, print_timbuk, only_threads);
  if (verbosity>=1)
    debug << "Printing completed" << std::endl;
}

print::print()
{
  
}


