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

#ifndef SYNTHESIS_REORDERINGS_H
#define SYNTHESIS_REORDERINGS_H

#include <list>
#include "abstraction/symbol.h"
#include <z3++.h>
#include <vector>
#include "location.h"
#include "constraint.h"
#include "cfg/program.h"
#include "abstraction/concurrent_state.h" // for the printer
#include <Limi/results.h>
#include "synthesis/trace_helpers.h"

namespace synthesis {

class reorderings
{
public:
  reorderings(const cfg::program& program);
  /**
   * @brief Discovers a summary of bad traces
   * 
   * @param trace The trace to analyse
   * @param synthesised_locks The locks we already found in prior rounds
   * @return 
   */
  dnf_constr process_trace(const std::vector< abstraction::psymbol >& trace, const synthesis::lock_symbols& synthesised_locks);
private:
  const cfg::program& program;
  
  struct seperated_trace {
    std::list<location> trace; // a list of all locations in the program
    std::vector<std::vector<const location*>> threaded_trace;
    std::vector<std::list<const location*>> reads;  // the vector represents the variables, the list the location where the variable is read
    std::vector<std::list<const location*>> writes; // the vector represents the variables
    std::vector<std::list<const location*>> waits; // the vector represents the variables
    std::vector<std::list<const location*>> resets; // the vector represents the variables
    std::vector<std::list<const location*>> notifies; // the vector represents the variables
    z3::expr sequential;
    z3::expr thread_order; // inner thread statement order
    z3::expr locks;
    z3::expr conditionals;
    z3::expr distinct;
    z3::expr synth_locks;
    unsigned threads;
    seperated_trace(unsigned variables, unsigned conditionals, unsigned threads, z3::context& ctx) :
    threaded_trace(threads),
    reads(variables),
    writes(variables),
    waits(conditionals),
    resets(conditionals),
    notifies(conditionals),
    sequential(ctx.bool_val(true)),
    thread_order(ctx.bool_val(true)),
    locks(ctx.bool_val(true)),
    conditionals(ctx.bool_val(true)),
    distinct(ctx.bool_val(true)),
    synth_locks(ctx.bool_val(true)),
    threads(threads) {}
  };
  
  void split_trace(const std::vector< abstraction::psymbol >& trace, synthesis::reorderings::seperated_trace& strace);
  conj_constr find_order(const seperated_trace& strace, const z3::model& model);
  void print_trace(const seperated_trace& strace, const z3::model& model, std::ostream& out);
  z3::context ctx;
  const Limi::printer<abstraction::psymbol> symbol_printer;
  void prepare_trace(synthesis::reorderings::seperated_trace& strace);
  std::vector<std::pair<const location*,const location*>> find_lock_locs(reorderings::seperated_trace& strace, const lock_list& locks);
  void synth_locks(synthesis::reorderings::seperated_trace& strace, const synthesis::lock_symbols& synthesised_locks);
  conj_constr wait_notify_order(const seperated_trace& strace, const z3::model& model);
};
}

#endif // SYNTHESIS_REORDERINGS_H
