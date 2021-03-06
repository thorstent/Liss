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

#include "synchronisation.h"
#include "options.h"
#include <iostream>
#include <algorithm>

using namespace synthesis;
using namespace std;

synchronisation::synchronisation(const cfg::program& program):
 symbol_printer(Limi::printer<abstraction::psymbol>()) {
  
}

void synchronisation::generate_sync(const cnf_constr& cnf_weak, cnf< lock >& locks)
{
  for (const disj_constr& d : cnf_weak) {
    locks.emplace_back();
    if (!find_lock(d, locks.back(), false) && !find_lock(d, locks.back(), true)) {
      cerr << "Inferrence failed for ";
      cerr << d;
      cerr << endl;
    }
  }
  
  //merge_locks(locks);
  // we do lock merging later
  //merge_locks_multithread(locks);
  
  if (verbosity >= 1) {
    debug << "Locks inferred: " << endl;
    debug << locks;
    
  }
  
}

unsigned lock_counter = 0;
bool synchronisation::find_lock(const disj_constr& disjunct, std::vector<lock>& locks, bool allow_only_weak)
{
  bool found = false;
  if (disjunct.size()>=2) {
    // find all combinations
    for (auto a = disjunct.begin(); a!=disjunct.end(); a++)
    {
      auto b = a;
      for (b++; b!=disjunct.end(); b++) {
        if (!a->from_wait_notify || !b->from_wait_notify || allow_only_weak) {
          // check if this is a lock
          const location* loc1a = &a->before;
          const location* loc1b = &b->after;
          const location* loc2b = &a->after;
          const location* loc2a = &b->before;
          if (loc1a->thread_id() == loc1b->thread_id() && loc2a->thread_id()==loc2b->thread_id() && 
            ((loc1a->instruction_id() >= loc1b->instruction_id() && loc2a->instruction_id() >= loc2b->instruction_id()) ||
            (loc1a->instruction_id() >= loc1b->instruction_id() && loc2a->instruction_id() <= loc2b->instruction_id()) ||
            (loc1a->instruction_id() <= loc1b->instruction_id() && loc2a->instruction_id() >= loc2b->instruction_id()) ||
            (loc1a->instruction_id() == loc1b->instruction_id() && loc2a->instruction_id() <= loc2b->instruction_id()) ||
            (loc1a->instruction_id() <= loc1b->instruction_id() && loc2a->instruction_id() == loc2b->instruction_id()))
          ) {
            if (loc1a->instruction_id() > loc1b->instruction_id()) {
              auto temp = loc1a; loc1a = loc1b; loc1b = temp;
            }
            if (loc2a->instruction_id() > loc2b->instruction_id()) {
              auto temp = loc2a; loc2a = loc2b; loc2b = temp;
            }
            lock l("l" + std::to_string(++lock_counter));
            // order the locations by thread number
            if (loc1a->thread_id() > loc2a->thread_id()) {
              l.locations.push_back(lock_location(*loc1a,*loc1b));
              l.locations.push_back(lock_location(*loc2a,*loc2b));
            } else {
              l.locations.push_back(lock_location(*loc2a,*loc2b));
              l.locations.push_back(lock_location(*loc1a,*loc1b));
            }
            locks.push_back(l);
            found = true;
          }
        }
      }
    }
  }
  return found;
}

