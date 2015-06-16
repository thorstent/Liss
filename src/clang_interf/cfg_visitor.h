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
              abstraction::identifier_store& identifier_store, const clang::CFGBlock& exit, const clang::FunctionDecl* function) :
  context(context), thread(thread), identifier_store(identifier_store), exit_block(&exit), function(function) {}
  
  void process(const clang::CFGBlock& block);
  
  state_id_type exit_state();
  state_id_type entry_state();
  
  static cfg_visitor process_function(clang::ASTContext& context, cfg::abstract_cfg& thread, abstraction::identifier_store& identifier_store, const clang::FunctionDecl* callee);
private:
  clang::ASTContext& context;
  cfg::abstract_cfg& thread;
  abstraction::identifier_store& identifier_store;
  block_map_t block_map;
  state_id_type exit_state_ = no_state;
  state_id_type entry_state_ = no_state;
  const clang::CFGBlock* exit_block;
  const clang::FunctionDecl* function;
  
  /**
   * @brief ...
   * 
   * @param block ...
   * @param last_state no_state if no proir state
   * @param parent_blocks The blocks already encountered on the way to this block
   */
  void process_block(const clang::CFGBlock& block, state_id_type last_state, std::unordered_set<unsigned> parent_blocks = std::unordered_set<unsigned>());
  /**
   * @brief Checks if this is an artifical node and if so adds the locking
   * 
   * @returns no_state if not artificial
   */
  state_id_type add_locking_artificial(clang::Stmt* stmt);
};
}

#endif // CLANG_INTERF_CFG_VISITOR_H
