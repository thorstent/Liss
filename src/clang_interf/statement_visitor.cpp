/*
 * Copyright 2016, IST Austria
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

#include "statement_visitor.h"
#include <iostream>
#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/Stmt.h>
#include "abstraction/symbol.h"
#include "parse_error.h"
#include "cfg_visitor.h"
#include "options.h"
#include "clang_interf/clang_helpers.h"

using namespace clang_interf;
using namespace std;
using namespace clang;
using namespace abstraction;

bool statement_visitor::VisitDeclRefExpr(DeclRefExpr* stmt)
{
  NamedDecl* decl = stmt->getFoundDecl();
  if (isa<VarDecl>(decl) && !isa<ParmVarDecl>(decl)) {
    VarDecl* vd = cast<VarDecl>(decl);
    if (!vd->isLocalVarDecl()) {
      string variable = decl->getName().str();
      if (variable != "nondet") {
        string type_name = get_type_name(stmt);
        if (type_name == "lock_t" || type_name == "conditional_t")
          throw parse_error("Variable " + variable + " must not use type lock_t or conditional_t");
        variable_type var = identifier_store.insert_variable(variable);
        symbol action(access_type, variable, var, identifier_store, stmt);
        state_id_type next = thread.add_state(action);
        lock_locations(stmt, next);
        add_successor(next);
      } else {
        last_nondet = true;
      }
    }
  }
  return true;
}

bool statement_visitor::TraverseBinAssign(BinaryOperator* stmt)
{
  TraverseStmt(stmt->getRHS());
  statement_visitor svisitor(context, thread, identifier_store, seen_stmt, function, true);
  svisitor.TraverseStmt(stmt->getLHS());
  if (svisitor.progress()) {
    add_successor(svisitor.end_state);
  }
  return true;
}

bool statement_visitor::TraverseStmt(Stmt* s)
{
  last_nondet = false;
  if (seen_stmt.find(s) == seen_stmt.end()) {
    if (!process_artificial(s))
      return RecursiveASTVisitor::TraverseStmt(s);
  }
  return true;
}

bool statement_visitor::process_artificial(Stmt* s)
{
  // check if the statement is artificial
  if (StringLiteral* str = dyn_cast_or_null<StringLiteral>(s)) {
    if (const BuiltinType* typ = dyn_cast<BuiltinType>(str->getType().getTypePtr())) {
      if (typ->getKind() == BuiltinType::UInt) {
        string name = str->getString();
        state_id_type state_id = thread.add_state(name);
        lock_locations(s, state_id, true, true);
        add_successor(state_id);
        return true; 
      }
    }
  }
  return false;
}


bool statement_visitor::TraverseReturnStmt(clang::ReturnStmt* s) {
  for (const auto& child : s->children()) {
    TraverseStmt(child);
  }
  string name = function->getNameInfo().getAsString();
  state_id_type state_id = thread.add_state("return " + name);
  lock_locations(s, state_id, true, false);
  add_successor(state_id);
  return true;
}

bool statement_visitor::TraverseCallExpr(CallExpr* s)
{
  abstraction::op_class operation = abstraction::op_class::epsilon;
  variable_type var = 0;
  bool assume = false;
  bool synthesised = false; // this is a synthesised lock
  string var_name;
  if (isa<FunctionDecl>(s->getCalleeDecl())) {
    FunctionDecl* callee = cast<FunctionDecl>(s->getCalleeDecl());
    DeclarationName dn = callee->getNameInfo().getName();
    std::string name = dn.getAsString();
    if (name=="yield") {
      operation = abstraction::op_class::yield;
    } else {
      // see if we find an argument
      if (s->getNumArgs()==1 && isa<ImplicitCastExpr>(s->getArg(0))) {
        ImplicitCastExpr* arg = cast<ImplicitCastExpr>(s->getArg(0));
        if (isa<DeclRefExpr>(arg->getSubExpr())){
          DeclRefExpr* argexp = cast<DeclRefExpr>(arg->getSubExpr());
          var_name = argexp->getFoundDecl()->getNameAsString();
          string type_name = get_type_name(argexp);
          
          if (name == "lock" || name == "lock_s") {
            if (type_name != "lock_t")
              throw parse_error("Variable " + var_name + " must be of type lock_t");
            var = identifier_store.insert_lock(var_name);
            operation = abstraction::op_class::lock;
            synthesised = name == "lock_s";
          } else if (name == "unlock" || name == "unlock_s") {
            if (type_name != "lock_t")
              throw parse_error("Variable " + var_name + " must be of type lock_t");
            var = identifier_store.insert_lock(var_name);
            operation = abstraction::op_class::unlock;
            synthesised = name == "unlock_s";
          } else if (name == "notify") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::notify;
          } else if (name == "reset") {
              if (type_name != "conditional_t")
                throw parse_error("Variable " + var_name + " must be of type conditional_t");
              var = identifier_store.insert_conditonal(var_name);
              operation = abstraction::op_class::reset;
          } else if (name == "wait" || name == "assume") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::wait;
            assume = name == "assume";
          } else if (name == "wait_not" || name == "assume_not") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::wait_not;
            assume = name == "assume_not";
          } else if (name == "wait_reset") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::wait_reset;
          }
        }
      }
    }
  }
  
  if (operation != abstraction::op_class::epsilon) {
    symbol action(operation, var_name, var, identifier_store, s);
    action.assume = assume;
    action.synthesised = synthesised;
    state_id_type next = thread.add_state(action);
    lock_locations(s, next);
    add_successor(next);
  } else {
    RecursiveASTVisitor::TraverseCallExpr(s);
    if (isa<FunctionDecl>(s->getCalleeDecl())) {
      FunctionDecl* callee = cast<FunctionDecl>(s->getCalleeDecl());
      string name = callee->getNameInfo().getAsString();
      if (callee->hasBody()) {
        // add call state
        state_id_type state_start = thread.add_state("call " + name);
        lock_locations(s, state_start, true, false);
        add_successor(state_start);
        
        cfg_visitor cvisitor = cfg_visitor::process_function(context, thread, identifier_store, callee);
        add_successor(cvisitor.entry_state());
        end_state = cvisitor.exit_state();
        
        state_id_type state_id = thread.add_state("ret " + name);
        lock_locations(s, state_id, false, true);
        add_successor(state_id);
        
        thread.get_state(state_start).return_state = state_id;
      } else {
        cerr << "Ignoring function without body: " << name << endl;
      }
    }
  }
  
  return true;
}

string statement_visitor::get_type_name(DeclRefExpr* expr)
{
  if (isa<VarDecl>(expr->getFoundDecl())) {
    VarDecl* decl = cast<VarDecl>(expr->getFoundDecl());
    if (isa<TypedefType>(decl->getType().getTypePtrOrNull())) {
      const TypedefType* type = cast<TypedefType>(decl->getType().getTypePtrOrNull());
      return type->getDecl()->getNameAsString();
    }
  }
  return string();
}

state_id_type statement_visitor::last_state()
{
  return end_state;
}

state_id_type statement_visitor::first_state()
{
  return start_state;
}

void statement_visitor::add_successor(state_id_type successor)
{
  if (end_state != no_state) {
    thread.add_edge(end_state, successor);
    // check if the locking makes sense (this could be a problem if two are in the same instruction)
    cfg::state& prev = thread.get_state(end_state);
    cfg::state& next = thread.get_state(successor);
    if (prev.lock_after == next.lock_after) {
      // these are part of the same statement
      prev.lock_after = SourceLocation();
      next.lock_before = SourceLocation();
    }
  } else {
    start_state = successor;
  }
  end_state = successor;
}

void statement_visitor::lock_locations(Stmt* stmt, state_id_type state_id, bool allow_before, bool allow_after)
{
  SourceManager& source_manager = context.getSourceManager();
    
  parent_result parent = find_stmt_parent(stmt, function->getBody());
  if (parent.stmt_to_lock) {
    SourceLocation start = parent.stmt_to_lock->getLocStart();
    SourceLocation end = parent.stmt_to_lock->getLocEnd();
    if (end.isMacroID()) {
      end = source_manager.getExpansionRange(end).second;
    }
    if (start.isMacroID()) {
      start = source_manager.getExpansionRange(start).first;
    }
    
    if (end.isValid()) {
      SourceLocation end_new = findLocationAfterSemi(end, context);
      if (end_new.isValid()) end = end_new;
    }
    
    if (start.isMacroID() || end.isMacroID()) {
      throw logic_error("Caught macro expansion");
    }
    
    if (start.isValid() || end.isValid()) {
      clang::FileID id = start.isValid() ? source_manager.getFileID(start) : source_manager.getFileID(end);
        if (id==source_manager.getMainFileID()) { // otherwise no lock placements
        cfg::state& state = thread.get_state(state_id);
        if (allow_before)
          state.lock_before = start;
        if (allow_after)
          state.lock_after = end;
        if (parent.braces_needed) state.braces_needed = parent.stmt_to_lock;
      }
    }
    
  }
}

