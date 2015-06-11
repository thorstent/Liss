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

#include "print_program.h"

#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/Expr.h>
#include "clang_interf/clang_helpers.h"

#include <iostream>
#include <sstream>

using namespace placement;
using namespace std;
using namespace clang;

print_program::print_program(const cfg::program& program) : program(program)
{

}

void print_program::place_text(Rewriter& rewriter, const cfg::state& state, const string& text, bool after, unordered_set<Stmt*>& added_brace) {
  if (state.braces_needed && added_brace.insert(state.braces_needed).second) {
    SourceLocation start = state.braces_needed->getLocStart();
    SourceLocation end = state.braces_needed->getLocEnd();
    // MeasureTokenLength gets us past the last token, and adding 1 gets
    // us past the ';'.
    int offset = Lexer::MeasureTokenLength(end, rewriter.getSourceMgr(), rewriter.getLangOpts());
    if (clang_interf::ends_semicolon(state.braces_needed)) offset += 1;
    end = end.getLocWithOffset(offset);
    // we need to add braces around the expression
    rewriter.InsertText(start, "{", true, true);
    rewriter.InsertText(end, "}", true, true);    
  }
  if(!after) {
    SourceLocation loc = state.lock_before;
    assert(rewriter.isRewritable(loc));
    rewriter.InsertText(loc, text + "\n", true, true);
  } else {
    SourceLocation loc = state.lock_after;
    assert(rewriter.isRewritable(loc));
    rewriter.InsertText(loc, "\n" + text, false, true);
  }
}

void print_program::place_locks(Rewriter& rewriter, const vector< pair< unsigned, abstraction::location > >& locks, const string name, bool after, unordered_set<Stmt*>& added_brace) {
  /*for (pair< unsigned, abstraction::location > l : locks) {
    // find location
    
    const cfg::state& state = program.minimised_threads()[l.second.thread]->get_state(l.second.state);
    parent_result parent = find_stmt_parent(state.lock_stmt, state.lock_function);
    SourceLocation start = parent.stmt_to_lock->getLocStart();
    SourceLocation end = parent.stmt_to_lock->getLocEnd();
    
    // MeasureTokenLength gets us past the last token, and adding 1 gets
    // us past the ';'.
    int offset = Lexer::MeasureTokenLength(end, rewriter.getSourceMgr(), rewriter.getLangOpts());
    if (parent.ends_semicolon) offset += 1;
    end = end.getLocWithOffset(offset);
    if (parent.braces_needed && added_brace.insert(parent.stmt_to_lock).second) {
      // we need to add braces around the expression
      rewriter.InsertText(start, "{", true, true);
      rewriter.InsertText(end, "}", true, true);
    }
    assert(rewriter.isRewritable(start));
    if(!after) {
      rewriter.InsertText(start, name + "(" + lock_name + to_string(l.first) + ");\n", true, true);
    } else {
      rewriter.InsertText(end, "\n" + name + "(" + lock_name + to_string(l.first) + ");", false, true);
    }
  }*/
}

void print_program::place_lock_decl(Rewriter& rewriter, const std::unordered_set<unsigned>& locks_in_use)
{
  bool found_lockt = false;
  Decl* first_decl = nullptr; // the first declaration after lock_t is found
  for (auto x : program.translation_unit->decls()) {
    // find the lock_t declaration
    if (TypedefDecl* lockt = dyn_cast<TypedefDecl>(x)) {
      if (lockt->getNameAsString() == lock_t)
        found_lockt = true;
    } else 
      if (found_lockt && rewriter.getSourceMgr().getFileID(x->getLocation()) == program.ast_context.getSourceManager().getMainFileID()) {
        first_decl = x;
        break;
      }    
  }
  assert (first_decl);
  // insert locks after this
  SourceLocation start = first_decl->getLocStart();
  assert(rewriter.isRewritable(start));
  for (unsigned l : locks_in_use) {
    rewriter.InsertText(start, lock_t + " " + lock_name + to_string(l) + ";\n", true, true);
  }
}

