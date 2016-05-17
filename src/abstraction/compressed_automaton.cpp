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

#include "compressed_automaton.h"

using namespace abstraction;
using namespace std;

#include <unordered_map>
#include <queue>
#include "options.h"

compressed_automaton< psymbol > abstraction::from_concurrent_automaton(const concurrent_automaton& ca)
{
  return from_concurrent_automaton(ca, compressed_automaton<abstraction::psymbol>());
}

compressed_automaton< psymbol > abstraction::from_concurrent_automaton(const concurrent_automaton& ca, const compressed_automaton<abstraction::psymbol>& concurrent)
{
  compressed_automaton<psymbol> result;
  bool nondet = false;
  result.symbol_translation = concurrent.symbol_translation;
  result.epsilon = concurrent.epsilon;
  
  // internal maps
  std::unordered_map<pcstate, com_state> states;
  std::unordered_map<psymbol, com_symbol>& symbols = result.symbol_map;
  for (unsigned i = 0; i < result.symbol_translation.size(); ++i) {
    symbols.insert(make_pair(result.symbol_translation[i],i));
  }
  
  std::unordered_set<pcstate> seen;
  std::priority_queue<pcstate> frontier;
  
  for (const pcstate& s : ca.initial_states()) {
    com_state sid = states.size();
    result.initial_states.push_back(sid);
    frontier.push(s);
    states.insert(make_pair(s, sid));
    result.final.push_back(false);
    result.transitions.emplace_back();
    seen.insert(s);
  }
  
  while (!frontier.empty()) {
    pcstate s = frontier.top();
    frontier.pop();
    auto sidi = states.find(s);
    assert(sidi != states.end());
    com_state sid = sidi->second;
    if (ca.is_final_state(s)) { 
      result.final[sid] = true;
    }
    for (const psymbol& sy : ca.next_symbols(s)) {
      auto syidi = symbols.insert(make_pair(sy, symbols.size()));
      com_symbol syid = syidi.first->second;
      if (syidi.second) {
        result.symbol_translation.push_back(sy);
        result.epsilon.push_back(false);
      }
      if (ca.is_epsilon(sy)) {
        result.epsilon[syid] = true;
      }
      vector<com_state> successors;
      for (const pcstate& next : ca.successors(s, sy)) {
        auto nextidi = states.insert(make_pair(next, states.size()));
        if (nextidi.second) {
          result.final.push_back(false);
          result.transitions.emplace_back();
        }
        com_state nextid = nextidi.first->second;
        successors.push_back(nextid);
        if (seen.insert(next).second) {
          frontier.push(next);
        }
      }
      if (successors.size()>0)
        result.transitions[sid].insert(make_pair(syid,successors));
      nondet = nondet || successors.size() > 1;
    }
  }
  
  assert(result.symbol_translation.size() == symbols.size());
  assert(result.transitions.size() == states.size());
  assert(result.final.size() == states.size());
  assert(result.epsilon.size() == symbols.size());
  
  if (verbosity>=2) {
    debug << "Number of states: " << states.size() << endl;
    debug << "Number of symbols: " << symbols.size() << endl;
    if (nondet) debug << "Non-deterministic automaton" << endl;
    else debug << "Deterministic automaton" << endl;
  }
  
  return result;
}

