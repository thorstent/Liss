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
#include <stdexcept>

typedef int16_t state_id_type; // a negative number means we already passed that state, a positive number means we are before that state
typedef int16_t reward_t;
constexpr state_id_type no_state = 0;
constexpr unsigned max_states = INT16_MAX;

constexpr unsigned max_variables = UINT16_MAX;
constexpr unsigned max_locks = 64;
constexpr unsigned max_conditionals = 64; 

typedef uint16_t variable_type;
typedef uint8_t lock_type;
typedef uint8_t conditional_type;


typedef int8_t thread_id_type;
constexpr unsigned max_threads = INT8_MAX;
constexpr thread_id_type no_thread = -1;

/**
 * DNFs and CNFs
 */
template <typename atom> class disj;

template <typename atom>
class conj : public std::vector<atom> {
public:
  disj<atom> operator!() const {
    disj<atom> dis;
    for(const atom& ca : *this) {
      dis.push_back(!ca);
    }
    return dis;
  }
};
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
class disj : public std::vector<atom> {
public:
  conj<atom> operator!() const {
    conj<atom> con;
    for(const atom& ca : *this) {
      con.push_back(!ca);
    }
    return con;
  }
};
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

template <typename atom> class cnf;

template <typename atom>
class dnf : public std::vector<conj<atom>> {
public:
  cnf<atom> operator!() const {
    cnf<atom> cnf;
    for(const conj<atom>& ca : *this) {
      cnf.push_back(!ca);
    }
    return cnf;
  }
};
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
class cnf : public std::vector<disj<atom>> {
public:
  dnf<atom> operator!() const {
    dnf<atom> dnf;
    for(const disj<atom>& ca : *this) {
      dnf.push_back(!ca);
    }
    return dnf;
  }
};
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

template<typename integer>
struct tagged_int {
  inline void set() { if (invalid()) throw std::logic_error("Value is invalid"); data = data | (mask); }
  inline void clear() { if (invalid()) throw std::logic_error("Value is invalid"); data = data & ~(mask); }
  inline void set(bool value) { if (invalid()) throw std::logic_error("Value is invalid"); if (value) set(); else clear(); }
  inline bool test() const { if (invalid()) throw std::logic_error("Value is invalid"); return data & (mask); }
  inline bool valid() const { return data != invalid_;}
  inline bool invalid() const { return data == invalid_; }
  inline integer payload() const { 
    if (invalid()) throw std::logic_error("Value is invalid");
    return data & ~(mask); 
  }
  inline void payload(integer value) { 
    if (value > max_payload) throw std::range_error("To high value for payload");
    data = (data & (mask)) | value;
  }
  inline tagged_int() : data(invalid) { }
  inline explicit tagged_int(integer i) { payload(i); }
  inline operator integer() { if (invalid()) throw std::logic_error("Value is invalid"); return payload(); } const
  inline bool operator==(const tagged_int& other) const { return data == other.data; }
  inline bool operator!=(const tagged_int& other) const { return data != other.data; }
  inline bool operator<(const tagged_int& other) const { return data < other.data; }
private:
  integer data = 0;
  constexpr static integer mask = ((integer)(1U)) << (sizeof(integer)*4-1);
  constexpr static integer invalid_ = -1;
public:
  constexpr static integer max_payload = (~mask)-1;
};

using tagged_int16 = tagged_int<uint16_t>;


#endif 