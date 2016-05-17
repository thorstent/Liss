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

#ifndef ABSTRACTION_CONCURRENT_STATE_H
#define ABSTRACTION_CONCURRENT_STATE_H

#include <Limi/internal/hash.h>
#include <Limi/internal/helpers.h>
#include <functional>
#include <bitset>
#include <set>
#include <memory>
#include <ostream>
#include <boost/dynamic_bitset.hpp>
#include "identifier_store.h"
#include "cfg/automaton.h"

namespace abstraction {
  
  using conflict_t = tagged_int16;
  
  struct concurrent_state {
    concurrent_state(unsigned no_threads, unsigned dnf_size);
    ~concurrent_state();
    
    concurrent_state(const concurrent_state& other);
    concurrent_state & operator= (const concurrent_state & other);
    concurrent_state(concurrent_state&& other);
    concurrent_state & operator= (concurrent_state && other);
    
    state_id_type* threads;
    thread_id_type length;
    thread_id_type current = no_thread;
    reward_t reward = 0;
    
    state_id_type& operator[] (const int i) {return threads[i];};
    const state_id_type& operator[] (const int i) const {return threads[i];};
    
    std::bitset<max_conditionals> conditionals; // bit is set if conditional is notified
    std::bitset<max_locks> locks; // bit is set if lock is taken
    
    // locks that have been violated (refering to synthesised locks that are inserted on the fly)
    boost::dynamic_bitset<unsigned> locksviolated;
    
    // see concurrent automata class
    std::set<conflict_t> conflicts;
    
    bool operator==(const concurrent_state &other) const;
    bool operator<(const concurrent_state &other) const;
  };
  
  typedef std::shared_ptr<concurrent_state> pcstate;
  
  std::ostream& operator<<(std::ostream &out, const concurrent_state& s);
  inline std::ostream& operator<<(std::ostream &out, const pcstate& s) {
    out << *s;
    return out;
  }

}

namespace std {
  template<> struct hash<abstraction::concurrent_state> {
    size_t operator()(const abstraction::concurrent_state& val) const;
  };
  template<> struct hash<abstraction::pcstate> {
    inline size_t operator()(const abstraction::pcstate& val) const {
      return hash<abstraction::concurrent_state>()(*val);
    }
  };
  
  template<> struct equal_to<abstraction::pcstate> {
    inline bool operator()(const abstraction::pcstate& a, const abstraction::pcstate& b) const {
      return equal_to<abstraction::concurrent_state>()(*a,*b);
    }
  };
  template<> struct less<abstraction::pcstate> {
    inline bool operator()(const abstraction::pcstate& a, const abstraction::pcstate& b) const {
      return less<abstraction::concurrent_state>()(*a,*b);
    }
  };
}



namespace Limi {
  
  template<> struct printer<abstraction::pcstate> : printer_base<abstraction::pcstate> {
    printer(const std::vector<cfg::automaton>& threads, const abstraction::identifier_store& identifier_store) : threads(threads), is(identifier_store) {}
    virtual void print(const abstraction::pcstate& state, std::ostream& out) const override;
  private:  
    const std::vector<cfg::automaton>& threads;
    const abstraction::identifier_store& is;
  };

}
#endif // ABSTRACTION_CONCURRENT_STATE_H
