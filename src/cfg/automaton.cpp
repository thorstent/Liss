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

bool automaton::int_is_final_state(const state_id& state) const {
  if (state>=0)
    return thread_.get_state(state).final;
  else {
    for (const edge& e : thread_.get_successors(state * -1)) {
      if (e.tag == nullptr && thread_.get_state(e.to).final) return true;
    }
  }
  return false;
}

void automaton::int_successors(const state_id& state, const reward_symbol& sigma, State_set& successors) const {
  if (state<0) {
    // we are after this state, examine the successor edges
    // we are only after the state if there are several successors
    for (const edge& e : thread_.get_successors(state * -1)) {
      if(std::equal_to<abstraction::psymbol>()(e.tag.get(), sigma.symbol)) {
        reward_t reward = e.cost*-2;
        sigma.reward = reward;
        successors.insert(e.to);
        break;
      } else if(std::equal_to<abstraction::psymbol>()(thread_.get_state(e.to).action.get(), sigma.symbol)) {
        reward_t reward = e.cost*-2;
        if (reward==0) reward = 1;
        sigma.reward = reward;
        auto su = thread_.get_successors(e.to);
        if (su.size() == 1) 
          successors.insert(su.front().to);
        else {
          successors.insert(e.to*-1);
        }
        break;
      }
    }
  } else {
    if (std::equal_to<abstraction::psymbol>()(thread_.get_state(state).action.get(), sigma.symbol)) {
      auto su = thread_.get_successors(state);
      if (su.size() == 1) {
        successors.insert(su.front().to);
      } else {
        successors.insert(state*-1);
      }
      sigma.reward = 1;
    }
  }
}

void automaton::int_next_symbols(const state_id& state, Symbol_set& symbols) const {
  if (state<0) {
    // we are after this state, examine the successor edges
    // we are only after the state if there are several successors
    for (const edge& e : thread_.get_successors(state * -1)) {
      //assert(e.tag != nullptr); 
      auto action = e.tag;
      reward_t reward = e.cost*-2;
      if (action == nullptr) {
        action = thread_.get_state(e.to).action;
        if (reward==0) reward = 1;
      }
      if (action != nullptr) 
        symbols.insert(reward_symbol(reward, action.get()));
    }
  } else {
    if (thread_.get_state(state).action!=nullptr)
      symbols.insert(reward_symbol(1,thread_.get_state(state).action.get()));
  }
}

void automaton::int_initial_states(Limi::automaton< state_id, abstraction::psymbol, automaton >::State_set& states) const
{
  assert (thread_.initial_states().size() == 1);
  state_id init = *thread_.initial_states().begin();
  states.insert(-init);
}
