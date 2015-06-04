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

#include "trace.h"
#include "options.h"
#include <sstream>
#include <z3++.h>
#include <unordered_map>
#include <vector>

using namespace synthesis;
using namespace std;

concurrent_trace synthesis::make_trace(z3::context& ctx, const cfg::program& program, const vector< abstraction::psymbol > trace)
{
  unordered_map<abstraction::psymbol, unsigned> iteration_counter;
  concurrent_trace result(program.no_threads());
  unsigned counter = 0;
  vector<unsigned> iter(program.no_threads(),0);
  for (const abstraction::psymbol& symbol : trace) {
    string name = "loc";
    
    if (iteration_counter.find(symbol)==iteration_counter.end())
      iteration_counter[symbol] = 1;
    else
      iteration_counter[symbol]++;
    iter[symbol->thread_id()] = max(iter[symbol->thread_id()],iteration_counter[symbol]);
    
    // create location
    if (verbosity >= 2) {
      stringstream ss;
      ss << symbol;
      if (iter[symbol->thread_id()]>1)
        ss << "(" << iter[symbol->thread_id()] << ")";
      name = ss.str();
    }
    location loc(ctx.fresh_constant(name.c_str(), ctx.int_sort()), name, symbol, counter++);
    loc.iteration = iter[symbol->thread_id()];
    result.threads[symbol->thread_id()].push_back(loc);
  }
  return result;
}
