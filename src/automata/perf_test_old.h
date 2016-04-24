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

#ifndef AUTOMATA_PERF_OLD_H
#define AUTOMATA_PERF_OLD_H
#include <Limi/automaton.h>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <deque>
#include <cassert>
#include <memory>
#include <iostream>
#include <Limi/results.h>
#include <Limi/internal/helpers.h>
#include "options.h"

namespace automata {
  
  template <class State, class Symbol, class Implementation>
  class perf_test_old
  {
  protected:
    typedef Limi::counterexample_chain<Symbol> counter_chain;
    typedef std::shared_ptr<counter_chain> pcounter_chain;
    typedef std::unordered_set<State> State_set;
    typedef std::unordered_set<Symbol> Symbol_set;
    typedef std::vector<State> State_vector;
    typedef std::vector<Symbol> Symbol_vector;
    typedef Limi::automaton<State, Symbol, Implementation> Automaton;
    
  protected:
    
    struct frontier_item {
      State s;
      explicit frontier_item(State s) : s(s), cex_chain(nullptr) {}
      frontier_item(State s, const pcounter_chain& parent, const Symbol& sym) : frontier_item(s)
      { }
      
      pcounter_chain cex_chain;
    };
    
    std::deque<frontier_item> initial_states_conv(const std::vector<State>& states) {
      std::deque< frontier_item > result;
      for(State state : states) {
        frontier_item p(state);
        result.push_back(p);
      }
      return result;
    }
    
  protected:
    const Automaton& autom;
    
    std::vector<State> initial_states = autom.initial_states();
    std::unordered_set<State> seen; // a list of reachable states
    std::deque<frontier_item> frontier = initial_states_conv(initial_states);
    
  public:
    
    perf_test_old(const Automaton& automaton) : autom(automaton)
    {  
    }
    
    void run()
    {
      unsigned loop_counter = 0;
      unsigned transitions = 0;
      while (frontier.size() > 0) {
        ++loop_counter;
        if (verbosity>=2 && loop_counter % 100 == 0) debug << loop_counter << " rounds; seen states: " << seen.size() << std::endl;
        const frontier_item current = frontier.front();
        frontier.pop_front();
        Symbol_vector next_symbols = autom.next_symbols(current.s);
        for (Symbol sigma : next_symbols) {
          ++transitions;
          State_vector states = autom.successors(current.s, sigma);
          
          for (State state : states) {
            if (seen.insert(state).second) {
              frontier_item next(state, current.cex_chain, sigma);
              frontier.push_front(next);
            }
          }
        }
      }
      if (verbosity>=1) debug << loop_counter << " rounds; seen states: " << seen.size() << "; transitions: " << transitions << std::endl;
    }
  };
  
  
}

#endif // AUTOMATA_PERF_OLD_H
