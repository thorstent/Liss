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

#ifndef ABSTRACTION_SYMBOL_H
#define ABSTRACTION_SYMBOL_H

#include <string>
#include <sstream>
#include <list>
#include <cassert>
#include "identifier_store.h"
#include <Limi/generics.h>
#include <Limi/internal/hash.h>

namespace clang {
class Stmt;
class SourceManager;
class FileEntry;
}

//TODO: remove this
typedef std::pair<clang::Stmt*, clang::Stmt*> stmt_loc;
typedef std::list<stmt_loc> call_stack;

namespace abstraction {

enum class op_class { 
  read,
  write,
  lock,
  unlock,
  wait,
  wait_not,
  notify,
  reset,
  wait_reset,
  yield,
  epsilon,
  tag
};

enum class variable_class {
  none,
  var,
  lock,
  conditional,
};

std::ostream& operator<< (std::ostream& out, const op_class& op);

// TODO: clean this class up
struct symbol
{
  op_class operation;
  variable_type variable = 0;
  call_stack cstack;
  
  std::string variable_name;
  const clang::FileEntry* fileentry = nullptr;
  unsigned line_no = 0;
  
  state_id tag_state = no_state;
  uint8_t tag_branch = 0;
  
  mutable uint16_t origin;
  /**
   * @brief True if this is an assume.
   * 
   * Assumes are like waits, but they are ignored during deadlock analysis
   * 
   */
  bool assume = false;
  
  /**
   * @brief The symbol has been synthesised in one of the previous iterations
   * 
   */
  bool synthesised = false;
  
  inline bool is_epsilon() const { return operation!=op_class::read && operation!=op_class::write && operation!=op_class::tag; }
  inline bool is_real_epsilon() const { return operation==op_class::epsilon; }
  
  symbol() : operation(op_class::epsilon) {}
  symbol(op_class operation, call_stack cstack, std::string variable_name, variable_type variable, identifier_store& is, const clang::Stmt* stmt);
  symbol(state_id state, uint8_t branch);
  inline const clang::Stmt* instr_id() const { return stmt; }
  symbol(const symbol& sym) = default;
  
  bool operator==(const symbol &other) const {
    if (tag_state == no_state) {
      if (other.tag_state!=no_state) return false;
      return (operation == other.operation && variable == other.variable && instr_id() == other.instr_id());
    } else {
      if (other.tag_state==no_state) return false;
      return (tag_state == other.tag_state) && (tag_branch == other.tag_branch);
    }
  }
  
  friend std::ostream& operator<< (std::ostream &out, const abstraction::symbol &val);
private:
  const clang::Stmt* stmt = nullptr;
};

typedef const symbol*  psymbol;

inline std::ostream& operator<< (std::ostream &out, const abstraction::psymbol &val) {
  out << *val;
  return out;
}
}

namespace std {
  template<> struct hash<abstraction::op_class> {
    inline size_t operator()(const abstraction::op_class& val) const {
      return hash<int>()(static_cast<int>(val));
    }
  };
  template<> struct hash<abstraction::symbol> {
    size_t operator()(const abstraction::symbol& val) const {
      std::size_t seed = 0;
      if (val.tag_state == no_state) {
        Limi::internal::hash_combine(seed, val.operation);
        Limi::internal::hash_combine(seed, val.variable);
        Limi::internal::hash_combine(seed, val.instr_id());
      } else {
        Limi::internal::hash_combine(seed, val.tag_state);
        Limi::internal::hash_combine(seed, val.tag_branch);
      }
      return seed;
    }
  };
  template<> struct hash<abstraction::psymbol> {
    inline size_t operator()(const abstraction::psymbol& val) const {
      return hash<abstraction::symbol>()(*val);
    }
  };
  template<> struct equal_to<abstraction::psymbol> {
    inline bool operator()(const abstraction::psymbol& a, const abstraction::psymbol& b) const {
      if (a && b)
        return equal_to<abstraction::symbol>()(*a,*b);
      else return a == b;
    }
  };
}

namespace Limi {
  template<> struct printer<abstraction::psymbol> : public printer_base<abstraction::psymbol> {
    inline virtual void print(const abstraction::psymbol& symbol, std::ostream& out) const {
      out << symbol;
    }
  };
}

#endif // ABSTRACTION_SYMBOL_H
