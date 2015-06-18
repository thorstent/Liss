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

concurrent_automaton::concurrent_automaton(const cfg::program& program, bool concurrent, bool collapse_epsilon, bool deadlock_automaton) : 
Limi::automaton<pcstate,psymbol,concurrent_automaton>(collapse_epsilon),
identifier_store_(program.identifiers()),
concurrent_(concurrent), deadlock_(deadlock_automaton)
{
  for (const cfg::abstract_cfg* thread : program.minimised_threads()) {
    cfgs.push_back(thread);
    threads.push_back(cfg::automaton(*thread));
  }
}


bool concurrent_automaton::int_is_final_state(const pcstate& state) const
{
  for (thread_id_type i = 0; i<state->length; ++i) {
    if ((*state)[i]!=no_state && !threads[i].is_final_state((*state)[i]))
      return false;
  }
  return true;
}

void concurrent_automaton::int_initial_states(concurrent_automaton::State_set& states) const
{
  states.insert(make_shared<concurrent_state>(threads.size(), locks_size));
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

void concurrent_automaton::int_successors_hist(const pcstate& state, const psymbol& sigma, const std::shared_ptr<Limi::counterexample_chain<psymbol>>& history, concurrent_automaton::State_set& successors) const
{
  thread_id_type thread = sigma->thread_id();
  if (state->current != no_thread && state->current != static_cast<int>(thread)) return;
  const cfg::automaton::State_set succs = threads[thread].successors(state->threads[thread], sigma); // cost is not relevant here
  assert(!succs.empty());
  bool progress;
  pcstate next = apply_symbol(state, sigma, history, progress);
  
  if (next) {
    if (!progress) {
      successors.insert(next);
      return;
    }
    //cout << threads[thread]->name(next->threads[thread]) << " -> ";
    bool first = true;
    for (const state_id_type& p : succs) {
      pcstate copy = first ? next : make_shared<concurrent_state>(*next); // copy needed if not first element
      next->reward += 1;
      // bound at 100
      if (history && history->size()%400 == 0) {
        next->reward =- 1000;
      }
      copy->threads[thread] = p;
      //cout << threads[thread]->name(p) << " ";
      if (concurrent_ || threads[thread].is_final_state(p)) copy->current = no_thread;
      successors.insert(copy);
      deadlock_states(copy, thread, successors);
      first = false;
    }
    //cout << successors.size() << endl;
  }
}

pcstate concurrent_automaton::apply_symbol(const pcstate& original_state, const psymbol& sigma, const std::shared_ptr<Limi::counterexample_chain<psymbol>>& history, bool& progress) const
{
  progress = true;
  
  if (original_state->current == no_thread || (sigma->assume&&!assumes_allow_switch) || sigma->synthesised) {
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
        if (!(original_state->current == no_thread || (sigma->assume&&!assumes_allow_switch) || sigma->synthesised)) {
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

bool concurrent_automaton::tick_lock(pcstate& cloned_state, unsigned int lock) const
{
  bool old = cloned_state->locksviolated.test(lock);
  if (!old) {
    cloned_state->locksviolated.set(lock);
    // this bit was not set before
    // check if we want to throw away this execution
    for (const auto& bad : locks) {
      // at least one of the conjuncts is fulfilled
      if ((cloned_state->locksviolated & bad) == bad) {
        // print
        return false;
      }
    }
  }
  return true;
}


/** How the conflicts work: sigma is the current symbol, thread is the thread of sigma
 * 1. Check existing conflicts. They fall into two categories, first check those which have not seen the conflict (clean)
 *    a) if conflict.conflict == sigma, then replace by tagged conflict (meaning we saw the conflict)
 *    b) else if thread == next_thread remove from set
 * 2. Check those who saw the conflict (tagged)
 *    a) if conflict.next == sigma, mark lock violated and remove from set
 *    b) else if thread == next_thread remove from set
 * 3. Check for new conflicts:
 *    a) if sigma in conflict_map add conflict
 */

bool concurrent_automaton::apply_bad_trace_dnf(pcstate& cloned_state, const psymbol& sigma) const
{
  // we don't deal with tags
  if (sigma->is_real_epsilon() || sigma->operation == op_class::tag)
    return true;
  
  thread_id_type thread = sigma->thread_id();
  // check existing conflicts
  for (auto itc = cloned_state->conflicts.begin(); itc != cloned_state->conflicts.end();) {
    const conflict_t& c = *itc;
    const conflict_info& ci = conflicts[c.payload()];
    bool del = false; // delete
    if (!c.test()) {
      // have not seen the conflict yet
      if (ci.next_thread() == thread) {
        // no longer valid
        del = true;
      } else if (ci.conflict.find(sigma) != ci.conflict.end()) {
        // escalate to conflict
        del = true;
        conflict_t c_ = c;
        c_.set();
        cloned_state->conflicts.insert(c_);
      }
    } else {
      // we are already in a conflict
      if (ci.next_thread() == thread) {
        // no longer needed
        del = true;
        if (equal_to<psymbol>()(sigma, ci.next)) {
          return tick_lock(cloned_state, ci.lock_id);
        }
      }
    }
    if (del)
      itc = cloned_state->conflicts.erase(itc);
    else
      ++itc;
  }
  // maybe this is a new conflict
  auto range = conflict_map.equal_range(sigma);
  for_each(range.first, range.second, [&cloned_state](const unordered_multimap<psymbol, unsigned>::value_type& c){cloned_state->conflicts.insert(conflict_t(c.second));});
  return true;
}



inline void concurrent_automaton::next_single(const pcstate& state, concurrent_automaton::Symbol_set& symbols, thread_id_type thread) const
{
  if ((*state)[thread]!=no_state) {
    for (const abstraction::psymbol& s : threads[thread].next_symbols((*state)[thread])) {
      if (successor_filter.empty() || s->is_epsilon() || successor_filter.find(s)!=successor_filter.end())
        symbols.insert(s);
    }
  }
}

void concurrent_automaton::deadlock_states(const pcstate& cloned_state, thread_id_type thread, Limi::automaton< pcstate, psymbol, concurrent_automaton >::State_set& successors) const
{
  if (deadlock_) {
    state_id_type state = (*cloned_state)[thread];
    if (threads[thread].is_final_state(state)) {
      shared_ptr<concurrent_state> dup = make_shared<concurrent_state>(*cloned_state);
      (*dup)[thread] = no_state;
      successors.insert(dup);
    }
  }
}


void concurrent_automaton::add_forbidden_traces(const synthesis::lock_symbols& new_locks)
{
  for (const disj<synthesis::lock_lists>& lockd : new_locks) {
    // one of these locks needs to hold
    locks.emplace_back(locks_size);
    for (const synthesis::lock_lists& lock : lockd) {
      // one single lock
      locks.back().push_back(true);
      // define conflicts mutually between locations
      for (auto it = lock.begin(); it!=lock.end(); ++it) {
        // each conflict in this vector is in conflict with all the other vectors
        psymbol pred = nullptr;
        for (const psymbol& current : (*it)) {
          if (!current->is_real_epsilon()) {
            if (pred) {
              for (auto it2 = lock.begin(); it2!=lock.end(); ++it2) {
                // locks talking about the same thread are ignored
                if (it->front()->thread_id()!=it2->front()->thread_id()) {
                  auto last = it2->end();
                  --last;
                  // create a list of conflicting symbols
                  std::unordered_set<psymbol> conflicting;
                  for (auto pl = it2->begin(); pl != it2->end(); ++pl) {
                    psymbol conflict = *pl;
                    if (!conflict->is_real_epsilon()) {
                      conflicting.insert(conflict);
                    }
                  }
                  if (!conflicting.empty()) {
                    conflicts.emplace_back(conflicting, current, locks_size);
                    conflict_map.insert(make_pair(pred, conflicts.size()-1));
                  }
                }
              }
            } // if(pred)
            pred = current;
          } // for lock
        }
        
        
      }
      ++locks_size;
    }
  }
  
  // convert locks to correct length
  for (auto& bad : locks) {
    bad.resize(locks_size, false);
  }
  
}

