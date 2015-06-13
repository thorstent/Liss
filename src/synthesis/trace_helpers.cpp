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

#include "trace_helpers.h"

using namespace synthesis;
using namespace std;

namespace synthesis {
lock_symbols locks_to_symbols(const cnf<::synthesis::lock>& locks, const vector<abstraction::psymbol>& trace) {
  lock_symbols result;
  for (const disj<::synthesis::lock>& d : locks) {
    result.emplace_back();
    for (::synthesis::lock l : d) {
      result.back().emplace_back();
      bool started = false;
      for (::synthesis::lock_location loc : l.locations) {
        result.back().back().emplace_back();
        assert (loc.start.instruction_id() <= loc.end.instruction_id());
        for (unsigned i = loc.start.instruction_id(); i <= loc.end.instruction_id(); ++i) {
          if (trace[i]->thread_id()==loc.start.thread_id())
            result.back().back().back().push_back(trace[i]);
        }
      }
    }
  }
  return result;
}

void blow_up_trace(const cfg::program& program, std::vector<abstraction::psymbol>& trace) {
  vector<abstraction::psymbol> previous(program.no_threads());
  for (auto it = trace.begin(); it != trace.end(); ++it) {
    abstraction::psymbol sy = *it;
    abstraction::psymbol prev = previous[sy->loc.thread];
    if (prev) {
      // find the edge
      bool found = false;
      for (const cfg::edge& e : program.minimised_threads()[sy->loc.thread]->get_successors(prev->loc.state)) {
        if (e.to == sy->loc.state) {
          found = true;
          for (state_id_type s : e.in_betweeners) {
            it = trace.insert(it, program.minimised_threads()[sy->loc.thread]->get_state(s).non_action_symbol.get());
            ++it;
          }
          break;
        }
      }
      assert(found);
    }
    previous[sy->loc.thread] = sy;
  }
}

// remove epsilon transitions from the beginning and the end
void trim_list(lock_list& list) {
  for (auto it = list.begin(); it != list.end() && (*it)->operation==abstraction::op_class::epsilon; ) {
    it = list.erase(it);
  }
  // from the end
  while (list.back()->operation == abstraction::op_class::epsilon) {
    list.pop_back();
  }
}

void split_list(const lock_list& list, vector<lock_list>& result) {
  auto master = list.begin();
  auto last = list.begin();
  while (master != list.end()) {
    if ((*master)->is_preemption_point()) {
      if (master != last) {
        result.emplace_back(last, master);
        trim_list(result.back());
        if (result.back().empty()) result.pop_back();
      }
      last = master+1;
    }
    ++master;
  }
  if (master != last) {
    result.emplace_back(last, master);
    trim_list(result.back());
    if (result.back().empty()) result.pop_back();
  }
}

void remove_preemption(lock_symbols& locks)
{
  for (auto dis = locks.begin(); dis != locks.end();) { // disj<std::vector<std::vector<abstraction::psymbol>>>
    for (auto lock = dis->begin(); lock != dis->end(); ) { // std::vector<std::vector<abstraction::psymbol>>
      vector<lock_list> splitted;
      for (auto list = lock->begin(); list != lock->end(); ++list) { // std::vector<abstraction::psymbol>
        split_list(*list, splitted);
      }
      lock->clear();
      lock->insert(lock->end(), splitted.begin(), splitted.end());
      if (lock->size() <= 1) // a lock with only one side makes no sense
        lock = dis->erase(lock);
      else
        ++lock;
    }
    if (dis->empty())
      dis = locks.erase(dis);
    else
      ++dis;
  }
}

}