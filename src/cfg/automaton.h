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

#ifndef CFG_AUTOMATON_H
#define CFG_AUTOMATON_H

#include "abstraction/symbol.h"
#include "cfg/abstract_cfg.h"
#include <Limi/automaton.h>

namespace cfg {
  
  // the idea is that a negative number indicates we are after the state
  
  struct state_printer : Limi::printer_base<state_id_type> {
    state_printer(const cfg::abstract_cfg& thread) : thread_(thread) {}
    virtual void print(const state_id_type& state, std::ostream& out) const override {
      if (state<0) {
        out << "a";
        out << thread_.get_state(state*-1);
      } else {
        //out << "b";
        out << thread_.get_state(state);
      }
    }
  private:
    const cfg::abstract_cfg& thread_;
  };
}


namespace cfg {

  class automaton : public Limi::automaton<state_id_type,abstraction::psymbol,automaton> {
  public:
    automaton(const abstract_cfg& thread, bool collapse_epsilon = false) : Limi::automaton<state_id_type,abstraction::psymbol,automaton>(collapse_epsilon, false), thread_(thread) {}
    bool int_is_final_state(const state_id_type& state) const;
    
    void int_initial_states(State_set& states) const;
    
    void int_successors(const state_id_type& state, const abstraction::psymbol& sigma, State_set& successors) const;
    
    void int_next_symbols(const state_id_type& state, Symbol_set& symbols) const;
    
    inline Limi::printer_base<state_id_type>* int_state_printer() const { return new cfg::state_printer(thread_); }
    
    inline bool int_is_epsilon(const abstraction::psymbol& symbol) const { return symbol->is_epsilon(); }
    
  private:  
    const abstract_cfg& thread_;
  };
}

#endif // CFG_AUTOMATON_H
