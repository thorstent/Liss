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

#include "concurrent_state.h"
#include "cfg/automaton.h"

using namespace abstraction;


concurrent_state::concurrent_state(unsigned no_threads, unsigned dnf_size) : threads(new state_id_type[no_threads] {}), 
length(no_threads), locksviolated(dnf_size) 
{
}

concurrent_state::concurrent_state(const concurrent_state& other) : threads(new state_id_type[other.length]), length(other.length), current(other.current), 
conditionals(other.conditionals), locks(other.locks), locksviolated(other.locksviolated), reward(other.reward), conflicts(other.conflicts)
{
  for (thread_id_type i = 0; i<length; ++i) {
    threads[i] = other.threads[i];
  }
}

concurrent_state::concurrent_state(concurrent_state&& other) : length(other.length), current(other.current),
conditionals(std::move(other.conditionals)), locks(std::move(other.locks)), reward(other.reward), locksviolated(std::move(other.locksviolated)), conflicts(move(other.conflicts))
{
  threads = other.threads;
  other.threads = nullptr;
}


concurrent_state& concurrent_state::operator=(const concurrent_state& other)
{
  if (this == &other)      // Same object?
    return *this;
  if (length < other.length) {
    delete[] threads;
    threads = new state_id_type[other.length];
  }
  
  length = other.length;
  current = other.current;
  conditionals = other.conditionals;
  locks = other.locks;
  reward = other.reward;
  
  for (thread_id_type i = 0; i<length; ++i) {
    threads[i] = other.threads[i];
  }
  
  locksviolated = other.locksviolated;
  
  conflicts = other.conflicts;
  
  return *this;
}

concurrent_state& concurrent_state::operator=(concurrent_state&& other)
{
  if (this == &other)      // Same object?
    return *this;
  
  length = other.length;
  current = other.current;
  conditionals = std::move(other.conditionals);
  locks = std::move(other.locks);
  reward = other.reward;
  
  threads = other.threads;
  other.threads = nullptr;
  
  locksviolated = std::move(other.locksviolated);
  
  conflicts = move(other.conflicts);
  
  return *this;
}


concurrent_state::~concurrent_state()
{
  delete[] threads;
}

bool concurrent_state::operator==(const concurrent_state& other) const
{
  if (&other == this) return true;
  if (length!=other.length) return false;
  for (thread_id_type i = 0; i<length; ++i) {
    if (!std::equal_to<state_id_type>()(threads[i],other.threads[i])) return false;
  }
  if (conditionals != other.conditionals) return false;
  if (locks != other.locks) return false;
  if (current != other.current) return false;
  if (locksviolated != other.locksviolated) return false;
  if (conflicts != other.conflicts) return false;
  return true;
}

bool concurrent_state::operator<(const concurrent_state& other) const
{
  if (reward != other.reward)
    return reward < other.reward;
  /*for (unsigned i = 0; i<length; i++) {
    if (operator[](i) != other[i])
      return operator[](i) < other[i];
  }
  /*size_t hash = std::hash<concurrent_state>()(*this);
  size_t hasho = std::hash<concurrent_state>()(other);
  if (hash!=hasho) return hash < hasho;
  assert(false);*/
  return false;
}


std::size_t std::hash< abstraction::concurrent_state >::operator()(const concurrent_state& val) const
{
  std::size_t seed = 0;
  for (thread_id_type i = 0; i<val.length; ++i) {
    Limi::internal::hash_combine(seed, val.threads[i]);
  }
  Limi::internal::hash_combine(seed, val.conditionals);
  Limi::internal::hash_combine(seed, val.locks);
  return seed;
}

void Limi::printer< abstraction::pcstate >::print(const pcstate& state, std::ostream& out) const
{
  out << "(";
  for (thread_id_type i = 0; i<state->length; ++i) {
    out << threads[i].state_printer()(cfg::reward_state((*state)[i]));
    if (i<state->length-1) out << ",";
  }
  out << ")";
  bool first = true;

  for (lock_type i = 0; i<is.no_locks(); ++i) {
    if (state->locks.test(i)) {
      if (first) out << "["; else out << ",";
      first = false;
      out << is.lookup_lock(i);
    }
  }
  if (!first)
    out << "]";
  first = true;
  for (conditional_type i = 0; i<is.no_conditionals(); ++i) {
    if (state->conditionals[i]) {
      if (first) out << "["; else out << ",";
      first = false;
      out << is.lookup_conditional(i);
    }
  }
  if (!first)
    out << "]";
}

std::ostream& abstraction::operator<<(std::ostream &out, const concurrent_state& s) {
  out << '(';
  for (unsigned i = 0; i < s.length; ++i) {
    out << s[i] << ',';
  }
  out << ')' << std::to_string(s.current);
  return out;
}
