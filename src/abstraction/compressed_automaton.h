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
#include <Limi/results.h>

namespace abstraction {
  using com_symbol = uint16_t;
  using com_state = uint32_t;
  
  template <class Symbol>
  struct compressed_automaton_independence {
    compressed_automaton_independence(const std::vector<Symbol>& translation_table) : translation_table(translation_table) {}
    inline bool operator()(const com_symbol& a, const com_symbol& b) const {
      assert (a < translation_table.size());
      assert (b < translation_table.size());
      return Limi::independence<Symbol>()(translation_table[a],translation_table[b]);
    }
  private:
    const std::vector<Symbol>& translation_table;
  };
  
  template <class Symbol>
  struct compressed_automaton_printer : public Limi::printer_base<com_symbol> {
    compressed_automaton_printer(const std::vector<Symbol>& translation_table) : translation_table(translation_table) {}
    inline virtual void print(const com_symbol& symbol, std::ostream& out) const {
      out << translation_table[symbol];
    }
  private:
    const std::vector<Symbol>& translation_table;
  };
  
  
  // wrapper automaton that translates the symbols
  template <class Implementation>
  class wrapper_automaton : public Limi::automaton<typename Implementation::State_,com_symbol,wrapper_automaton<Implementation>> {
  public:
    using Symbol = typename Implementation::Symbol_;
    using State = typename Implementation::State_;
    using State_vector = typename Implementation::State_vector;
    using Symbol_vector = std::vector<com_symbol>;
    wrapper_automaton(const Implementation& inner, std::unordered_map<Symbol,com_symbol>& symbol_map, std::vector<Symbol>& symbol_translation) :
    Limi::automaton<typename Implementation::State_,com_symbol,wrapper_automaton<Implementation>>(false, inner.no_epsilon_produced), inner(inner), symbol_map(symbol_map), symbol_translation(symbol_translation) {}
    
    inline bool int_is_final_state(const State& state) const {
      return inner.is_final_state(state);
    }
    
    inline void int_initial_states(State_vector& states) const { inner.initial_states(states); }
    
    inline void int_successors(const State& state, const com_symbol& sigma, State_vector& successors) const {
      assert (sigma<symbol_translation.size());
      inner.successors(state, symbol_translation[sigma], successors);
    }
    
    void int_next_symbols(const State& state, Symbol_vector& symbols) const {
      typename Implementation::Symbol_vector syminner;
      inner.next_symbols(state, syminner);
      for (const auto& s : syminner) {
        auto it = symbol_map.find(s);
        com_symbol sy;
        if (it==symbol_map.end()) 
          sy = add_symbol(s);
        else
          sy = it->second;
        symbols.push_back(sy);
      }
    }
    
    inline Limi::printer_base<com_symbol>* int_symbol_printer() const { return new compressed_automaton_printer<Symbol>(symbol_translation); }
    
    inline Limi::printer_base<State>* int_state_printer() const { return inner.int_state_printer(); }
        
    inline bool int_is_epsilon(const com_symbol& symbol) const { assert (symbol<symbol_translation.size()); return inner.is_epsilon(symbol_translation[symbol]); }
    
    Limi::inclusion_result<Symbol> translate_result(const Limi::inclusion_result<com_symbol>& oldres) const {
      Limi::inclusion_result<Symbol> result;
      result.included = oldres.included;
      result.bound_hit = oldres.bound_hit;
      result.max_bound = oldres.max_bound;
      for (com_symbol s : oldres.counter_example ) {
        result.counter_example.push_back(symbol_translation[s]);
      }
      return result;
    }
  private:
    const Implementation& inner;
    std::unordered_map<Symbol,com_symbol>& symbol_map;
    std::vector<Symbol>& symbol_translation;
    
    com_symbol add_symbol(const Symbol& symbol) const {
      com_symbol s = symbol_translation.size();
      symbol_translation.push_back(symbol);
      symbol_map.insert(std::make_pair(symbol, s));
      return s;
    }
  };

  template <class Symbol>
  class compressed_automaton : public Limi::automaton<com_state,com_symbol,compressed_automaton<Symbol>> {
  public:
    using State_vector = std::vector<com_state>;
    using Symbol_vector = std::vector<com_symbol>;
    compressed_automaton() : Limi::automaton<com_state,com_symbol,compressed_automaton<Symbol>>(false, true) {}
    inline bool int_is_final_state(const com_state& state) const {
      return final[state];
    }
    
    inline void int_initial_states(State_vector& states) const { states.insert(states.end(), initial_states.begin(), initial_states.end()); }
    
    inline void int_successors(const com_state& state, const com_symbol& sigma, State_vector& successors) const {
      auto it = transitions[state].find(sigma);
      if (it != transitions[state].end())
        successors.insert(successors.end(), it->second.begin(), it->second.end());
      assert(successors.size()>0);
    }
    
    void int_next_symbols(const com_state& state, Symbol_vector& symbols) const {
      for (const auto& it : transitions[state]) {
        if (successor_filter.empty() || int_is_epsilon(it.first) || successor_filter.find(it.first)!=successor_filter.end()) {
          symbols.push_back(it.first);
        }
      }
    }
    
    inline Limi::printer_base<com_symbol>* int_symbol_printer() const { return new compressed_automaton_printer<Symbol>(symbol_translation); }
    
    inline bool int_is_epsilon(const com_symbol& symbol) const { assert(symbol<epsilon.size()); return epsilon[symbol]; }
    
    friend compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca);
    friend compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca, const compressed_automaton<abstraction::psymbol>& concurrent);
    
    template <class Implementation>
    wrapper_automaton<Implementation> get_wrapper(const Implementation& inner) {
      return wrapper_automaton<Implementation>(inner, symbol_map, symbol_translation);
    }
    
    compressed_automaton_independence<Symbol>& get_independence() {
      return independence_;
    }
    
    /**
     * @brief This set gives a set of allowed non-epsilon successors
     * 
     * It is used during verification of a trace
     * 
     */
    std::unordered_set<com_symbol> successor_filter;
  private:
    // variables
    State_vector initial_states;
    std::vector<bool> final;
    std::vector<bool> epsilon;
    std::vector<Symbol> symbol_translation;
    std::vector<std::unordered_map<com_symbol,std::vector<com_state>>> transitions; // a pair of symbol and successor for that symbol
    
    std::unordered_map<Symbol,com_symbol> symbol_map;
    
    compressed_automaton_independence<Symbol> independence_ = compressed_automaton_independence<Symbol>(symbol_translation);
  };
  compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca);
  compressed_automaton<abstraction::psymbol> from_concurrent_automaton(const concurrent_automaton& ca, const compressed_automaton<abstraction::psymbol>& concurrent);

}

#endif // ABSTRACTION_COMPRESSED_AUTOMATON_H
