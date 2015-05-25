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

#ifndef ABSTRACTION_MULTI_AUTOMATON_H
#define ABSTRACTION_MULTI_AUTOMATON_H

#include <memory>
#include <cassert>


#include "concurrent_state.h"
#include <Limi/automaton.h>
#include <synthesis/constraint.h>
#include <boost/dynamic_bitset.hpp>
#include "cfg/program.h"
#include "cfg/automaton.h"

namespace abstraction {
  
  class concurrent_automaton : public Limi::automaton<pcstate,psymbol,concurrent_automaton> {
  public:
    concurrent_automaton(const cfg::program& program, bool concurrent, bool collapse_epsilon = false);
    bool int_is_final_state(const pcstate& state) const;
    
    void int_initial_states(State_set& states) const;
    
    void int_successors(const pcstate& state, const psymbol& sigma, State_set& successors) const;
    
    void int_next_symbols(const pcstate& state, Symbol_set& symbols) const;
    
    inline Limi::printer_base<pcstate>* int_state_printer() const { return new Limi::printer<pcstate>(threads, identifier_store_); }
    
    inline bool int_is_epsilon(const psymbol& symbol) const {
      return symbol->is_epsilon();
    }
    
    /**
     * @brief This set gives a set of allowed non-epsilon successors
     * 
     * It is used during verification of a trace
     * 
     */
    std::unordered_set<psymbol> successor_filter;
    
    void add_forbidden_traces(const synthesis::dnf& forbidden_traces);
  private:
    std::vector<cfg::automaton> threads;
    std::vector<const cfg::abstract_cfg*> cfgs;
    const abstraction::identifier_store& identifier_store_;
    bool concurrent_;

    /**
     * @brief Applies a symbol to a state
     * 
     * @param sigma The symbol
     * @param progress Returns true if the program can advance to the next program location
     * @param original_state The state we are coming from
     * @return abstraction::pcstate If it is possible to apply the symbol than this is a copy of the original state with the symbol applied, otherwise null
     */
    pcstate apply_symbol(const pcstate& original_state, const psymbol& sigma, bool& progress) const;
    
    /**
     * @brief Apply happens-before constraints from the dnf
     * 
     * The function checks if the location matches a known location in the dnf
     * of bad traces. If so it markes this in the state. If the dnf is violated
     * the function returns false.
     * 
     * @param cloned_state The state (will be changed to reflect which ones are hit)
     * @return true if this successor is allowed, false otherwise
     */
    bool apply_bad_trace_dnf(pcstate& cloned_state, const psymbol& sigma) const;
    void next_single(const pcstate& state, Symbol_set& symbols, unsigned thread) const;
    
    std::vector<boost::dynamic_bitset<unsigned long >> bad_traces; // these patterns match the bad traces
    std::unordered_map<location,std::unordered_set<unsigned>> location_map_before; // maps locations to the bits in the first bitset
    std::unordered_map<location,std::unordered_set<unsigned>> location_map_after; // maps locations to the bits in the second bitset
    unsigned bad_traces_size = 0;
  };
}

#endif // ABSTRACTION_MULTI_AUTOMATON_H
