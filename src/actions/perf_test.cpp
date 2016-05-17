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

#include "perf_test.h"
#include "abstraction/concurrent_automaton.h"
#include "automata/perf_test.h"
#include "automata/perf_test_old.h"
#include <iostream>
#include <chrono>
#include <unordered_set>

using namespace std;
using namespace std::chrono;
using namespace actions;

double perf_test::run_time(const cfg::program& program, bool concurrent, bool epsilon_removal)
{
  abstraction::concurrent_automaton automaton(program, concurrent, epsilon_removal);
  automata::perf_test<abstraction::pcstate,abstraction::psymbol,abstraction::concurrent_automaton> algo(automaton);
  steady_clock::time_point t1 = steady_clock::now();
  algo.run();
  steady_clock::time_point t2 = steady_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  return time_span.count();
}

double perf_test::run_time_old(const cfg::program& program, bool concurrent, bool epsilon_removal)
{
  abstraction::concurrent_automaton automaton(program, concurrent, epsilon_removal);
  automata::perf_test_old<abstraction::pcstate,abstraction::psymbol,abstraction::concurrent_automaton> algo(automaton);
  steady_clock::time_point t1 = steady_clock::now();
  algo.run();
  steady_clock::time_point t2 = steady_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  return time_span.count();
}

unsigned count_in_thread(const cfg::abstract_cfg& thread) {
  
  unordered_set<abstraction::psymbol> locked1;
  
  std::deque<pair<state_id_type,bool>> frontier;
  frontier.push_back(make_pair(1,false));
  while (!frontier.empty()) {
    pair<state_id_type,bool> p = frontier.front();
    state_id_type next = p.first;
    bool locked = p.second;
    frontier.pop_front();
    const cfg::state& s = thread.get_state(next);
    // check if s is locking
    bool locking = s.action && s.action->variable_name=="l1" && s.action->operation==abstraction::op_class::lock;
    bool unlocking = s.action && s.action->variable_name=="l1" && s.action->operation==abstraction::op_class::unlock;
    assert (!locking || !unlocking);
    assert (!unlocking || locked);
    if (locked && !unlocking && s.action) locked1.insert(s.action.get());
    for (const cfg::edge e : thread.get_successors(next)) {
      if (!e.back_edge) { 
        frontier.push_back(make_pair(e.to, (locked || locking) && !unlocking));
        //const cfg::state& to = thread.get_state(e.to);
      }
    }
  }
  return locked1.size();
}

void perf_test::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  
  unsigned total = 0;
  // count locked by l1 instructions (this is only some hack to get some numbers for the paper)
  for (const auto& t : program.minimised_threads())
    total = total + count_in_thread(*t);
  
  cout << "Locked by l1: " << total << endl;
  /*cout << "Time for sequential with epsilon transitions: ";
  cout << run_time(program, false, false) << "s" << endl;
  
  //if (verbosity==-2) {
  cout << "Time for seqeuntial without epsilon transitions: ";
  cout << run_time(program, false, true) << "s" << endl;
  //}*/
  
  //if (verbosity==-1) {
  /*cout << "Time for concurrent with epsilon transitions: ";
  cout << run_time(program, true, false) << "s" << endl;
  //}

  //if (verbosity==-1) {
  cout << "Time for concurrent with epsilon transitions (old): ";
  cout << run_time_old(program, true, false) << "s" << endl;*/
  //}
  
  /*//if (verbosity==-2) {
  cout << "Time for concurrent without epsilon transitions: ";
  cout << run_time(program, true, true) << "s" << endl;
  //}*/
}