void print_program::remove_duplicates(vector< pair< unsigned, abstraction::location > >& locks) {
  /*for (unsigned i = 0; i < locks.size(); ++i) {
    for (unsigned j = i+1; j < locks.size(); ++j) {
      if (locks[i].first == locks[j].first) {
        const auto& statei = program.minimised_threads()[locks[i].second.thread]->get_state(locks[i].second.state);
        const auto& statej = program.minimised_threads()[locks[j].second.thread]->get_state(locks[j].second.state);
        if (statei.lock_stmt == statej.lock_stmt) {
          // these are actually refering to the same instruction
          locks.erase(locks.begin()+j);
        }
      }
    }
  }*/
}

void print_program::print_with_locks(placement_result locks_to_place, const string& outname)
{
  remove_duplicates(locks_to_place.locks_a);
  remove_duplicates(locks_to_place.locks_b);
  remove_duplicates(locks_to_place.unlocks_a);
  remove_duplicates(locks_to_place.unlocks_b);
  Rewriter rewriter(program.ast_context.getSourceManager(), program.ast_context.getLangOpts());
  
  unordered_set<Stmt*> added_brace;
  place_locks(rewriter, locks_to_place.locks_b, lock_instr, false, added_brace);
  place_locks(rewriter, locks_to_place.locks_a, lock_instr, true, added_brace);
  place_locks(rewriter, locks_to_place.unlocks_b, unlock_instr, false, added_brace);
  place_locks(rewriter, locks_to_place.unlocks_a, unlock_instr, true, added_brace);
  
  std::unordered_set<unsigned> locks_in_use;
  for (const pair< unsigned, abstraction::location >& lock : locks_to_place.locks_a) {
    locks_in_use.insert(lock.first);
  }
  for (const pair< unsigned, abstraction::location >& lock : locks_to_place.locks_b) {
    locks_in_use.insert(lock.first);
  }
  place_lock_decl(rewriter, locks_in_use);
  
  // print out the program
  std::error_code error_code;
  llvm::raw_fd_ostream outFile(outname, error_code, llvm::sys::fs::F_None);
  rewriter.getEditBuffer(program.ast_context.getSourceManager().getMainFileID()).write(outFile);
  outFile.close();
}

void print_program::print_original(const string& outname)
{
  Rewriter rewriter(program.ast_context.getSourceManager(), program.ast_context.getLangOpts());
  
  // print out the program
  std::error_code error_code;
  llvm::raw_fd_ostream outFile(outname, error_code, llvm::sys::fs::F_None);
  rewriter.getEditBuffer(program.ast_context.getSourceManager().getMainFileID()).write(outFile);
  outFile.close();
}

void print_program::print_test_locations(const string& outname)
{
  Rewriter rewriter(program.ast_context.getSourceManager(), program.ast_context.getLangOpts());
  unordered_set<Stmt*> added_brace;
  
  unordered_set<SourceLocation> before_lock, after_lock;
  
  for (const cfg::abstract_cfg* thread : program.threads()) {
    for (unsigned i = 1; i <= thread->no_states(); ++i) {
      const cfg::state& state = thread->get_state(i);
      stringstream ss;
      ss << state;
      if (state.lock_before != SourceLocation() && before_lock.insert(state.lock_before).second)
        place_text(rewriter, state, "before " + ss.str(), false, added_brace);
      if (state.lock_after != SourceLocation() && after_lock.insert(state.lock_after).second)
        place_text(rewriter, state, "after " + ss.str(), true, added_brace);
    }
  }
  
  // print out the program
  std::error_code error_code;
  llvm::raw_fd_ostream outFile(outname, error_code, llvm::sys::fs::F_None);
  rewriter.getEditBuffer(program.ast_context.getSourceManager().getMainFileID()).write(outFile);
  outFile.close();
}


