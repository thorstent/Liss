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

#include "reorderings.h"
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
#include "lock.h"

using namespace synthesis;
using namespace std;

reorderings::reorderings(const cfg::program& program) :
program(program), symbol_printer(Limi::printer<abstraction::psymbol>()) {
  
}

struct lock_pair {
  const location* lock;
  const location* unlock;
  lock_pair(const location* lock, const location* unlock) : lock(lock), unlock(unlock) {}
};

void reorderings::prepare_trace(reorderings::seperated_trace& strace)
{
  assert(condyield_is_always_yield); // other semantics are not supported by this class
  
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
  
  // a unique final location
  location final_state(ctx.fresh_constant("final", ctx.int_sort()), "final location");
  strace.trace.push_back(final_state);
  
  vector<const location*> thread_endpoints(program.no_threads(), nullptr);  
  
  int counter = 0;
  for (const auto& th : strace.threaded_trace)
  for (const location* loc : th) {
    thread_id_type thread_id = loc->symbol->thread_id();
    variable_type var = loc->symbol->variable;
    
    string name = loc->name_str;
    
    bool seq_condition = true;
    
    // deal with other operations
    switch(loc->symbol->operation) {
      case abstraction::op_class::lock: {
          seq_condition = false;
          lockings[var].push_back(lock_pair(loc, nullptr));
        }
        break;
      case abstraction::op_class::unlock:
        lockings[var].back().unlock = loc;
        break;
      case abstraction::op_class::wait_reset:
        resets[var].push_back(loc);
        // no break here
      case abstraction::op_class::wait:
      case abstraction::op_class::wait_not: {
          seq_condition = loc->symbol->assume && !assumes_allow_switch;
          waits[var].push_back(loc);
        }
      break;
      case abstraction::op_class::notify:
        notifies[var].push_back(loc);
        break;
      case abstraction::op_class::reset:
        resets[var].push_back(loc);
        break;
      case abstraction::op_class::read:
        strace.reads[var].push_back(loc);
        break;
      case abstraction::op_class::write:
        strace.writes[var].push_back(loc);
        break;
      case abstraction::op_class::epsilon:
        break;
      case abstraction::op_class::yield:
        seq_condition = false;
        break;
    }
    
    if (thread_endpoints[thread_id]) {
      if (seq_condition)
        sequential = sequential && ((z3::expr)*loc) == ((z3::expr)*thread_endpoints[thread_id]) + ctx.int_val(1);
      thread_order = thread_order && *loc > *thread_endpoints[thread_id];
    }
    thread_endpoints[thread_id] = loc;
  }
  
  for (const auto& endp : thread_endpoints) {
    // final must be after all other locations
    thread_order = thread_order && *endp < final_state;
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


dnf_constr reorderings::process_trace(const std::vector< abstraction::psymbol >& trace, const placement::lock_symbols& synthesised_locks)
{
  seperated_trace strace(program.identifiers().no_variables(), program.identifiers().no_conditionals(), program.no_threads(), ctx);
  split_trace(trace, strace);
  prepare_trace(strace);
  synth_locks(strace, synthesised_locks);
  
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
  slv_good.add(strace.synth_locks);
  
  z3::solver slv_good_weak(ctx);
  slv_good_weak.add(strace.thread_order);
  slv_good_weak.add(strace.distinct);
  slv_good_weak.add(strace.locks);
  slv_good_weak.add(strace.sequential);
  slv_good_weak.add(strace.synth_locks);
  
  
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
    z3::model model = slv_good.get_model();
    if (verbosity >=3) {
      debug << "Solver:" << endl << slv_good << endl;
      debug << "Model:" << endl << model << endl;
    }
    conj_constr constraint = find_order(strace, model);
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
  slv_bad.add(strace.synth_locks);
  // gather concurrent traces that are not possible sequentially
  if (verbosity >=1) {
    debug << "Find concurrent traces that are not sequential" << endl;
  }
  vector<conj_constr> bad_traces_int;
  
#ifdef SANITY
  // sanity check: 
  slv_bad.push();
  slv_bad.add(original_trace_gen_expr);
  if (slv_bad.check()!=z3::sat) cerr << endl << endl << "SANITY: Original trace not valid concurrent trace" << endl;
  slv_bad.pop();
#endif
  
  slv_bad.add(!make_constraint(ctx, seq_traces));
  
  slv_bad.push();
  unsigned count = 0;
  while (slv_bad.check() == z3::sat) {
    z3::model model = slv_bad.get_model();
    if (verbosity >=3) {
      debug << "Solver:" << endl << slv_bad << endl;
      debug << "Model:" << endl << model << endl;
    }
    conj_constr constraint = find_order(strace, model);
    conj_constr wnconstraint = wait_notify_order(strace, model);
    constraint.insert(constraint.end(), wnconstraint.begin(), wnconstraint.end());
    min_unsat<constraint_atom>(slv_good_weak, constraint, [](const constraint_atom& ca)->z3::expr{return ca;});
    assert (!constraint.empty());
    slv_bad.add(!make_constraint(ctx, constraint));
    bad_traces_int.push_back(constraint);

    if (verbosity >= 2) {
      debug << count++;
      debug << ": Found constraint: ";
      debug << constraint;
      debug << endl;
      debug << "Trace:" << endl;
      print_trace(strace, model, debug);
    }
  }
  slv_bad.pop();
  
  // filter out duplicates:
  min_unsat<conj_constr>(slv_bad, bad_traces_int, [this](const conj_constr& c)->z3::expr{return !make_constraint(ctx, c);});
  
  dnf_constr bad_traces;
  for (const conj_constr& p : bad_traces_int) {
    bad_traces.push_back(p);
  }
  
  if (verbosity >= 1) {
    debug << "All bad traces: " << endl;
    //print_constraint(bad_traces, symbol_printer, debug);
    for (unsigned i = 0; i < bad_traces.size(); ++i) {
      if (all_of(bad_traces_int[i].begin(), bad_traces_int[i].end(), [](constraint_atom ca) {return ca.before.original_position<ca.after.original_position;}))
        debug << "(*) ";
      debug << bad_traces_int[i];
      if (i < bad_traces.size() - 1) 
        debug << " \\/" << std::endl;
    }
    debug << endl;
  }
  return bad_traces;
}

conj_constr reorderings::find_order(const seperated_trace& strace, const z3::model& model)
{
  conj_constr result;
  // get the assigned numbers:
  
  //cout << model << endl;
  
  for (variable_type i = 0; i < strace.reads.size(); ++i) {
    // reads should not collide with any write, enforce their order
    for (auto it = strace.reads[i].begin(); it != strace.reads[i].end(); ++it) {
      for (auto it2 = strace.writes[i].begin(); it2 != strace.writes[i].end(); ++it2) {
        if ((**it).thread_id() != (**it2).thread_id()) {
          assert (model.eval(**it < **it2).get_bool() != model.eval(**it2 < **it).get_bool());
          if (model.eval(**it < **it2).get_bool()) {
            //cout << (**it < **it2) << model.eval(**it) << " < " << model.eval(**it2) << endl;
            result.push_back(constraint_atom(**it,**it2));
          }
          else {
            //cout << (**it2 < **it) << model.eval(**it2) << " < " << model.eval(**it) << endl;
            result.push_back(constraint_atom(**it2,**it));
          }
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

conj_constr reorderings::wait_notify_order(const reorderings::seperated_trace& strace, const z3::model& model)
{
  typedef pair<const location*, int> loc_pair;
  conj_constr result;

  for (conditional_type i = 0; i < strace.waits.size(); ++i) {
    vector<loc_pair> order;
    // adds waits, notifies and resets to a list
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
      // it moves backwards
      auto it = find_if(order.rbegin(), order.rend(), [wait](loc_pair p){return p.first==wait;});
      assert (it!=order.rend());
      // it2 moves forwards
      auto it2 = find_if(order.begin(), order.end(), [wait](loc_pair p){return p.first==wait;});
      assert (it2!=order.end());
      vector<bool> before(strace.threads); // mark off the threads we already saw
      vector<bool> after(strace.threads);
      abstraction::op_class before_symbol = abstraction::op_class::notify;
      abstraction::op_class before_symbol2 = abstraction::op_class::notify;
      abstraction::op_class after_symbol = abstraction::op_class::reset;
      abstraction::op_class after_symbol2 = abstraction::op_class::wait_reset;
      if (it->first->symbol->operation == abstraction::op_class::wait_not) {
        // for wait_not invert the logic of the symbols
        before_symbol = abstraction::op_class::reset;
        before_symbol2 = abstraction::op_class::wait_reset;
        after_symbol = abstraction::op_class::notify;
        after_symbol2 = abstraction::op_class::notify;
      }
      auto ref_loc = *it->first;
      // look for notifies before and resets after
      for (++it; it != order.rend(); ++it) {
        if (it->first->symbol->operation == before_symbol || it->first->symbol->operation == before_symbol2) {
          if (!before[it->first->symbol->thread_id()] && it->first->symbol->thread_id() != ref_loc.symbol->thread_id()) {
            before[it->first->symbol->thread_id()] = true;
            result.push_back(constraint_atom(*it->first, ref_loc, true));
          }
        }
      }
      for (++it2; it2 != order.end(); ++it2) {
        if (it2->first->symbol->operation == after_symbol || it2->first->symbol->operation == after_symbol2) {
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


void reorderings::print_trace(const reorderings::seperated_trace& strace, const z3::model& model, ostream& out)
{
  typedef pair<location, int> loc_pair;
  vector<loc_pair> order;
  for (const location& l : strace.trace) {
    int number;
    if (model.eval(l).get_int(number))
      order.push_back(make_pair(l, number));
  }
  
  std::sort(order.begin(), order.end(), [](const loc_pair& a, const loc_pair& b) { return a.second < b.second; } );
  
  for (const loc_pair& p : order) {
    //out << p.first.name << " " << model.eval(p.first.name) << " ";
    if (!p.first.description.empty())
      out << p.first.description;
    else {
      out << p.first;
    }
    out << endl;
  }
}

void reorderings::split_trace(const vector< abstraction::psymbol >& trace, reorderings::seperated_trace& strace)
{
  unordered_map<abstraction::psymbol, unsigned> iteration_counter;
  strace.threaded_trace=std::vector<std::vector<const location*>>(program.no_threads());
  unsigned counter = 0;
  vector<unsigned> iter(program.no_threads(),0);
  for (const abstraction::psymbol& symbol : trace) {
    string name = "loc";
    
    if (iteration_counter.find(symbol)==iteration_counter.end())
      iteration_counter[symbol] = 1;
    else
      iteration_counter[symbol]++;
    iter[symbol->thread_id()] = max(iter[symbol->thread_id()],iteration_counter[symbol]);
    
    // create location
    if (verbosity >= 2) {
      stringstream ss;
      ss << symbol;
      if (iter[symbol->thread_id()]>1)
        ss << "(" << iter[symbol->thread_id()] << ")";
      name = ss.str();
    }
    location loc(ctx.fresh_constant(name.c_str(), ctx.int_sort()), name, symbol, counter++);
    loc.iteration = iter[symbol->thread_id()];
    strace.trace.push_back(loc);
    strace.threaded_trace[symbol->thread_id()].push_back(&strace.trace.back());
  }
}

std::vector<std::pair<const location*,const location*>> reorderings::find_lock_locs(reorderings::seperated_trace& strace, const std::vector<abstraction::psymbol>& lock) {
  std::vector<std::pair<const location*,const location*>> result;
  // here we find the subsets of the locations we are looking for
  if (lock.size()>1) {
    const std::vector<const location*>& thread = strace.threaded_trace[lock.front()->thread_id()];
    std::vector<bool> used(thread.size(),false); // we used this location already, it is therefore not relevant to consider
    for (auto itsy = lock.begin(); itsy!=lock.end(); ++itsy) {
      // find this symbol in the thread of the trace
      for (unsigned i = 0; i < thread.size(); ++i) {
        if (equal_to<abstraction::psymbol>()(thread[i]->symbol, *itsy) && !used[i]) {
          // we found a match, see how long it can go on
          const location* start = thread[i];
          const location* end = start;
          used[i] = true;
          auto itsy2 = itsy;
          while (i+1 < thread.size() && itsy2+1 != lock.end() && equal_to<abstraction::psymbol>()(thread[i+1]->symbol, *(itsy2+1))) {
            // the next symbol also matches
            ++i;++itsy2;
            used[i] = true;
            end = thread[i];
          }
          if (start != end) {
            result.push_back(make_pair(start,end));
          }
        }
      }
    }
  }
  return result;
}

void reorderings::synth_locks(reorderings::seperated_trace& strace, const placement::lock_symbols& synthesised_locks)
{
  z3::expr& cnf = strace.synth_locks;
  for (const disj<std::vector<std::vector<abstraction::psymbol>>>& lockd : synthesised_locks) {
    // one of these locks needs to hold
    z3::expr disj = ctx.bool_val(false);

    for (const std::vector<std::vector<abstraction::psymbol>>& lock : lockd) {
      // one single lock
      z3::expr locke = ctx.bool_val(true);
      // define conflicts mutually between locations
      for (auto it = lock.begin(); it!=lock.end(); ++it) {
        // each conflict in this vector is in conflict with all the other vectors
        std::vector<std::pair<const location*,const location*>> locs = find_lock_locs(strace, *it);
        auto it2 = it+1;
        for (; it2!=lock.end(); ++it2) {
          if (it!=it2) {
            std::vector<std::pair<const location*,const location*>> locs2 = find_lock_locs(strace, *it2);
            for (const std::pair<const location*,const location*>& l : locs)
              for (const std::pair<const location*,const location*>& l2 : locs2) {
                locke = locke && (*l.first > *l2.second || *l.second < *l2.first);
              }
            
          }
        }
        
      }
      disj = disj || locke;
    }
    cnf = cnf && disj; 
  }
}
