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
#include <clang/Lex/Lexer.h>
#include <iostream>

using namespace clang_interf;
using namespace std;
using namespace clang;

void cfg_visitor::process(const CFGBlock& block)
{
  process_block(block, no_state);
}

// inject an artifical element into the CFG
Stmt* make_artificial(ASTContext& context, string name, SourceLocation loc_start, SourceLocation loc_end) {
  BuiltinType* ch = new BuiltinType(BuiltinType::UInt);
  SourceLocation startend[] = { loc_start, loc_end };
  StringLiteral* str = StringLiteral::Create(context, name, StringLiteral::Ascii, false, QualType(ch, 0), startend , 2);
  return str;
}

cfg_visitor cfg_visitor::process_function(ASTContext& context, cfg::abstract_cfg& thread, abstraction::identifier_store& identifier_store, const clang::FunctionDecl* callee)
{
  Stmt* body = callee->getBody();
  string name = callee->getNameInfo().getAsString();
  assert(isa<CompoundStmt>(body));
  CompoundStmt* com = cast<CompoundStmt>(body);
  Stmt** children = new Stmt*[com->size()+2];
  unsigned i = 0;
  // function start
  Stmt* s = make_artificial(context, "start " + name, SourceLocation(), body->getLocStart().getLocWithOffset(1));
  children[i] = s;++i;
  for(Stmt* c : com->body()) {
    children[i] = c;
    ++i;
  }
  // function end
  s = make_artificial(context, "end " + name, body->getLocEnd(), SourceLocation());
  children[i] = s;++i;
  com->setStmts(context, children, i);
  clang::CFG::BuildOptions bo;
  std::unique_ptr<clang::CFG> cfg = CFG::buildCFG(callee, callee->getBody(), &context, bo);
  //cout << name << endl;
  //cfg->dump(context.getLangOpts(), false);
  cfg_visitor cvisitor(context, thread, identifier_store, cfg->getExit(), callee);
  cvisitor.process(cfg->getEntry());
  // restore original
  com->setStmts(context, &children[1], i-2);
  return cvisitor;
}



// parent_blocks is used to identify back_edges
void cfg_visitor::process_block(const CFGBlock& block, state_id_type last_state, unordered_set<unsigned> parent_blocks)
{
  unordered_set<const Stmt*> seen_stmt;
  
  statement_visitor svisitor(context, thread, identifier_store, seen_stmt, function);
  
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
      state_id_type dummy_state = thread.add_dummy_state();
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
        process_block(*succ_block, last_state, parent_blocks);
      }
    }
    
      
  } else {
    bool back_edge = parent_blocks.find(block.getBlockID()) != parent_blocks.end(); // this block was encountered before
    thread.add_edge(last_state, found->second, back_edge);
  }
  

}

state_id_type cfg_visitor::entry_state()
{
  return entry_state_;
}

state_id_type cfg_visitor::exit_state()
{
  return exit_state_;
}


