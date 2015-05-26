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

#ifndef SYNTHESIS_FIND_LOCKS_H
#define SYNTHESIS_FIND_LOCKS_H

#include <list>
#include "abstraction/symbol.h"
#include <z3++.h>
#include <vector>
#include "location.h"
#include "constraint.h"
#include "cfg/program.h"
#include "abstraction/concurrent_state.h" // for the printer
#include <Limi/results.h>
#include "trace.h"

namespace synthesis {

class find_locks
{
public:
  find_locks(z3::context& ctx, const cfg::program& program);
  /**
   * @brief Discovers a summary of bad traces
   * 
   * @param trace The trace to analyse
   * @return std::pair< synthesis::dnf, synthesis::dnf > The first element is traditional bad trace analysis. The second element is a pair where less happens-before relations are removed, namely 
   * all those that are automatically true due to wait-notifies are not removed
   */
  void process_trace(const synthesis::concurrent_trace& trace);
private:
  const cfg::program& program;
  struct atomic_instr {
    z3::expr atomic;
    location before;
    location after;
    atomic_instr(z3::context& ctx, const location& before, const location& after) : atomic(ctx.fresh_constant("loc_atomic", ctx.bool_sort())), after(after), before(before) {};
  };
  // TODO: do not have pointers into a list
  struct seperated_trace {
    std::list<location> trace; // a list of all locations in the program (list because pointers are used)
    std::list<atomic_instr> atomics;
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
    z3::expr atomic_execution;
    unsigned threads;
    seperated_trace(unsigned variables, unsigned conditionals, unsigned threads, z3::context& ctx) :
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
      atomic_execution(ctx.bool_val(true)),
      threads(threads) {}
  };
  conj_constr find_order(const seperated_trace& strace, const z3::model& model);
  void print_trace(const seperated_trace& strace, const z3::model& model, std::ostream& out);
  void print_trace_bound(const find_locks::seperated_trace& strace, const z3::model& model, std::ostream& out, z3::expr start, z3::expr end);
  z3::context& ctx;
  const Limi::printer<abstraction::psymbol> symbol_printer;
  std::bitset<max_locks> get_lockset(const abstraction::pcstate& state);
  void prepare_trace(const synthesis::concurrent_trace& trace, synthesis::find_locks::seperated_trace& strace);
  conj_constr wait_notify_order(const seperated_trace& strace, const z3::model& model);
};
}

#endif // SYNTHESIS_FIND_LOCKS_H
