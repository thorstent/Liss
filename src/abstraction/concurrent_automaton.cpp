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

#include "concurrent_automaton.h"

#include <cstring>
#include <algorithm>
#include <iostream>
#include "options.h"

using namespace std;

using namespace abstraction;

concurrent_automaton::concurrent_automaton(const cfg::program& program, bool concurrent, bool collapse_epsilon) : 
Limi::automaton<pcstate,psymbol,concurrent_automaton>(collapse_epsilon),
identifier_store_(program.identifiers()),
concurrent_(concurrent)
{
  for (const cfg::abstract_cfg* thread : program.minimised_threads()) {
    cfgs.push_back(thread);
    threads.push_back(cfg::automaton(*thread));
  }
}


bool concurrent_automaton::int_is_final_state(const pcstate& state) const
{
  for (thread_id_type i = 0; i<state->length; ++i) {
    if (!threads[i].is_final_state((*state)[i]))
      return false;
  }
  return true;
}

void concurrent_automaton::int_initial_states(concurrent_automaton::State_set& states) const
{
  states.insert(make_shared<concurrent_state>(threads.size(), bad_traces_size));
  thread_id_type length = threads.size();
  for (thread_id_type i = 0; i<length; ++i) {
    State_set duplicates = states;
    states.clear();
    bool first = true;
    for (const state_id_type& init : threads[i].initial_states()) {

      if (first) {
        for (const pcstate& s : duplicates) {
          shared_ptr<concurrent_state> dup = make_shared<concurrent_state>(*s);
          dup->threads[i] = init;
          states.insert(dup);
        }
      } else {
        for (const pcstate& s : duplicates) {
          shared_ptr<concurrent_state> dup = make_shared<concurrent_state>(*s);
          dup->threads[i] = init;
          states.insert(dup);
        }
      }
      first = false;
    }
  }
  State_set duplicates = states;
  states.clear();
  for (pcstate s : duplicates) {
    states.insert(s);
  }
}

void concurrent_automaton::int_next_symbols(const pcstate& state, concurrent_automaton::Symbol_set& symbols) const
{
  if (state->current == -1) {
    for (thread_id_type i = 0; i<state->length; ++i) {
      next_single(state, symbols, i);
    }
  } else {
    next_single(state, symbols, state->current);
  }
}

void concurrent_automaton::int_successors(const pcstate& state, const psymbol& sigma, concurrent_automaton::State_set& successors) const
{
  thread_id_type thread = sigma->thread_id();
  if (state->current != no_thread && state->current != static_cast<int>(thread)) return;
  cfg::reward_symbol rs(0,sigma);
  const cfg::automaton::State_set succs = threads[thread].successors(state->threads[thread], rs); // cost is not relevant here
  assert(!succs.empty());
  bool progress;
  pcstate next = apply_symbol(state, sigma, progress);
  
  if (next) {
    if (!progress) {
      successors.insert(next);
      return;
    }
    //cout << threads[thread]->name(next->threads[thread]) << " -> ";
    bool first = true;
    for (const state_id_type& p : succs) {
      pcstate copy = first ? next : make_shared<concurrent_state>(*next); // copy needed if not first element
      next->reward += rs.reward;
      copy->threads[thread] = p;
      //cout << threads[thread]->name(p) << " ";
      if (concurrent_ || threads[thread].is_final_state(p)) copy->current = no_thread;
      successors.insert(copy);
      first = false;
    }
    //cout << successors.size() << endl;
  }
}

