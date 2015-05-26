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

#include "find_locks.h"
#include <z3++.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <list>
#include <stdexcept>
#include <climits>
#include "location.h"
#include "options.h"
#include "z3_helpers.h"

using namespace synthesis;
using namespace std;

find_locks::find_locks(z3::context& ctx, const cfg::program& program) :
program(program), ctx(ctx), symbol_printer(Limi::printer<abstraction::psymbol>()) {
  
}

struct lock_pair {
  const location* lock;
  const location* unlock;
  lock_pair(const location* lock, const location* unlock) : lock(lock), unlock(unlock) {}
};

void find_locks::prepare_trace(const concurrent_trace& trace, find_locks::seperated_trace& strace)
{
  vector<list<lock_pair>> lockings(program.identifiers().no_locks()); // gathers informations about locks and unlocks
  vector<list<const location*>>& notifies = strace.notifies; // the places where the conditional is notified
  vector<list<const location*>>& resets = strace.resets; // the places where the conditional is reset
  // the places where the conditional is waited for
  // it is a pair, the first element is the pre-wait, the second the actual wait. There can be a context switch between wait
  // and pre-wait if the condition is not fulfilled at the pre-wait
  vector<list<const location*>>& waits = strace.waits;
  
  // the constraints
  z3::expr& sequential = strace.sequential;
  z3::expr& thread_order = strace.thread_order; // inner thread statement order
  z3::expr& locks = strace.locks;
  z3::expr& conditionals = strace.conditionals;
  z3::expr& distinct = strace.distinct;
  z3::expr& atomic_execution = strace.atomic_execution;
  
  // a unique final location
  location final_state(ctx.fresh_constant("final", ctx.int_sort()), "final location");
  strace.trace.push_back(final_state);
  
  vector<location*> thread_endpoints(program.no_threads(), nullptr);  
  
  int counter = 0;
  for (const auto& th : trace.threads)
  for (const location loc : th) {
    thread_id_type thread_id = loc.symbol->thread_id();
    variable_type var = loc.symbol->variable;
    
    string name = loc.name_str;
    strace.trace.push_back(loc);
    location* loc_ptr = &strace.trace.back();
    
    bool seq_condition = true; // seq has to hold
        
    // deal with other operations
    switch(loc.symbol->operation) {
      case abstraction::op_class::lock: {
          lockings[var].push_back(lock_pair(loc_ptr, nullptr));
        }
        break;
      case abstraction::op_class::unlock:
        lockings[var].back().unlock = loc_ptr;
        break;
      case abstraction::op_class::wait_reset:
        resets[var].push_back(loc_ptr);
      case abstraction::op_class::wait:
      case abstraction::op_class::wait_not: {
          waits[var].push_back(loc_ptr);
        }
      break;
      case abstraction::op_class::notify:
        notifies[var].push_back(loc_ptr);
        break;
      case abstraction::op_class::reset:
        resets[var].push_back(loc_ptr);
        break;
      case abstraction::op_class::read:
        strace.reads[var].push_back(loc_ptr);
        break;
      case abstraction::op_class::write:
        strace.writes[var].push_back(loc_ptr);
        break;
      case abstraction::op_class::epsilon:
        break;
      case abstraction::op_class::yield:
        seq_condition = false;
        break;
    }
    
    if (thread_endpoints[thread_id]) {
      if (seq_condition) {
        sequential = sequential && ((z3::expr)loc) == ((z3::expr)*thread_endpoints[thread_id]) + ctx.int_val(1);
        strace.atomics.emplace_back(ctx, *thread_endpoints[thread_id], loc);
        atomic_execution = atomic_execution && implies(strace.atomics.back().atomic, ((z3::expr)loc) == ((z3::expr)*thread_endpoints[thread_id]) + ctx.int_val(1));
      }
      thread_order = thread_order && loc > *thread_endpoints[thread_id];
    }
    thread_endpoints[thread_id] = loc_ptr;

  }
  
  // generate the distinct constraint
  vector<Z3_ast> expressions;
  for (const location& l : strace.trace)
    expressions.push_back(l.name);
  Z3_ast distinct_ = Z3_mk_distinct(ctx, expressions.size(), &expressions[0]);
  distinct = z3::expr(ctx, distinct_);
  
  
  // conditions for locks (for each pair, either one before the other or the other wait round)
  for (unsigned i = 0; i < lockings.size(); ++i) {
    for (auto it = lockings[i].begin(); it != lockings[i].end(); ++it) {
      for (auto it2 = lockings[i].begin(); it2 != lockings[i].end(); ++it2) {
        if (it != it2) {
          if (it->unlock == nullptr)
            it->unlock = &final_state;
          if (it2->unlock == nullptr)
            it2->unlock = &final_state;
          // make the lock sections exclusive to each other
          locks = locks && ((*(it->lock) > *(it2->unlock)) || (*(it->lock) < *(it2->lock)));
        }
      }
    }
  }
  
  // conditions for conditionals
  for (unsigned i = 0; i < waits.size(); ++i) {
    for (auto it = waits[i].begin(); it != waits[i].end(); ++it) {
      bool is_wait = (*it)->symbol->operation!=abstraction::op_class::wait_not;
      if (is_wait) {
        z3::expr conditional_notify = ctx.bool_val(false); // it is not notified to begin with
        for (auto itn = notifies[i].begin(); itn != notifies[i].end(); ++itn) {
          // if this is a wait, then ensure that it is after a notify and no reset is in between
          z3::expr conditional_one_notify = (**itn < **it);
          for (auto itr = resets[i].begin(); itr != resets[i].end(); ++itr) {
            if (*it != *itr) {
              conditional_one_notify = conditional_one_notify && (**itr < **itn || **itr > **(it));
            }
          }
          conditional_notify = conditional_notify || conditional_one_notify;
        }
        conditionals = conditionals && conditional_notify;
      } else {
        // we have a wait_not
        z3::expr conditional_notify = ctx.bool_val(false);
        // firstly we can be before all the notifies, that is fine
        {
          z3::expr conditional_one_notify = ctx.bool_val(true);
          for (auto itn = notifies[i].begin(); itn != notifies[i].end(); ++itn) {
            conditional_one_notify = conditional_one_notify && (**it < **itn);
          }
          conditional_notify = conditional_notify || conditional_one_notify;
        }
        // or a reset needs to be before a notify
        for (auto itr = resets[i].begin(); itr != resets[i].end(); ++itr) {
          // if this is a wait, then ensure that it is after a notify and no reset is in between
          z3::expr conditional_one_notify = (**itr < **it);
          for (auto itn = notifies[i].begin(); itn != notifies[i].end(); ++itn) {
            conditional_one_notify = conditional_one_notify && (**itn < **itr || **itn > **it);
          }
          conditional_notify = conditional_notify || conditional_one_notify;
        }
        conditionals = conditionals && conditional_notify;
      }
    }
  }
}


