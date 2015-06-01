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

#ifndef ABSTRACTION_COMPRESSED_AUTOMATON_H
#define ABSTRACTION_COMPRESSED_AUTOMATON_H

#include <Limi/automaton.h>
#include <vector>
#include "concurrent_automaton.h"

namespace abstraction {
  using com_symbol = uint16_t;
  using com_state = uint32_t;
  template <class Symbol>
  struct compressed_automaton_independence {
    compressed_automaton_independence(const std::vector<Symbol>& translation_table) : translation_table(translation_table) {}
    inline bool operator()(const com_symbol& a, const com_symbol& b) const {
      return Limi::independence<Symbol>()(translation_table[a],translation_table[b]);
    }
  private:
    const std::vector<Symbol>& translation_table;
  };

  template <class Symbol>
  class compressed_automaton : public Limi::automaton<com_state,com_symbol,compressed_automaton<Symbol>> {
  public:
    using State_set = std::unordered_set<com_state>;
    using Symbol_set = std::unordered_set<com_symbol>;
    compressed_automaton() : Limi::automaton<com_state,com_symbol,compressed_automaton<Symbol>>(false, false, true) {}
    inline bool int_is_final_state(const com_state& state) const {
      return final[state];
    }
    
    inline void int_initial_states(State_set& states) const { states.insert(initial_states.begin(), initial_states.end()); }
    
    inline void int_successors(const com_state& state, const com_symbol& sigma, State_set& successors) const {
      auto it = transitions[state].find(sigma);
      if (it != transitions[state].end())
        successors.insert(it->second.begin(), it->second.end());
      assert(successors.size()>0);
    }
    
    void int_next_symbols(const com_state& state, Symbol_set& symbols) const {
      for (const auto& it : transitions[state]) {
        symbols.insert(it.first);
      }
    }
    
    inline Limi::printer_base<com_symbol>* int_symbol_printer() const { return new printer(symbol_translation); }
    
    inline bool int_is_epsilon(const com_symbol& symbol) const { return epsilon[symbol]; }
    
    friend compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca);
    friend compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca, const compressed_automaton<abstraction::psymbol>& concurrent);
    
    compressed_automaton_independence<Symbol> get_independence() {
      return compressed_automaton_independence<Symbol>(symbol_translation);
    }
  private:
    // variables
    State_set initial_states;
    std::vector<bool> final;
    std::vector<bool> epsilon;
    std::vector<Symbol> symbol_translation;
    std::vector<std::unordered_map<com_symbol,std::vector<com_state>>> transitions; // a pair of symbol and successor for that symbol
    
    struct printer : public Limi::printer_base<com_symbol> {
      printer(const std::vector<Symbol>& translation_table) : translation_table(translation_table) {}
      inline virtual void print(const com_symbol& symbol, std::ostream& out) const {
        out << translation_table[symbol];
      }
    private:
      const std::vector<Symbol>& translation_table;
    };
  };
  compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca);
  compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca, const compressed_automaton<abstraction::psymbol>& concurrent);


}

#endif // ABSTRACTION_COMPRESSED_AUTOMATON_H
