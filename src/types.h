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

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>
#include <ostream>

typedef int16_t state_id_type; // a negative number means we already passed that state, a positive number means we are before that state
typedef int16_t reward_t;
const state_id_type no_state = 0;
const unsigned max_states = INT16_MAX;

const unsigned max_variables = UINT16_MAX;
const unsigned max_locks = 64;
const unsigned max_conditionals = 64; 

typedef uint16_t variable_type;
typedef uint8_t lock_type;
typedef uint8_t conditional_type;


typedef int8_t thread_id_type;
const unsigned max_threads = INT8_MAX;
const thread_id_type no_thread = -1;

/**
 * DNFs and CNFs
 */

template <typename atom>
class conj : public std::vector<atom> {};
template <typename atom>
std::ostream& operator<<(std::ostream& out, const conj<atom>& c) {
  if (c.size()==0)
    out << "true";
  for (unsigned i = 0; i < c.size(); ++i) {
    out << c[i];
    if (i < c.size() - 1) out << " /\\ ";
  }
  return out;
}

template <typename atom>
class disj : public std::vector<atom> {};
template <typename atom>
std::ostream& operator<<(std::ostream& out, const disj<atom>& c) {
  if (c.size()==0)
    out << "false";
  for (unsigned i = 0; i < c.size(); ++i) {
    out << c[i];
    if (i < c.size() - 1) out << " \\/ ";
  }
  return out;
}

template <typename atom>
class dnf : public std::vector<conj<atom>> {};
template <typename atom>
std::ostream& operator<<(std::ostream& out, const dnf<atom>& c) {
  if (c.size()==0)
    out << "false";
  for (unsigned i = 0; i < c.size(); ++i) {
    out << c[i];
    if (i < c.size() - 1) out << " \\/ ";
    out << std::endl;
  }
  return out;
}

template <typename atom>
class cnf : public std::vector<disj<atom>> {};
template <typename atom>
std::ostream& operator<<(std::ostream& out, const cnf<atom>& c) {
  if (c.size()==0)
    out << "true";
  for (unsigned i = 0; i < c.size(); ++i) {
    out << c[i];
    if (i < c.size() - 1) out << " /\\ ";
    out << std::endl;
  }
  return out;
}

#endif 