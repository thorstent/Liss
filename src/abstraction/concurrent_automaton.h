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
#include "synthesis/trace_helpers.h"

namespace abstraction {
  
  class concurrent_automaton : public Limi::automaton<pcstate,psymbol,concurrent_automaton> {
  public:
    /**
     * @brief ...
     * 
     * @param program ...
     * @param concurrent ...
     * @param collapse_epsilon ...
     * @param deadlock_automaton If true every exit state is split, it may continue or terminate
     */
    concurrent_automaton(const cfg::program& program, bool concurrent, bool collapse_epsilon = false, bool deadlock_automaton = false);
    bool int_is_final_state(const pcstate& state) const;
    
    void int_initial_states(State_vector& states) const;
    
    void int_successors(const abstraction::pcstate& state, const abstraction::psymbol& sigma, State_vector& successors) const;
    
    void int_next_symbols(const pcstate& state, Symbol_vector& symbols) const;
    
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
    
    void add_forbidden_traces(const synthesis::lock_symbols& new_locks);
  private:
    std::vector<cfg::automaton> threads;
    std::vector<const cfg::abstract_cfg*> cfgs;
    const abstraction::identifier_store& identifier_store_;
    bool concurrent_;
    bool deadlock_; // deadlock automaton

    /**
     * @brief Applies a symbol to a state
     * 
     * @param sigma The symbol
     * @param original_state The state we are coming from
     * @return abstraction::pcstate If it is possible to apply the symbol than this is a copy of the original state with the symbol applied, otherwise null
     */
    pcstate apply_symbol(const abstraction::pcstate& original_state, const abstraction::psymbol& sigma, bool& progress) const;
    
    /**
     * @brief Adds successors for final states that cannot proceed
     * 
     * @param cloned_state ...
     * @param thread The thread that was advanced
     * @param successors ...
     * @return void
     */
    void deadlock_states(const abstraction::pcstate& cloned_state, thread_id_type thread, State_vector& successors) const;
    
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
    bool apply_bad_trace_dnf(abstraction::pcstate& cloned_state, const abstraction::psymbol& sigma) const;
    /**
     * @brief Tick of the lock in the cloned state.
     * 
     * Also checks if now any disjunct of locks is violated

     * @return false if the lock is violated
     */
    bool tick_lock(abstraction::pcstate& cloned_state, int lock) const;
    void next_single(const abstraction::pcstate& state, Symbol_vector& symbols, thread_id_type thread) const;
    
    /** A conflict
     * means that between past and next there happens a conflicting instruction
     */
    struct conflict_info {
      psymbol next;
      std::unordered_set<psymbol> conflict;
      inline thread_id_type next_thread() const { return next->thread_id(); }
      inline thread_id_type conflict_thread() const { return (*conflict.begin())->thread_id(); }
      int lock_id; // the lock that should be ticked (if -1 then terminate right away)
      conflict_info(std::unordered_set<psymbol> conflict, psymbol next, unsigned lock_id) : next(next), conflict(conflict), lock_id(lock_id) {}
    };
    
    std::vector<conflict_info> conflicts;
    std::unordered_multimap<psymbol, unsigned> conflict_map;
    
    // locks are represented in a dnf
    std::vector<boost::dynamic_bitset<unsigned>> locks; // these patterns match the locks
    unsigned locks_size = 0;
  };
}

#endif // ABSTRACTION_MULTI_AUTOMATON_H