void find_locks::process_trace(const concurrent_trace& trace)
{
  seperated_trace strace(program.identifiers().no_variables(), program.identifiers().no_conditionals(), program.no_threads(), ctx);
  prepare_trace(trace, strace);
  
#ifdef SANITY
  z3::expr original_trace = ctx.bool_val(true);
  for (const location& l : strace.trace) {
    if (l.original_position>=0) {
      original_trace = original_trace && (l == ctx.int_val(l.original_position));
    }
  }
#endif

  
  // build solver for good traces
  
  
#ifdef SANITY
  // sanity check: build original counter-example trace
  z3::solver slv(ctx);
  slv.add(original_trace);
  auto r = slv.check();
  assert (r==z3::sat);
  conj_constr original_trace_gen = find_order(strace, slv.get_model());
  z3::expr original_trace_gen_expr = make_constraint(ctx, original_trace_gen);
#endif
  
  z3::solver slv_good(ctx);
  slv_good.add(strace.thread_order);
  slv_good.add(strace.distinct);
  slv_good.add(strace.locks);
  slv_good.add(strace.conditionals);
  slv_good.add(strace.sequential);
  
  
#ifdef SANITY
  // sanity check: counter example is not sequential
  slv_good.push();
  if (slv_good.check()!=z3::sat) cerr << endl << endl << "SANITY: No valid sequential interleaving exists for this counter-example" << endl;
  slv_good.add(original_trace_gen_expr);
  if (slv_good.check()!=z3::unsat) {
    cerr << endl << endl << "SANITY: Original trace is sequential; Here is the trace" << endl;
    print_trace(strace, slv_good.get_model(), cerr);
    cerr << "And here is the sequential constraint" << endl;
    cerr << original_trace_gen;
    cerr << endl;
  }
  slv_good.pop();
#endif
  
  slv_good.push();
  
  dnf_constr seq_traces;
  
  if (verbosity >=1) {
    debug << "Find all sequential traces" << endl;
  }  
  // accumulate sequential traces
  while (slv_good.check() == z3::sat) {
    if (verbosity >=3) {
      debug << "Solver:" << endl << slv_good << endl;
      debug << "Model:" << endl << slv_good.get_model() << endl;
    }
    conj_constr constraint = find_order(strace, slv_good.get_model());
    slv_good.add(!make_constraint(ctx, constraint));
    if (verbosity >= 2) {
      debug << "Found constraint: ";
      debug << constraint;
      debug << endl;
    }
    seq_traces.push_back(constraint);
  }
  slv_good.pop();
  
  // build solver for bad traces
  z3::solver slv_bad(ctx);
  slv_bad.add(strace.thread_order);
  slv_bad.add(strace.distinct);
  slv_bad.add(strace.locks);
  slv_bad.add(strace.conditionals);
  slv_bad.add(strace.atomic_execution);
  // gather concurrent traces that are not possible sequentially
  if (verbosity >=1) {
    debug << "Find atomic sections" << endl;
  }
  
#ifdef SANITY
  // sanity check: 
  slv_bad.push();
  slv_bad.add(original_trace_gen_expr);
  if (slv_bad.check()!=z3::sat) cerr << endl << endl << "SANITY: Original trace not valid concurrent trace" << endl;
  slv_bad.pop();
#endif
  
  // only interested in bad traces
  slv_bad.add(!make_constraint(ctx, seq_traces));
  
  // trace copy
  std::vector<atomic_instr> trace_copy;
  for (const auto& i : strace.atomics) {
    trace_copy.push_back(i);
  }
  // minimal set of atomic constraints needed to eliminate all bad traces
  min_unsat<atomic_instr>(slv_bad, trace_copy, [](const atomic_instr& loc)->z3::expr{return loc.atomic;});
  
  for (const atomic_instr& l : trace_copy) {
    cout << l.before << "--" << l.after << endl;
  }
  
  slv_bad.push();
  for (unsigned i = 1; i < trace_copy.size(); ++i) {
    slv_bad.add(trace_copy[i].atomic);
  }
  while (slv_bad.check() == z3::sat) {
    if (verbosity >=3) {
      debug << "Solver:" << endl << slv_bad << endl;
      debug << "Model:" << endl << slv_bad.get_model() << endl;
    }
    conj_constr constraint = find_order(strace, slv_bad.get_model());
    min_unsat<constraint_atom>(slv_good, constraint, [](const constraint_atom& ca)->z3::expr{return ca;});

    if (verbosity >= 2) {
      debug << "Found constraint: ";
      debug << constraint;
      debug << endl;
    }
    
    // find interrupting instructions
    print_trace_bound(strace, slv_bad.get_model(), cout, trace_copy[0].before.name, trace_copy[0].after.name);
    
    // find interrupters
    z3::expr bef = trace_copy[0].before.name;
    z3::expr af = trace_copy[0].after.name;
    std::vector<location> trace_copy;
    for (const auto& i : strace.trace) {
      if (i.symbol_valid) trace_copy.push_back(i);
    }
    cout << "Min inside" << endl;
    min_unsat<location>(slv_bad, trace_copy, [bef,af](const location& l)->z3::expr{return l <= bef || l >= af;});
    for (const location& l : trace_copy) {
      cout << l << endl;
    }
    slv_bad.add(!make_constraint(ctx, constraint));
  }
  slv_bad.pop();
  
  /*// filter out duplicates:
  min_unsat<pair<conj_constr,conj_constr>>(slv_bad, bad_traces_int, [this](const pair<conj_constr,conj_constr>& c)->z3::expr{return !make_constraint(ctx, c.first);});
  min_unsat<conj_constr>(slv_bad, bad_traces_weak, [this](const conj_constr& c)->z3::expr{return !make_constraint(ctx, c);});
  
  dnf_constr bad_traces;
  for (const pair<conj_constr,conj_constr>& p : bad_traces_int) {
    conj_constr c1(p.first);
    //c1.insert(c1.end(), p.second.begin(), p.second.end());
    bad_traces.push_back(c1);
  }
  
  if (verbosity >= 1) {
    debug << "All bad traces: " << endl;
    //print_constraint(bad_traces, symbol_printer, debug);
    for (unsigned i = 0; i < bad_traces.size(); ++i) {
      if (all_of(bad_traces_int[i].first.begin(), bad_traces_int[i].first.end(), [](constraint_atom ca) {return ca.before.original_position<ca.after.original_position;}))
        debug << "(*) ";
      debug << bad_traces[i];
      if (i < bad_traces.size() - 1) 
        debug << " \\/" << std::endl;
    }
    debug << endl;
    debug << "All weak constraints: " << endl;
    debug << bad_traces_weak;
  }
  return make_pair(bad_traces, bad_traces_weak);*/
}

