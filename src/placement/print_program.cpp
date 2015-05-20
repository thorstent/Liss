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

#include <clang/AST/ParentMap.h>

using namespace placement;
using namespace std;
using namespace clang;

print_program::print_program(const cfg::program& program) : program(program)
{

}

void print_program::place_locks(Rewriter& rewriter, const vector< pair< unsigned, placement::location > >& locks, const string name, bool after, unordered_set<Stmt*>& added_brace) {
  for (pair< unsigned, placement::location > l : locks) {
    // find location
    
    const cfg::state& state = program.minimised_threads()[l.second.thread]->get_state(l.second.state);
    assert(state.action);
    parent_result parent = find_stmt_parent(state.action->instr_stmt(), state.action->function_stmt());
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
      rewriter.InsertText(start, name + "(l" + to_string(l.first) + ");\n", true, true);
    } else {
      rewriter.InsertText(end, "\n" + name + "(l" + to_string(l.first) + ");", false, true);
    }
  }
}

void print_program::print_with_locks(const vector< pair< unsigned, placement::location > >& locks, const vector< pair< unsigned, placement::location > >& unlocks, const string& outname)
{
  Rewriter rewriter(program.ast_context.getSourceManager(), program.ast_context.getLangOpts());
  
  unordered_set<Stmt*> added_brace;
  place_locks(rewriter, locks, "lock_s", false, added_brace);
  place_locks(rewriter, unlocks, "unlock_s", true, added_brace);
  
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


print_program::parent_result print_program::find_stmt_parent(Stmt* stmt, Stmt* function)
{
  ParentMap map(function);
  bool ends_semicolon = true;
  while (true) {
    Stmt* parent = map.getParent(stmt);
    if (isa<CompoundStmt>(parent)) {
      return parent_result(stmt, false, ends_semicolon);
    } else if (IfStmt* ifs = dyn_cast<IfStmt>(parent)) {
      if (stmt!=ifs->getCond())
        return parent_result(stmt, true, ends_semicolon);
      ends_semicolon = false;
    } else if (WhileStmt* whiles = dyn_cast<WhileStmt>(parent)) {
      if (stmt!=whiles->getCond())
        return parent_result(stmt, true, ends_semicolon);
      ends_semicolon = false;
    } else if (DoStmt* dos = dyn_cast<DoStmt>(parent)) {
      if (stmt!=dos->getCond()) 
        return parent_result(stmt, true, ends_semicolon);
      ends_semicolon = true;
    } else {
      ends_semicolon = true;
    }
    stmt = parent;
  }
}
