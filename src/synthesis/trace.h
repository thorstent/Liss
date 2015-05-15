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

#ifndef SYNTHESIS_TRACE_H
#define SYNTHESIS_TRACE_H

#include "abstraction/symbol.h"
#include "cfg/program.h"
#include "location.h"
#include <vector>
#include <list>

namespace synthesis {
  struct concurrent_trace {
    concurrent_trace(unsigned no_threads) : threads(no_threads) {}
    std::vector<std::list<location>> threads;
  };
  
  concurrent_trace make_trace(z3::context& ctx, const cfg::program& program, const std::list< abstraction::psymbol > trace);
}

#endif