conj_constr find_locks::find_order(const find_locks::seperated_trace& strace, const z3::model& model)
{
  conj_constr result;
  // get the assigned numbers:
  
  for (variable_type i = 0; i < strace.reads.size(); ++i) {
    // reads should not collide with any write, enforce their order
    for (auto it = strace.reads[i].begin(); it != strace.reads[i].end(); ++it) {
      for (auto it2 = strace.writes[i].begin(); it2 != strace.writes[i].end(); ++it2) {
        if ((**it).thread_id() != (**it2).thread_id()) {
          if (model.eval(**it < **it2).get_bool())
            result.push_back(constraint_atom(**it,**it2));
          else 
            result.push_back(constraint_atom(**it2,**it));
        }
      }
    }
    // writes should also not collide with each other
    for (auto it = strace.writes[i].begin(); it != strace.writes[i].end(); ++it) {
      for (auto it2 = strace.writes[i].begin(); it2 != strace.writes[i].end(); ++it2) {
        if ((**it).thread_id() != (**it2).thread_id()) {
          if (model.eval(**it < **it2).get_bool())
            result.push_back(constraint_atom(**it,**it2));
          else 
            result.push_back(constraint_atom(**it2,**it));
        }
      }
    }
  }
  
  return result;
}

conj_constr find_locks::wait_notify_order(const find_locks::seperated_trace& strace, const z3::model& model)
{
  typedef pair<const location*, int> loc_pair;
  conj_constr result;

  for (conditional_type i = 0; i < strace.waits.size(); ++i) {
    vector<loc_pair> order;
    for (const location* wait : strace.waits[i]) {
      z3::expr number_expr = model.eval(*wait);
      int number;
      Z3_get_numeral_int(ctx, number_expr, &number);
      order.push_back(make_pair(wait, number));  
    }
    for (const location* notify : strace.notifies[i]) {
      z3::expr number_expr = model.eval(*notify);
      int number;
      Z3_get_numeral_int(ctx, number_expr, &number);
      order.push_back(make_pair(notify, number));  
    }
    for (const location* reset : strace.resets[i]) {
      z3::expr number_expr = model.eval(*reset);
      int number;
      Z3_get_numeral_int(ctx, number_expr, &number);
      order.push_back(make_pair(reset, number));  
    }
    std::sort(order.begin(), order.end(), [](const loc_pair& a, const loc_pair& b) { return a.second < b.second; } );
    for (const location* wait : strace.waits[i]) {
      auto it = find_if(order.rbegin(), order.rend(), [wait](loc_pair p){return p.first==wait;});
      assert (it!=order.rend());
      auto it2 = find_if(order.begin(), order.end(), [wait](loc_pair p){return p.first==wait;});
      assert (it2!=order.end());
      vector<bool> before(strace.threads); // mark off the threads we already saw
      vector<bool> after(strace.threads);
      abstraction::op_class before_symbol = abstraction::op_class::notify;
      abstraction::op_class after_symbol = abstraction::op_class::reset;
      if (it->first->symbol->operation != abstraction::op_class::wait_not) {
        before_symbol = abstraction::op_class::reset;
        after_symbol = abstraction::op_class::notify;
      }
      auto ref_loc = *it->first;
      // look for notifies before and resets after
      for (++it; it != order.rend(); ++it) {
        if (it->first->symbol->operation != before_symbol) {
          if (!before[it->first->symbol->thread_id()] && it->first->symbol->thread_id() != ref_loc.symbol->thread_id()) {
            before[it->first->symbol->thread_id()] = true;
            result.push_back(constraint_atom(*it->first, ref_loc, true));
          }
        }
      }
      for (++it2; it2 != order.end(); ++it2) {
        if (it2->first->symbol->operation != after_symbol) {
          if (!after[it2->first->symbol->thread_id()] && it2->first->symbol->thread_id() != ref_loc.symbol->thread_id()) {
            after[it2->first->symbol->thread_id()] = true;
            result.push_back(constraint_atom(ref_loc, *it2->first, true));
          }
        }
      }
      
    }
  }
  return result;
}


