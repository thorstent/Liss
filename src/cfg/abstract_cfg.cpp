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

using namespace cfg;
using namespace std;

state::state(state_id_type id, const abstraction::symbol& action) : action(std::make_shared<abstraction::symbol>(action)) { assert(this->action->loc.state == id); }
state::state(thread_id_type thread_id, state_id_type id) : non_action_symbol(std::make_shared<abstraction::symbol>(thread_id, id)) {}

void state::name(std::string newn) { 
  if (non_action_symbol) { 
    std::shared_ptr<abstraction::symbol> newp = make_shared<abstraction::symbol>(*non_action_symbol);
    newp->variable_name = newn;
    non_action_symbol = newp;
  }
}

void state::id(state_id_type new_id) {
  if (action) { 
    std::shared_ptr<abstraction::symbol> newp = make_shared<abstraction::symbol>(*action);
    newp->loc.state = new_id;
    action = newp;
  }
  if (non_action_symbol) { 
    std::shared_ptr<abstraction::symbol> newp = make_shared<abstraction::symbol>(*non_action_symbol);
    newp->loc.state = new_id;
    non_action_symbol = newp;
  }
}

void edge::id(state_id_type new_id) {
  if (tag) { 
    std::shared_ptr<abstraction::symbol> newp = make_shared<abstraction::symbol>(*tag);
    newp->loc.state = new_id;
    tag = newp;
  }
}

std::ostream& cfg::operator<<(std::ostream& os, const state& s) {
  if (s.action)
    os << s.action;
  else if (s.final) {
    os << "Exit";
  } else {
    assert (s.non_action_symbol);
    os << to_string(s.non_action_symbol->thread_id()) << "-";
    os << s.non_action_symbol->variable_name;
  }
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

bool edge::operator==(const edge& other) const {
  return to==other.to;
}

bool edge::operator<(const edge& other) const {
  return to<other.to;
}

abstract_cfg::abstract_cfg(const clang::FunctionDecl* fd, thread_id_type thread_id)
: declaration(fd), thread_id(thread_id)
{
  add_state("No state");
  clang::DeclarationName dn = declaration->getNameInfo().getName();
  name = dn.getAsString();
}


state_id_type abstract_cfg::add_state(abstraction::symbol symbol)
{
  if (states.size()>=max_states) throw range_error("Maximum number of states reached");
  symbol.loc.thread = thread_id;
  symbol.loc.state = states.size() - 1;
  states.emplace_back(states.size(), symbol);
  edges.emplace_back();
  return states.size() - 1;
}

state_id_type abstract_cfg::add_state(const string& name)
{
  if (states.size()>=max_states) throw range_error("Maximum number of states reached");
  states.emplace_back(thread_id, states.size());
  edges.emplace_back();
  states[states.size()-1].name(name);
  return states.size() - 1;
}


void abstract_cfg::mark_final(state_id_type state)
{
  states[state].final = true;
}

void abstract_cfg::tag_edge(state_id_type state, uint8_t edge)
{
  edges[state][edge].tag = make_shared<abstraction::symbol>(thread_id, state, edge);
}


void abstract_cfg::add_edge(state_id_type from, state_id_type to, bool back_edge, bool auto_tag, reward_t cost)
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

state_id_type abstract_cfg::add_dummy_state()
{
  if (states.size() == 1)
    return add_state("Init");
  return add_state("Dummy");
}

void abstract_cfg::minimise(bool leave_lockables)
{
  std::unordered_set<state_id_type> remain; // leave these states alone
  if (leave_lockables) {
    for (unsigned i = 0; i <= no_states(); ++i) {
      if (states[i].lock_before != clang::SourceLocation() || states[i].lock_after != clang::SourceLocation()) {
        remain.insert(i);
      }
    }
  }
  std::unordered_set<state_id_type> seen;
  
  // add the current set of initial states to the frontier
  std::deque<pair<state_id_type,vector<state_id_type>>> frontier2;
  frontier2.push_back(make_pair(1,vector<state_id_type>()));
  
  // check if the successor of every state is good, otherwise replace with successor of successor
  while (!frontier2.empty()) {
    pair<state_id_type,vector<state_id_type>> nextp = frontier2.front();
    state_id_type next = nextp.first;
    assert(states[next].action || states[next].final || next == 1 || remain.find(next)!=remain.end());
    vector<state_id_type> parents = nextp.second;
    frontier2.pop_front();
    if (seen.find(next) == seen.end()) {
      parents.push_back(next);
      seen.insert(next);
      
      set<edge> successors; // set of successors
      unsigned i;
      for (i = 0; i<edges[next].size();++i) {
        edge edge_to = edges[next][i];
        state& succ = states[edge_to.to];
        if (!succ.action && !succ.final && remain.find(edge_to.to)==remain.end()) {
          // remove this successor
          for (const edge& edge2 : get_successors(edge_to.to)) {
            edge new_edge(edge2);
            if (!new_edge.tag) new_edge.tag = edge_to.tag;
            new_edge.in_betweeners.insert(new_edge.in_betweeners.begin(), edge_to.to);
            new_edge.in_betweeners.insert(new_edge.in_betweeners.begin(), edge_to.in_betweeners.begin(), edge_to.in_betweeners.end());
            edges[next].push_back(new_edge);
          }
        } else {
          successors.insert(edge_to);
        }
      }
      edges[next].clear();
      // add information to the edges
      for (edge e : successors) {
        frontier2.push_back(make_pair(e.to,parents));
        reward_t cost = parents.end() - find(parents.begin(), parents.end(), e.to);
        bool back_edge = cost != 0;
        e.back_edge = back_edge;
        edges[next].push_back(e);
        if (e.tag) tag_edge(next, edges[next].size()-1); // only tag those tagged before
      }
    }
  }
}


void abstract_cfg::compact()
{
  vector<bool> active(states.size(), false);
  // get a list of active states
  std::deque<state_id_type> frontier;
  frontier.push_back(1);
  
  while (!frontier.empty()) {
    state_id_type next = frontier.front();
    frontier.pop_front();
    active[next] = true;
    for (edge& e : edges[next]) {
      if (!e.back_edge) {
        frontier.push_back(e.to);
      }
    }
  }
  
  vector<state_id_type> mapping(states.size(), no_state);
 
  //filter out the ones not active
  state_id_type last_active = 0;
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
          states[i].id(i);
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
    if (states[i].return_state != no_state && mapping[states[i].return_state]!=no_state) {
      states[i].return_state = mapping[states[i].return_state];
    }
    for (edge& e : edges[i]) {
      if (mapping[e.to]!=no_state) e.to = mapping[e.to];
      e.id(i);
      e.in_betweeners.clear();
    }
  }
}

const unordered_set< state_id_type > abstract_cfg::get_forward_successors(state_id_type from) const
{
  unordered_set< state_id_type > res;
  for (const edge& e : edges[from]) {
    if (!e.back_edge) res.insert(e.to);
  }
  return res;
}

