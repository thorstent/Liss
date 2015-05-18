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

#include "abstract_cfg.h"
#include <cassert>
#include <clang/AST/Decl.h>
#include <deque>
#include <stdexcept>
#include <list>
#include <Limi/internal/helpers.h>
#include <set>

namespace std{
  template <> struct hash <std::unordered_multiset<const abstraction::symbol*>> {
    size_t operator()(const std::unordered_multiset<const abstraction::symbol*>& to_hash) const {
      // this is a hack and will result in all hashed to collide, but that should be ok as long as there are only few elements in the set
      return 0;
    }
  };
}

using namespace cfg;
using namespace std;

std::ostream& cfg::operator<<(std::ostream& os, const state& s) {
  if (s.action)
    os << s.action;
  else if (s.final) {
    os << "Exit";
  } else {
    os << s.name;
  }
  os << "(" << s.distance << ")";
  return os;
}

ostream& cfg::operator<<(ostream& os, const edge& e)
{
  if (e.tag) {
    os << e.tag;
  }
  if (e.cost != 0) {
    os << "(" << e.cost << ")";
  }
  return os;
}


abstract_cfg::abstract_cfg(const clang::FunctionDecl* fd, thread_id_type thread_id)
: declaration(fd), thread_id(thread_id)
{
  add_state("No state");
  clang::DeclarationName dn = declaration->getNameInfo().getName();
  name = dn.getAsString();
}

state_id abstract_cfg::add_state(const abstraction::symbol& symbol)
{
  if (states.size()>=max_states) throw range_error("Maximum number of states reached");
  states.emplace_back(states.size(), symbol);
  edges.emplace_back();
  states.back().action->thread_id = thread_id;
  states.back().action->state = states.size() - 1;
  return states.size() - 1;
}

state_id abstract_cfg::add_state(const string& name)
{
  if (states.size()>=max_states) throw range_error("Maximum number of states reached");
  states.emplace_back(states.size());
  edges.emplace_back();
  states[states.size()-1].name = name;
  return states.size() - 1;
}


void abstract_cfg::mark_final(state_id state)
{
  states[state].final = true;
}

void abstract_cfg::tag_edge(state_id state, uint8_t edge)
{
  edges[state][edge].tag = make_shared<abstraction::symbol>(state, edge);
}


void abstract_cfg::add_edge(state_id from, state_id to, bool back_edge, bool auto_tag, reward_t cost)
{
  assert(from>=0 && to>=0);
  if (auto_tag && edges[from].size() == 1) {
    // automatic tagging is too expensive. We now have the add_tag function
    if (!states[from].non_det)
      tag_edge(from, 0);
  }
  edges[from].emplace_back(to, back_edge, cost);
  if (auto_tag && edges[from].size() > 1) {
    if (!states[from].non_det)
      tag_edge(from, edges[from].size()-1);
  }
}

state_id abstract_cfg::add_dummy_state()
{
  if (states.size() == 1)
    return add_state("Init");
  return add_state("Dummy");
}

struct successors_pair_less {
  bool operator()(std::pair<state_id,bool> p1, std::pair<state_id,bool> p2) const {
    return p1.first < p2.first;
  }
};

void abstract_cfg::minimise()
{
  std::unordered_set<state_id> seen;
  
  // add the current set of initial states to the frontier
  std::deque<pair<state_id,vector<state_id>>> frontier2;
  frontier2.push_back(make_pair(1,vector<state_id>()));
  
  // check if the successor of every state is good, otherwise replace with successor of successor
  while (!frontier2.empty()) {
    pair<state_id,vector<state_id>> nextp = frontier2.front();
    state_id next = nextp.first;
    assert(states[next].action || states[next].final || next == 1);
    vector<state_id> parents = nextp.second;
    frontier2.pop_front();
    if (seen.find(next) == seen.end()) {
      parents.push_back(next);
      seen.insert(next);
      
      set<std::pair<state_id,bool>, successors_pair_less> successors; // set of successors
      
      for (unsigned i = 0; i<edges[next].size();++i) {
        edge edge_to = edges[next][i];
        state& succ = states[edge_to.to];
        if (!succ.action && !succ.final) {
          // remove this successor
          for (const edge& edge2 : get_successors(edge_to.to)) {
            edge new_edge(edge2);
            if (edge_to.tag)
              new_edge.tag = edge_to.tag;
            edges[next].push_back(std::move(new_edge));
          }
        } else {
          auto newp = make_pair(edge_to.to, edge_to.tag!=nullptr);
          successors.insert(make_pair(edge_to.to, edge_to.tag!=nullptr));
        }
      }
      edges[next].clear();
      // re-add the edges
      for (std::pair<state_id,bool> succ : successors) {
        frontier2.push_back(make_pair(succ.first,parents));
        reward_t cost = parents.end() - find(parents.begin(), parents.end(), succ.first);
        bool back_edge = cost != 0;
        add_edge(next, succ.first, back_edge, false, cost);
        if (succ.second) {
          tag_edge(next, edges[next].size()-1);
        }
      }
    }
  }
  
  calc_distance(1);
}

unsigned int abstract_cfg::calc_distance(state_id state)
{
  if (states[state].distance!=0) return states[state].distance;
  unsigned min = 999;
  if (states[state].final) min = 0;
  else {
    for (edge& e : edges[state]) {
      unsigned other;
      if (e.back_edge) {
        other = states[e.to].distance;
      } else {
        other = calc_distance(e.to);
      }
      min = std::min(min, other+1);
    }
  }
  states[state].distance = min+1;
  return min+1;
}

void abstract_cfg::compact()
{
  vector<bool> active(states.size(), false);
  // get a list of active states
  std::deque<state_id> frontier;
  frontier.push_back(1);
  
  while (!frontier.empty()) {
    state_id next = frontier.front();
    frontier.pop_front();
    active[next] = true;
    for (edge& e : edges[next]) {
      if (!e.back_edge) {
        frontier.push_back(e.to);
      }
    }
  }
  
  vector<state_id> mapping(states.size(), no_state);
 
  //filter out the ones not active
  state_id last_active = 0;
  for (unsigned i = 1; i < states.size(); ++i) {
    if (!active[i]) {
      // find next one and move it ahead
      unsigned j = i;
      for (; j < states.size(); ++j) {
        if (active[j]) {
          // move this
          states[i] = states[j];
          edges[i] = std::move(edges[j]);
          active[j] = false;
          mapping[j] = i;
          states[i].id = i;
          if(states[i].action)
            states[i].action->state = i;
          break;
        }
      }
      if (j == states.size()) break;
    }
    last_active = i;
  }
  
  // truncate the vectors
  states.erase(states.begin()+last_active+1, states.end());
  edges.erase(edges.begin()+last_active+1, edges.end());
  
  // now update the mappings
  for (unsigned i = 1; i < states.size(); ++i) {
    for (edge& e : edges[i]) {
      if (mapping[e.to]!=no_state) e.to = mapping[e.to];
      if (e.tag) e.tag->state = i;
    }
  }
}

const unordered_set< state_id > abstract_cfg::get_forward_successors(state_id from) const
{
  unordered_set< state_id > res;
  for (const edge& e : edges[from]) {
    if (!e.back_edge) res.insert(e.to);
  }
  return res;
}