void find_locks::print_trace(const find_locks::seperated_trace& strace, const z3::model& model, ostream& out)
{
  typedef pair<location, int> loc_pair;
  vector<loc_pair> order;
  for (const location& l : strace.trace) {
    z3::expr number_expr = model.eval(l);
    int number;
    Z3_get_numeral_int(ctx, number_expr, &number);
    order.push_back(make_pair(l, number));
  }
  
  std::sort(order.begin(), order.end(), [](const loc_pair& a, const loc_pair& b) { return a.second < b.second; } );
  
  for (const loc_pair& p : order) {
    if (!p.first.description.empty())
      out << p.first.description;
    else {
      out << p.first;
    }
    out << endl;
  }
}

void find_locks::print_trace_bound(const find_locks::seperated_trace& strace, const z3::model& model, ostream& out, z3::expr start, z3::expr end)
{
  typedef pair<location, int> loc_pair;
  vector<loc_pair> order;
  for (const location& l : strace.trace) {
    z3::expr number_expr = model.eval(l);
    int number;
    Z3_get_numeral_int(ctx, number_expr, &number);
    order.push_back(make_pair(l, number));
  }
  
  int start_num, end_num;
  z3::expr number_start = model.eval(start);
  Z3_get_numeral_int(ctx, number_start, &start_num);
  z3::expr number_end = model.eval(end);
  Z3_get_numeral_int(ctx, number_end, &end_num);
  
  std::sort(order.begin(), order.end(), [](const loc_pair& a, const loc_pair& b) { return a.second < b.second; } );
  order.erase(std::remove_if(order.begin(), order.end(), [start_num,end_num](const loc_pair& a) { return a.second <= start_num || a.second >= end_num; } ), order.end());
  
  for (const loc_pair& p : order) {
    if (!p.first.description.empty())
      out << p.first.description;
    else {
      out << p.first;
    }
    out << endl;
  }
}