pcstate concurrent_automaton::apply_symbol(const pcstate& original_state, const psymbol& sigma, bool& progress) const
{
  progress = true;
  
  if (original_state->current == no_thread || (sigma->assume&&!assumes_allow_switch)) {
    // test if locking is ok
    switch (sigma->operation) {
      case abstraction::op_class::read:
      case abstraction::op_class::write:
      case abstraction::op_class::epsilon:
      case abstraction::op_class::unlock:
      case abstraction::op_class::notify:
      case abstraction::op_class::reset:
      case abstraction::op_class::yield:
        break;
      case abstraction::op_class::wait_reset:
      case abstraction::op_class::wait:
        if (!original_state->conditionals.test(sigma->variable)) {
          // no context switching on synthesised symbols
          return nullptr;
        }
        break;
      case abstraction::op_class::wait_not:
        if (original_state->conditionals.test(sigma->variable)) {
          return nullptr;
        }
        break;
      case abstraction::op_class::lock:
        if (original_state->locks.test(sigma->variable)) {
          return nullptr;
        }
        break;
    }
  }
  
  pcstate cloned_state = make_shared<concurrent_state>(*original_state);
  
  if (condyield_is_always_yield) {
    switch (sigma->operation) {
      case abstraction::op_class::read:
      case abstraction::op_class::write:
      case abstraction::op_class::epsilon:
      case abstraction::op_class::unlock:
      case abstraction::op_class::notify:
      case abstraction::op_class::reset:
      case abstraction::op_class::yield:
        break;
      case abstraction::op_class::wait_reset:
      case abstraction::op_class::wait:
      case abstraction::op_class::wait_not:
      case abstraction::op_class::lock:
        if (original_state->current != no_thread) {
          cloned_state->current = no_thread;
          progress = false;
          return cloned_state;
        }
        break;
    }
  }
  
  cloned_state->current = sigma->thread_id();
  // apply changes
  switch (sigma->operation) {
    case abstraction::op_class::read:
    case abstraction::op_class::write:
    case abstraction::op_class::epsilon:
      break;
    case abstraction::op_class::lock:
      if (original_state->locks.test(sigma->variable)) {
        cloned_state->current = no_thread;
        progress = false;
      } else {
        cloned_state->locks.set(sigma->variable);
      }
      break;
    case abstraction::op_class::unlock:
      cloned_state->locks.reset(sigma->variable);
      break;
    case abstraction::op_class::notify:
      cloned_state->conditionals.set(sigma->variable);
      break;
    case abstraction::op_class::reset:
      cloned_state->conditionals.reset(sigma->variable);
      break;
    case abstraction::op_class::wait_reset:
      if (!cloned_state->conditionals.test(sigma->variable)) {
        cloned_state->current = no_thread;
        progress = false;
      } else {
        cloned_state->conditionals.reset(sigma->variable);
      }
      break;
    case abstraction::op_class::wait:
      if (!cloned_state->conditionals.test(sigma->variable)) {
        cloned_state->current = no_thread;
        progress = false;
      }
      break;
    case abstraction::op_class::wait_not:
      if (cloned_state->conditionals.test(sigma->variable)) {
        cloned_state->current = no_thread;
        progress = false;
      }
      break;
    case abstraction::op_class::yield:
      cloned_state->current = no_thread;
      break;
  }
  if (!apply_bad_trace_dnf(cloned_state, sigma)) return nullptr;
  return cloned_state;
}

bool concurrent_automaton::apply_bad_trace_dnf(pcstate& cloned_state, const psymbol& sigma) const
{
  const location& loc = sigma->loc;
  auto before = location_map_before.find(loc);
  if (before != location_map_before.end()) {
    for (unsigned pos : before->second) {
      cloned_state->found_before.set(pos);
    }
  }
  auto after = location_map_after.find(loc);
  if (after != location_map_after.end()) {
    for (unsigned pos : after->second) {
      if (cloned_state->found_before.test(pos)) {
        // the first position was reached, we also reach the second position now
        cloned_state->found_after.set(pos);
        // now let's see if this violates any of the constraints of the dnf
        // (for the dnf to be violated it needs to be violated on all atoms in the conjunction)
        for (const auto& bad : bad_traces) {
          // at least one of the conjuncts is fulfilled
          if ((cloned_state->found_after & bad) == bad) {
            return false;
          }
        }
      }
    }
  }
  return true;
}



inline void concurrent_automaton::next_single(const pcstate& state, concurrent_automaton::Symbol_set& symbols, unsigned int thread) const
{
  for (const cfg::reward_symbol& s : threads[thread].next_symbols((*state)[thread])) {
    if (successor_filter.empty() || s.symbol->is_epsilon() || successor_filter.find(s.symbol)!=successor_filter.end())
    symbols.insert(s.symbol);
  }
}

void concurrent_automaton::add_forbidden_traces(const synthesis::dnf_constr& forbidden_traces)
{
  for (const synthesis::conj_constr& conj : forbidden_traces) {
    bad_traces.emplace_back(bad_traces_size);
    for (const synthesis::constraint_atom& ca : conj) {
      bad_traces.back().push_back(true);
      location_map_before[ca.before.symbol->loc].insert(bad_traces_size);
      location_map_after[ca.after.symbol->loc].insert(bad_traces_size);
      ++bad_traces_size;
    }
  }
  
  // convert bad_traces
  for (auto& bad : bad_traces) {
    bad.resize(bad_traces_size, false);
  }
  
}

