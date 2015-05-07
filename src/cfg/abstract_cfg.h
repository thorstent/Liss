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

namespace clang {
  class Stmt;
  class FunctionDecl;
}

namespace cfg {

//TODO: Remove distance if not needed
struct state {
  state_id id;
  std::shared_ptr<abstraction::symbol> action;
  bool final = false;
  bool non_det = false; // state branches non-deterministically
  std::string name;
  unsigned distance = 0;
  state(state_id id, const abstraction::symbol& action) : id(id), action(std::make_shared<abstraction::symbol>(action)) {}
  state(state_id id) : id(id), action(nullptr) {}
};

std::ostream& operator<<(std::ostream& os, const state& s);

struct edge {
  bool back_edge = false; // this edge is the back-edge of a loop
  std::shared_ptr<abstraction::symbol> tag = nullptr; // can be null
  state_id to;
  reward_t cost = 0; // back_edges have a cost for going back, all other edges have cost 0
  edge(state_id to, bool back_edge, int cost) : to(to), back_edge(back_edge), cost(cost) {}
};

std::ostream& operator<<(std::ostream& os, const edge& e);
  
// States are integers. Every state has a command attached to it
class abstract_cfg
{
public:
  abstract_cfg(const clang::FunctionDecl* fd);
  abstract_cfg(const abstract_cfg& other) = default;
  const clang::FunctionDecl* declaration; // just to have this around
  
  state_id add_state(const abstraction::symbol& symbol, const clang::Stmt* stmt);
  state_id add_state(const std::string& name);
  state_id add_dummy_state();
  //const std::shared_ptr<abstraction::symbol>& lookup_state(const clang::Stmt* stmt) const;
  void mark_final(state_id state);

  /**
   * @brief ...
   * 
   * @param from ...
   * @param to ...
   * @param back_edge A back edge is an edge that points to an earlier node in the CFG (without them the CFG is acyclic)
   */
  void add_edge(state_id from, state_id to, bool back_edge = false, bool auto_tag = true, reward_t cost = 0);
  
  /**
   * @brief Removes unneeded states from the CFG, leaving only the most important ones (those that have actions)
   * 
   */
  void minimise();
  
  
  
  std::string name;
  
  inline const std::unordered_set<state_id> initial_states() const {
    return initial_states_;
  }
  inline const state& get_state(state_id id) const { 
    return states[id];
  }
  inline state& get_state(state_id id) { 
    assert (id>0);
    assert (id!=no_state);
    return states[id];
  }
  inline const std::vector<edge> get_successors(state_id from) const {
    return edges[from];
  }

private:
  typedef std::unordered_set<std::unordered_multiset<const abstraction::symbol*>> set_of_set;
  std::vector<state> states;
  std::vector<std::vector<edge>> edges;
  std::unordered_set<state_id> initial_states_;
  void tag_edge(state_id state, uint8_t edge);
  unsigned calc_distance(state_id state);
};
}

namespace Limi {
  template<> struct printer<state_id> : printer_base<state_id> {
    printer(const cfg::abstract_cfg& thread) : thread_(thread) {}
    virtual void print(const state_id& state, std::ostream& out) const override {
      out << thread_.get_state(state);
    }
  private:
    const cfg::abstract_cfg& thread_;
  };
}

#endif // CFG_ABSTRACT_CFG_H