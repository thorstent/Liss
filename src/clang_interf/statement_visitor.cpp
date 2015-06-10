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

#include "statement_visitor.h"
#include <iostream>
#include <clang/AST/ASTContext.h>
#include "abstraction/symbol.h"
#include "parse_error.h"
#include "cfg_visitor.h"
#include "options.h"


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
        symbol action(access_type, variable, var, identifier_store, stmt, function);
        state_id_type next = thread.add_state(action);
        check_mainfile(stmt, next);
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
  if (seen_stmt.find(s) == seen_stmt.end())
    return RecursiveASTVisitor::TraverseStmt(s);
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
    symbol action(operation, var_name, var, identifier_store, s, function);
    action.assume = assume;
    action.synthesised = synthesised;
    state_id_type next = thread.add_state(action);
    check_mainfile(s, next);
    add_successor(next);
  } else {
    RecursiveASTVisitor::TraverseCallExpr(s);
    if (isa<FunctionDecl>(s->getCalleeDecl())) {
      FunctionDecl* callee = cast<FunctionDecl>(s->getCalleeDecl());
      string name = callee->getNameInfo().getName().getAsString();
      if (callee->hasBody()) {
        Stmt* body = callee->getBody();
        std::unique_ptr<clang::CFG> cfg = CFG::buildCFG(callee, callee->getBody(), &context, clang::CFG::BuildOptions());
        cfg_visitor cvisitor(context, thread, identifier_store, cfg->getExit());
        cvisitor.process_block(cfg->getEntry(), body, no_state);
        add_successor(cvisitor.entry_state());
        end_state = cvisitor.exit_state();
        auto& es = thread.get_state(cvisitor.entry_state());
        es.return_state = cvisitor.exit_state();
        es.name("call " + name);
        es.lock_policy = lock_policy_t::before;
        es.lock_stmt = s;
        es.lock_function = function;
        auto& exs = thread.get_state(cvisitor.exit_state());
        exs.name("ret " + name);
        exs.lock_policy = lock_policy_t::after;
        exs.lock_stmt = s;
        exs.lock_function = function;
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
  } else {
    start_state = successor;
  }
  end_state = successor;
}

void statement_visitor::check_mainfile(Stmt* stmt, state_id_type state)
{
  clang::FileID id = context.getSourceManager().getFileID(stmt->getLocStart());
  if (id!=context.getSourceManager().getMainFileID()) {
    thread.get_state(state).lock_policy = lock_policy_t::none;
  }
}

