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

#ifndef AUTOMATA_RESULTS_H
#define AUTOMATA_RESULTS_H

#include <unordered_set>
#include <list>
#include <algorithm>
#include <memory>
#include <Limi/generics.h>

namespace automata {
  
  /**
   * @brief Represents the result of the deadlock algorithm.
   * 
   * If \ref deadlock_result is false, than all other fields of this struct must be ignored (they are not set).
   * 
   * @tparam Symbol The type of symbols
   * @tparam State The type of states.
   * 
   */
  template <class Symbol, class State>
  struct deadlock_result {
    /**
     * @brief True if a deadlock was found
     * 
     */
    bool deadlock_found = false;
    /**
     * @brief True if this is a deadlock. False if it is a lifelock.
     * 
     */
    bool no_successor = false;
    /**
     * @brief The state that is deadlocked
     * 
     */
    State dead_state;
    /**
     * @brief A list of successors this state has, but none is activated
     * 
     * This set is only filled if \ref Limi::automaton::int_next_symbols() returns a strict
     * superset of the symbols that actually procduce successors using \ref Limi::automaton::int_successors().
     * 
     */
    std::vector<Symbol> impossible_successors;
    /**
     * @brief A list of symbols that lead to \ref dead_state.
     * 
     */
    std::list<Symbol> counter_example;
    /**
     * @brief If it is a livelock contains a loop that leads back to \ref dead_state.
     * 
     */
    std::list<Symbol> loop;
    
    /**
     * @brief Print this result.
     * 
     * @param stream The stream to print to.
     * @param symbol_printer The printer to print out the counter-example if any.
     * @param state_printer The printer for states (such as \ref dead_state)
     */
    void print_long(std::ostream& stream, const Limi::printer_base<Symbol>& symbol_printer, const Limi::printer_base<State>& state_printer) {
      if (!deadlock_found)
        stream << "No deadlock found" << std::endl;
      else {
        stream << "Deadlock";
        stream << std::endl;
        if (no_successor) stream << "Dead state (no successor)"; else stream << "No final state is reachable from";
        stream << ": " << state_printer(dead_state) << std::endl;
        if (no_successor) {
          if (!impossible_successors.empty()) {
            stream << "Impossible successors:" << std::endl;
            for (const auto& sy : impossible_successors) {
              stream << symbol_printer(sy);
              stream << std::endl;
            }
          } else {
            stream << "No successors at all." << std::endl;
          }
        } else {
          if (!loop.empty()) {
            stream << "Loop:" << std::endl;
            for (const auto& sy : loop) {
              stream << symbol_printer(sy);
              stream << std::endl;
            }
          }
        }
        stream << "Trace:" << std::endl;
        for (const auto& s : counter_example) {
          stream << symbol_printer(s);
          stream << std::endl;
        }
      }
    }
  };
  
}

#endif //AUTOMATA_RESULTS_H