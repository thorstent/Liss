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
  
  struct reward_state {
    reward_t reward;
    state_id_type state;
    reward_state(state_id_type state, reward_t cost) : reward(cost), state(state) {}
    explicit reward_state(state_id_type state) : reward(0), state(state) {}
    inline bool operator==(const reward_state& other) const { return state == other.state; }
  };
  
  inline std::ostream& operator<<(std::ostream& os, const reward_state& s) {
    os << s.state;
    if (s.reward!=0) os << "(" << s.reward << ")";
    return os;
  }
  
  struct state_printer : Limi::printer_base<reward_state> {
    state_printer(const cfg::abstract_cfg& thread) : thread_(thread) {}
    virtual void print(const reward_state& state_, std::ostream& out) const override {
      state_id_type state = state_.state;
      if (state==no_state) {
        out << "no state";
      } else {
        if (state<0) {
          out << "a";
          out << thread_.get_state(state*-1);
        } else {
          //out << "b";
          out << thread_.get_state(state);
        }}
    }
  private:
    const cfg::abstract_cfg& thread_;
  };
}

namespace std {
  template<> struct hash<cfg::reward_state> {
    size_t operator()(const cfg::reward_state& cs) const {
      return hash<state_id_type>()(cs.state);
    }
  };
}

namespace cfg {

  class automaton : public Limi::automaton<reward_state,abstraction::psymbol,automaton> {
  public:
    automaton(const abstract_cfg& thread, bool collapse_epsilon = false) : Limi::automaton<reward_state,abstraction::psymbol,automaton>(collapse_epsilon), thread_(thread) {}
    bool int_is_final_state(const reward_state& state) const;
    
    void int_initial_states(State_vector& states) const;
    
    void int_successors(const reward_state& state, const abstraction::psymbol& sigma, State_vector& successors) const;
    
    void int_next_symbols(const reward_state& state, Symbol_vector& symbols) const;
    
    inline Limi::printer_base<reward_state>* int_state_printer() const { return new cfg::state_printer(thread_); }
    
    inline bool int_is_epsilon(const abstraction::psymbol& symbol) const { return symbol->is_epsilon(); }
    
  private:  
    const abstract_cfg& thread_;
  };
}

#endif // CFG_AUTOMATON_H
