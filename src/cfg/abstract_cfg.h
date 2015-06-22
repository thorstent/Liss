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

#ifndef CFG_ABSTRACT_CFG_H
#define CFG_ABSTRACT_CFG_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "types.h"
#include "abstraction/symbol.h"

#include <clang/Basic/SourceLocation.h>

namespace std {
  template <>
  struct hash<clang::SourceLocation> {
    size_t operator()(const clang::SourceLocation& loc) const {
      return loc.getRawEncoding();
    }
  };
}

namespace clang {
  class Stmt;
  class FunctionDecl;
}

namespace cfg {

class abstract_cfg;
  
struct state {
  inline state_id_type id() { assert(action||non_action_symbol); return action ? action->loc.state : non_action_symbol->loc.state; }
  void id(state_id_type new_id);
  std::shared_ptr<const abstraction::symbol> action;
  std::shared_ptr<const abstraction::symbol> non_action_symbol; // the symbol that is used to represent this state if no action occured
  bool final = false;
  bool non_det = false; // state branches non-deterministically
  void name(std::string newn);
  state_id_type return_state = no_state; // if this is a function call, then it contains the position where the function call returns
  clang::SourceLocation lock_before, lock_after; // the place to put locks if this state can be locked
  clang::Stmt* braces_needed = nullptr; // if braces need to be added before adding locks this points to the instruction
  state(state_id_type id, const abstraction::symbol& action);
  state(thread_id_type thread_id, state_id_type id);
  reward_t distance = 0; // max distance from initial state (no back edges)
  inline bool is_dummy() const { return static_cast<bool>(non_action_symbol); }
private:
};

std::ostream& operator<<(std::ostream& os, const state& s);

struct edge {
  void id(state_id_type new_id);
  bool back_edge = false; // this edge is the back-edge of a loop
  std::shared_ptr<const abstraction::symbol> tag = nullptr; // can be null
  state_id_type to;
  reward_t cost = 0; // back_edges have a cost for going back, all other edges have cost 0
  std::vector<state_id_type> in_betweeners; // a list of states in between that has been removed by minimise
  edge(state_id_type to, bool back_edge, int cost) : to(to), back_edge(back_edge), cost(cost) {}
  bool operator==(const edge& other) const;
  bool operator<(const edge& other) const;
};

std::ostream& operator<<(std::ostream& os, const edge& e);
  
// States are integers. Every state has a command attached to it
class abstract_cfg
{
public:
  abstract_cfg(const clang::FunctionDecl* fd, thread_id_type thread_id);
  abstract_cfg(const abstract_cfg& other) = default;
  const clang::FunctionDecl* declaration; // just to have this around
  
  state_id_type add_state(abstraction::symbol symbol);
  state_id_type add_state(const std::string& name);
  state_id_type add_dummy_state();
  //const std::shared_ptr<abstraction::symbol>& lookup_state(const clang::Stmt* stmt) const;
  void mark_final(state_id_type state);

  /**
   * @brief ...
   * 
   * @param from ...
   * @param to ...
   * @param back_edge A back edge is an edge that points to an earlier node in the CFG (without them the CFG is acyclic)
   */
  void add_edge(state_id_type from, state_id_type to, bool back_edge = false, bool auto_tag = true, reward_t cost = 0);
  
  /**
   * @brief Removes unneeded states from the CFG, leaving only the most important ones (those that have actions)
   * 
   * @param leave_lockables means that the states that can be locked remain
   */
  void minimise(bool leave_lockables);
  
  /**
   * @brief Ensures that the arrays are tightly packed
   */
  void compact();
  
  std::string name;
  
  inline const std::unordered_set<state_id_type> initial_states() const {
    std::unordered_set<state_id_type> ret;
    ret.insert(1); // default initial state
    return ret;
  }
  /**
   * @brief Gets a state
   * 
   * @param id The state id 1..no_states
   */
  inline const state& get_state(state_id_type id) const { 
    if (id <= 0) throw std::invalid_argument("Id must be >0");
    return states[id];
  }
  inline state& get_state(state_id_type id) { 
    if (id <= 0) throw std::invalid_argument("Id must be >0");
    return states[id];
  }
  inline const std::vector<edge>& get_successors(state_id_type from) const {
    return edges[from];
  }
  const std::unordered_set<state_id_type> get_forward_successors(state_id_type from) const;
  inline unsigned no_states() const { return states.size()-1; }
  
  const thread_id_type thread_id;

private:
  typedef std::unordered_set<std::unordered_multiset<const abstraction::symbol*>> set_of_set;
  std::vector<state> states;
  std::vector<std::vector<edge>> edges;
  void tag_edge(state_id_type state, uint8_t edge);
};
}

namespace Limi {
  template<> struct printer<state_id_type> : printer_base<state_id_type> {
    printer(const cfg::abstract_cfg& thread) : thread_(thread) {}
    virtual void print(const state_id_type& state, std::ostream& out) const override {
      out << thread_.get_state(state);
    }
  private:
    const cfg::abstract_cfg& thread_;
  };
}

#endif // CFG_ABSTRACT_CFG_H
