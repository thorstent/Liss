/*
 * Copyright 2016, IST Austria
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

#ifndef AUTOMATA_PERF_H
#define AUTOMATA_PERF_H

#include <Limi/automaton.h>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <deque>
#include <cassert>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <Limi/results.h>
#include <Limi/internal/helpers.h>
#include "options.h"

namespace automata {
  
  template <class State, class Symbol, class Implementation, class Independence = Limi::independence<Symbol>>
  class perf_test
  {
  protected:
    typedef Limi::counterexample_chain<Symbol> counter_chain;
    typedef std::shared_ptr<counter_chain> pcounter_chain;
    typedef std::vector<State> State_vector;
    typedef std::vector<Symbol> Symbol_vector;
    typedef std::unordered_set<Symbol> Symbol_set;
    typedef Limi::automaton<State, Symbol, Implementation> Automaton;
    
  protected:
    
    struct frontier_item {
      State s;
      Symbol_set sleep_set;
      explicit frontier_item(State s) : s(s), cex_chain(nullptr) {}
      frontier_item(State s, const pcounter_chain& parent, const Symbol& sym, const Symbol_set& sleep_set) : frontier_item(s)
      { this->sleep_set = sleep_set; }
      
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
    std::unordered_map<State,Symbol_set> seen; // a list of reachable states
    std::deque<frontier_item> frontier = initial_states_conv(initial_states);
    Independence independence_;
    
  public:
    
    perf_test(const Automaton& automaton, const Independence& independence = Independence()) : autom(automaton), independence_(independence)
    {  
    }
        
    void run()
    {
      unsigned loop_counter = 0;
      unsigned transitions = 0;
      while (frontier.size() > 0) {
        ++loop_counter;
        if (verbosity>=2 && loop_counter % 100 == 0) debug << loop_counter << " rounds; seen states: " << seen.size() << std::endl;
        frontier_item current = frontier.front();
        frontier.pop_front();
        
        Symbol_set next_symbols;
        auto it = seen.find(current.s);
        if (it == seen.end()) { // not found
          seen.insert(std::make_pair(current.s, current.sleep_set));
          autom.next_symbols(current.s, next_symbols);
          Limi::internal::set_remove(next_symbols, current.sleep_set);
        } else { // found
          next_symbols = it->second;
          Limi::internal::set_remove(next_symbols, current.sleep_set);
          Limi::internal::set_intersect(current.sleep_set, it->second);
          it->second = current.sleep_set;
        }
        
        for (Symbol sigma : next_symbols) {
          ++transitions;   
          State_vector states = autom.successors(current.s, sigma);
          
          Symbol_set next_sleep_set(current.sleep_set);
          // sort out the non-independent ones
          for (auto it = next_sleep_set.begin(); it!=next_sleep_set.end(); ) {
            if (!independence_(*it, sigma)) 
              it = next_sleep_set.erase(it);
            else
              ++it;
          }
          
          for (State state : states) {
         
            frontier_item next(state, current.cex_chain, sigma, next_sleep_set);
            frontier.push_front(next);
          }
          current.sleep_set.insert(sigma);
        }
      }
      if (verbosity>=1) debug << loop_counter << " rounds; seen states: " << seen.size() << "; transitions: " << transitions << std::endl;
    }
  };
  
  
}

#endif // AUTOMATA_PERF_H