void synchronisation::merge_locks(std::list<lock>& locks)
{
  /*
   * We merge if locks a and b are either 
   * - contained in each other
   * - would cause a deadlock
   * - are directly adjacent
   */
  for (auto i = locks.begin(); i!=locks.end(); i++) {
    assert (i->locations.size() == 2);
    // check if another lock should be merged with this one
    lock_location& locp1a = i->locations.front();
    lock_location& locp1b = i->locations.back();
    auto j = i;
    for (j++; j!=locks.end(); ) {
      assert (j->locations.size() == 2);
      lock_location& locp2a = j->locations.front();
      lock_location& locp2b = j->locations.back();
      bool found = false;
      // make sure they talk about the same threads
      if (locp1a.start.thread_id() == locp2a.end.thread_id() && locp1b.start.thread_id() == locp2b.end.thread_id()) {
        // check if one is contained in the other
        if (check_contained(locp1a, locp2a) && check_contained(locp1b, locp2b)) {
          merge_overlap(locp1a, locp2a);
          merge_overlap(locp1b, locp2b);
          found = true;
        } else if (check_overlap(locp1a, locp2a) && check_overlap(locp1b, locp2b) &&
          ((locp1a.start.instruction_id() > locp2a.start.instruction_id()) != (locp1b.start.instruction_id() > locp2b.start.instruction_id()) )
        ) {
          // check for deadlock (they overlap and one is aquired before the other
          merge_overlap(locp1a, locp2a);
          merge_overlap(locp1b, locp2b);
          found = true;
        }
      }
      if (found) {
        // add locations that were not previously merged (these were already deleted in locks[j].locations
        j = locks.erase(j);
      }
      else
        j++;
    }
  }
}

template <class content>
bool list_contains(const std::list<content>& v, const content& element) {
  for(content c: v)
    if (c==element) return true;
    return false;
}

/*
void synchronisation::merge_locks_multithread(std::list<lock>& locks)
{
  *
   * We merge if a set of locks talks about the same set of locations. Initially every lock has two locations
   *
  for (auto i = locks.begin(); i!=locks.end(); i++) {
    // check if another lock should be merged with this one
    auto j = i;
    for (j++; j!=locks.end(); j++) {
      // the thing we want to merge with should have only two locations
      assert (j->locations.size() == 2);
      location_pair locp2a = j->locations.front();
      location_pair locp2b = j->locations.back();
      // check if one of the locations in the lock j is in the list of locations of i
      location_pair* present = nullptr; // the location thas 
      location_pair* other = nullptr;
      if (list_contains(i->locations, locp2a)) {
        present = &locp2a;
        other = &locp2b;
      }
      if (list_contains(i->locations, locp2b)) {
        present = &locp2b;
        other = &locp2a;
      }
      if (present != nullptr) {
        vector<list<lock>::iterator> to_delete;
        to_delete.push_back(j);
        // this is a merge candidate if we find the other corresponding items
        // the other items are those corresponding to the non-present location
        for (auto loc : i->locations) {
          if (loc != *present) { // this lock we already found
            auto h = i;
            for (h++; h!=locks.end(); h++) {
              if (h!=j) { // we don't want to test the same lock again
                if ((h->locations.front() == *other && h->locations.back() == loc) || (h->locations.back() == *other && h->locations.front() == loc)) {
                  to_delete.push_back(h);
                  break;
                }
              }
            }
          }
        }
        
        if (to_delete.size() == i->locations.size()) {
          i->locations.push_back(*other);
          for (auto it : to_delete)
            locks.erase(it);
          break;
        }
      }
    }
  }
}*/

inline bool synchronisation::check_overlap(const lock_location& locs1, const lock_location& locs2)
{
  if (locs1.start.thread_id() == locs2.start.thread_id()) {
    if ((locs1.start.instruction_id() <= locs2.start.instruction_id() && locs2.start.instruction_id() <= locs1.end.instruction_id()) ||
      (locs1.start.instruction_id() <= locs2.start.instruction_id() && locs1.start.instruction_id() <= locs2.end.instruction_id())
    )
      return true;
  }
  return false;
}

inline bool synchronisation::check_contained(const lock_location& locs1, const lock_location& locs2)
{
  if (locs1.start.thread_id() == locs2.start.thread_id()) {
    if ((locs1.start.instruction_id() <= locs2.start.instruction_id() && locs2.end.instruction_id() <= locs1.end.instruction_id()) ||
      (locs2.start.instruction_id() <= locs1.start.instruction_id() && locs1.end.instruction_id() <= locs2.end.instruction_id())
    )
      return true;
  }
  return false;
}

inline void synchronisation::merge_overlap(lock_location& locs1, const lock_location& locs2)
{
  locs1.start=locs1.start.instruction_id() <= locs2.start.instruction_id() ? locs1.start : locs2.start;
  locs1.end=locs1.end.instruction_id() >= locs2.end.instruction_id() ? locs1.end : locs2.end;
}