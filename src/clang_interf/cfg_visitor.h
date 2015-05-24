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

#ifndef CLANG_INTERF_CFG_VISITOR_H
#define CLANG_INTERF_CFG_VISITOR_H

#include "pch.h"
#include <unordered_map>

#include "cfg/abstract_cfg.h"
#include "abstraction/identifier_store.h"

#include "statement_visitor.h"


namespace clang_interf {
 

class cfg_visitor
{
public:
  typedef std::unordered_map<unsigned int,const state_id_type> block_map_t;
  cfg_visitor(clang::ASTContext& context, cfg::abstract_cfg& thread,
              abstraction::identifier_store& identifier_store, const clang::CFGBlock& exit) :
  context(context), thread(thread), identifier_store(identifier_store), exit_block(&exit) {}
  
  /**
   * @brief ...
   * 
   * @param block ...
   * @param last_state no_state if no proir state
   * @param parent_blocks The blocks already encountered on the way to this block
   */
  void process_block(const clang::CFGBlock& block, call_stack cstack, state_id_type last_state, std::unordered_set<unsigned> parent_blocks = std::unordered_set<unsigned>());
  
  state_id_type exit_state();
  state_id_type entry_state();
private:
  clang::ASTContext& context;
  cfg::abstract_cfg& thread;
  abstraction::identifier_store& identifier_store;
  block_map_t block_map;
  state_id_type exit_state_ = no_state;
  state_id_type entry_state_ = no_state;
  const clang::CFGBlock* exit_block;
};
}

#endif // CLANG_INTERF_CFG_VISITOR_H
