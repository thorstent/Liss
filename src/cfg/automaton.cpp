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

#include "automaton.h"

using namespace cfg;

bool automaton::int_is_final_state(const reward_state& state_) const {
  state_id_type state = state_.state;
  if (state>=0)
    return thread_.get_state(state).final;
  else {
    for (const edge& e : thread_.get_successors(state * -1)) {
      if (e.tag == nullptr && thread_.get_state(e.to).final) return true;
    }
  }
  return false;
}

void automaton::int_successors(const reward_state& state_, const abstraction::psymbol& sigma, State_vector& successors) const {
  state_id_type state = state_.state;
  if (state<0) {
    // we are after this state, examine the successor edges
    // we are only after the state if there are several successors
    for (const edge& e : thread_.get_successors(state * -1)) {
      if(std::equal_to<abstraction::psymbol>()(e.tag.get(), sigma)) {
        reward_t reward = e.cost*-1;
        successors.push_back(reward_state(e.to, reward));
        break;
      } else if(std::equal_to<abstraction::psymbol>()(thread_.get_state(e.to).action.get(), sigma)) {
        reward_t reward = e.cost*-1;
        if (reward==0) reward = 1;
        auto su = thread_.get_successors(e.to);
        if (su.size() == 1) 
          successors.push_back(reward_state(su.front().to, reward));
        else {
          successors.push_back(reward_state(e.to*-1, reward));
        }
        break;
      }
    }
  } else {
    if (std::equal_to<abstraction::psymbol>()(thread_.get_state(state).action.get(), sigma)) {
      reward_t reward = 1;
      auto su = thread_.get_successors(state);
      if (su.size() == 1) {
        successors.push_back(reward_state(su.front().to, reward));
      } else {
        successors.push_back(reward_state(state*-1, reward));
      }
    }
  }
}

void automaton::int_next_symbols(const reward_state& state_, Symbol_vector& symbols) const {
  state_id_type state = state_.state;
  if (state<0) {
    // we are after this state, examine the successor edges
    // we are only after the state if there are several successors
    for (const edge& e : thread_.get_successors(state * -1)) {
      //assert(e.tag != nullptr); 
      abstraction::psymbol action = e.tag.get();
      reward_t reward = e.cost*-2;
      if (action == nullptr) {
        action = thread_.get_state(e.to).action.get();
        if (reward==0) reward = 1;
      }
      if (action != nullptr)
        symbols.push_back(action);
    }
  } else {
    if (thread_.get_state(state).action!=nullptr)
      symbols.push_back(thread_.get_state(state).action.get());
  }
}

void automaton::int_initial_states(State_vector& states) const
{
  assert (thread_.initial_states().size() == 1);
  state_id_type init = *thread_.initial_states().begin();
  states.push_back(reward_state(-init));
}
