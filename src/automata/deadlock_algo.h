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

#ifndef AUTOMATA_DEADLOCK_ALGO_H
#define AUTOMATA_DEADLOCK_ALGO_H

#include <Limi/automaton.h>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <deque>
#include <memory>
#include <iostream>
#include "results.h"
#include <Limi/internal/helpers.h>

namespace automata {
  
  /**
   * @brief This class defines an algorithm that can deadlocks and livelocks.
   * 
   * A deadlock is a state in the automaton that has no successors, but is not a final state.
   * A livelock is a state that has successors, but none of these can reach a final state. A typical 
   * example for this case is that two threads deadlock, but a third one can still loop.
   * 
   */
  template <class State, class Symbol, class Implementation>
  class deadlock_algo
  {
    typedef Limi::counterexample_chain<Symbol> counter_chain;
    typedef std::shared_ptr<counter_chain> pcounter_chain;
    typedef std::unordered_set<State> State_set;
    typedef std::unordered_set<Symbol> Symbol_set;
    typedef Limi::automaton<State, Symbol, Implementation> Automaton;
    
    struct frontier_item {
      State s;
      explicit frontier_item(State s) : s(s), cex_chain(nullptr) {}
      frontier_item(State s, const pcounter_chain& parent, const Symbol& sym) : frontier_item(s)
      {cex_chain = std::make_shared<counter_chain>(sym, parent); }
      
      pcounter_chain cex_chain;
    };
    
    std::deque<frontier_item> initial_states_conv(const std::unordered_set<State>& states) {
      std::deque< frontier_item > result;
      for(State state : states) {
        frontier_item p(state);
        result.push_back(p);
      }
      return result;
    }
    
    
  public:
    
    /**
     * @brief Finds deadlocks and livelocks.
     * 
     * @param automaton The automaton. 
     * 
     * @return Returns information about a deadlock or livelock if found.
     */
    deadlock_result<Symbol,State> run(const Automaton& automaton)
    {
      
      std::unordered_set<State> initial_states = automaton.initial_states();
      std::unordered_set<State> reachable; // a list of reachable states
      std::deque<frontier_item> frontier = initial_states_conv(initial_states);
      std::unordered_map<State,std::unordered_set<State>> predecessor_map;
      std::unordered_set<State> final_states;
      
      deadlock_result<Symbol,State> result;
      result.deadlock_found = false;
      
      #ifdef DEBUG_PRINTING
      unsigned loop_counter = 0;
      #endif
      while (frontier.size() > 0) {
        #ifdef DEBUG_PRINTING
        ++loop_counter;
        if (DEBUG_PRINTING>=2 && loop_counter % 100 == 0) std::cout << loop_counter << " rounds; seen states: " << reachable.size() << std::endl;
        #endif
        const frontier_item current = frontier.front();
        frontier.pop_front();
        #ifdef DEBUG_PRINTING
        if (DEBUG_PRINTING>=3) std::cout << automaton.state_printer()(current.s) << std::endl;
        #endif
        
        if (reachable.insert(current.s).second) {
          if (automaton.is_final_state(current.s))
            final_states.insert(current.s);
          bool has_successors = false;
          bool one_assumes = false;
          Symbol_set next_symbols = automaton.next_symbols(current.s);
          for (Symbol sigma : next_symbols) {
            // TODO Fix this (piercing throuh the template)
            one_assumes = one_assumes || sigma->assume;
            State_set states = automaton.successors(current.s, sigma);
            
            for (State state : states) {
              frontier_item next(state, current.cex_chain, sigma);
              frontier.push_back(next);
              has_successors = true;
              predecessor_map[state].insert(current.s);
            }
            
          }
          
          if (!automaton.is_final_state(current.s) && !has_successors) {
            if (one_assumes) {
              // add this to the final states because reaching this state is ok as well
              final_states.insert(current.s); 
            } else {
              result.counter_example = current.cex_chain->to_list();
              result.deadlock_found = true;
              result.no_successor = true;
              result.dead_state = current.s;
              result.impossible_successors = next_symbols;
              return result;
            }
          }
        }
      }
      
      /*
       * #ifdef DEBUG_PRINTING
       *      if (DEBUG_PRINTING >= 2) {
       *        std::cout << "States discovered: " << reachable.size() << std::endl;
       *        std::cout << "Marking predecessors" << std::endl;
    }
    #endif
    
    // marking the predecessors
    std::deque<State> pfrontier(final_states.begin(), final_states.end());
    std::unordered_set<State> good_states; // states that terminate
    
    while (!pfrontier.empty()) {
      const State current = pfrontier.front();
      pfrontier.pop_front();
      if (good_states.find(current)==good_states.end()) {
        good_states.insert(current);
        for (State pred : predecessor_map[current]) {
          pfrontier.push_back(pred);
    }
    }
    }
    
    #ifdef DEBUG_PRINTING
    if (DEBUG_PRINTING >= 2) {
      std::cout << "Finding non-terminating states" << std::endl;
    }
    #endif
    
    // now go through the state space once again to find a state that is not marked
    std::deque<frontier_item> frontier2 = initial_states_conv(initial_states);
    std::unordered_set<State> seen;
    while (!frontier2.empty()) {
      const frontier_item current = frontier2.front();
      frontier2.pop_front();
      if (seen.insert(current.s).second) {
        Symbol_set next_symbols = automaton.next_symbols(current.s);
        
        for (Symbol sigma : next_symbols) {
          State_set states = automaton.successors(current.s, sigma);
          for (State state : states) {
            frontier_item next(state, current.cex_chain, sigma);
            frontier2.push_front(next);
    }
    }
    
    
    if (good_states.find(current.s) == good_states.end()) {
      
      // find the loop from this state
      std::deque<frontier_item> frontier3;
      std::unordered_set<State> seen3;
      frontier3.push_back(frontier_item(current.s));
      while (!frontier3.empty()) {
        const frontier_item current3 = frontier3.front();
        frontier3.pop_front();
        if (seen3.insert(current3.s).second) {
          Symbol_set next_symbols = automaton.next_symbols(current3.s);
          
          for (Symbol sigma : next_symbols) {
            State_set states = automaton.successors(current3.s, sigma);
            for (State state : states) {
              frontier_item next(state, current3.cex_chain, sigma);
              frontier3.push_back(next);
    }
    }
    } else {
      // we found the loop
      result.loop = current3.cex_chain->to_list();
    }
    }
    
    result.counter_example = current.cex_chain->to_list();
    result.deadlock_found = true;
    result.no_successor = false;
    result.dead_state = current.s;
    result.impossible_successors = next_symbols;
    return result;
    }  
    }
    }*/
      
      return result;
    }
    
  };
  
  
}

#endif // AUTOMATA_DEADLOCK_ALGO_H
