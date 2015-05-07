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

#ifndef ABSTRACTION_CSYMBOL_H
#define ABSTRACTION_CSYMBOL_H

#include "symbol.h"
#include <Limi/internal/hash.h>

namespace abstraction {
  
  struct csymbol {
    psymbol symbol;
    uint8_t thread_id;
    csymbol(psymbol symbol, uint8_t thread_id) : symbol(symbol), thread_id(thread_id) {}
    csymbol(){}
    
    bool operator==(const csymbol &other) const {
      return thread_id==other.thread_id && *symbol == *other.symbol;
    }
    
    bool operator<(const csymbol &other) const {
      return symbol < other.symbol;
    }
    
    bool operator>(const csymbol &other) const {
      return symbol > other.symbol;
    }
  };
  
  typedef csymbol pcsymbol;
  
  inline std::ostream& operator<< (std::ostream &out, const abstraction::pcsymbol &val) {
    out << std::to_string(val.thread_id);
    out << *val.symbol;
    return out;
  }
}

namespace Limi {
  template<> struct independence<abstraction::pcsymbol> {
    inline bool operator()(const abstraction::pcsymbol& a, const abstraction::pcsymbol& b) const {
      return a.thread_id!=b.thread_id && 
      (
        a.symbol->variable != b.symbol->variable ||
        a.symbol->operation==abstraction::op_class::read && b.symbol->operation==abstraction::op_class::read ||
        a.symbol->is_epsilon() || b.symbol->is_epsilon() ||
        a.symbol->operation==abstraction::op_class::tag || b.symbol->operation==abstraction::op_class::tag
      );
    }
  };
}

namespace std {  
  template<> struct hash<abstraction::pcsymbol> {
    inline size_t operator()(const abstraction::pcsymbol& val) const {
      size_t seed = std::hash< abstraction::psymbol >()(val.symbol);
      Limi::internal::hash_combine(seed, val.thread_id);
      return seed;
    }
  };
}

namespace Limi {
  template<> struct printer<abstraction::pcsymbol> : public printer_base<abstraction::pcsymbol> {
    virtual void print(const abstraction::pcsymbol& symbol, std::ostream& out) const {
      out << std::to_string(symbol.thread_id);
      out << symbol.symbol;
    }
  };
}

#endif // ABSTRACTION_CSYMBOL_H