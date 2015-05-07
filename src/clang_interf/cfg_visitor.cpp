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

#include "cfg_visitor.h"

#include <stdexcept>
#include <clang/AST/ASTContext.h>
#include <cassert>

using namespace clang_interf;
using namespace std;
using namespace clang;

// parent_blocks is used to identify back_edges
void cfg_visitor::process_block(const CFGBlock& block, call_stack cstack, state_id last_state, unordered_set<unsigned> parent_blocks)
{
  unordered_set<const Stmt*> seen_stmt;
  
  statement_visitor svisitor(context, thread, identifier_store, seen_stmt, cstack);
  
  auto found = block_map.find(block.getBlockID());
  if (found == block_map.end()) {
    for (const CFGElement& e : block) {
      Optional<CFGStmt> cfgstmt = e.getAs<CFGStmt>();
      if (cfgstmt.hasValue()) {
        Stmt* stmt = const_cast<Stmt*>(cfgstmt.getValue().getStmt());
        svisitor.TraverseStmt(stmt);
        seen_stmt.insert(stmt);
      }
    }
    if (svisitor.progress()) {
      // make the connection
      if (last_state!=no_state)
        thread.add_edge(last_state, svisitor.first_state());
      last_state = svisitor.last_state();
      // cache the first state for later
      block_map.insert(make_pair(block.getBlockID(), svisitor.first_state()));
    } else {
      state_id dummy_state = thread.add_dummy_state();
      // make the connection
      if (last_state!=no_state)
        thread.add_edge(last_state, dummy_state);
      last_state = dummy_state;
      // cache the first state for later
      block_map.insert(make_pair(block.getBlockID(), dummy_state));
    }
    
    if (entry_state_ == no_state) {
      // this is the first state
      assert(!svisitor.progress());
      entry_state_ = last_state;
    }
    
    if (&block == exit_block) {
      assert(!svisitor.progress());
      exit_state_ = last_state;
    }
    
    // check terminator to see if it is a loop or if
    if (block.getTerminator()) {
      switch (block.getTerminator()->getStmtClass()) {
        default: break;
        case Stmt::WhileStmtClass:
          thread.get_state(last_state).non_det = svisitor.last_nondet;
          break;
        case Stmt::IfStmtClass:
          thread.get_state(last_state).non_det = svisitor.last_nondet;
          break;
      }
    }
    
    parent_blocks.insert(block.getBlockID());
    for (auto it = block.succ_begin(); it != block.succ_end(); ++it) {
      const CFGBlock* succ_block = it->getReachableBlock();
      if (succ_block) {
        process_block(*succ_block, cstack, last_state, parent_blocks);
      }
    }
    
      
  } else {
    bool back_edge = parent_blocks.find(block.getBlockID()) != parent_blocks.end(); // this block was encountered before
    thread.add_edge(last_state, found->second, back_edge);
  }
  

}

state_id cfg_visitor::entry_state()
{
  return entry_state_;
}

state_id cfg_visitor::exit_state()
{
  return exit_state_;
}


