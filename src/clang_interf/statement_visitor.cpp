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
      if (variable != "nondet_int") {
        string type_name = get_type_name(stmt);
        if (type_name == "lock_t" || type_name == "conditional_t")
          throw parse_error("Variable " + variable + " must not use type lock_t or conditional_t");
        variable_type var = identifier_store.insert_variable(variable);
        cstack.back().second = stmt;
        symbol action(access_type, cstack, variable, var, identifier_store, stmt);
        state_id next = thread.add_state(action, stmt);
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
  statement_visitor svisitor(context, thread, identifier_store, seen_stmt, cstack, true);
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
  string var_name;
  if (isa<FunctionDecl>(s->getCalleeDecl())) {
    FunctionDecl* callee = cast<FunctionDecl>(s->getCalleeDecl());
    DeclarationName dn = callee->getNameInfo().getName();
    std::string name = dn.getAsString();
    if (name=="int_yield") {
      operation = abstraction::op_class::yield;
    } else {
      // see if we find an argument
      if (s->getNumArgs()==1 && isa<ImplicitCastExpr>(s->getArg(0))) {
        ImplicitCastExpr* arg = cast<ImplicitCastExpr>(s->getArg(0));
        if (isa<DeclRefExpr>(arg->getSubExpr())){
          DeclRefExpr* argexp = cast<DeclRefExpr>(arg->getSubExpr());
          var_name = argexp->getFoundDecl()->getNameAsString();
          string type_name = get_type_name(argexp);
          
          if (name == "int_lock") {
            if (type_name != "lock_t")
              throw parse_error("Variable " + var_name + " must be of type lock_t");
            var = identifier_store.insert_lock(var_name);
            operation = abstraction::op_class::lock;
          } else if (name == "int_unlock") {
            if (type_name != "lock_t")
              throw parse_error("Variable " + var_name + " must be of type lock_t");
            var = identifier_store.insert_lock(var_name);
            operation = abstraction::op_class::unlock;
          } else if (name == "int_notify") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::notify;
          } else if (name == "int_reset") {
              if (type_name != "conditional_t")
                throw parse_error("Variable " + var_name + " must be of type conditional_t");
              var = identifier_store.insert_conditonal(var_name);
              operation = abstraction::op_class::reset;
          } else if (name == "int_wait" || name == "int_assume") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::wait;
            assume = name == "int_assume";
          } else if (name == "int_wait_not" || name == "int_assume_not") {
            if (type_name != "conditional_t")
              throw parse_error("Variable " + var_name + " must be of type conditional_t");
            var = identifier_store.insert_conditonal(var_name);
            operation = abstraction::op_class::wait_not;
            assume = name == "int_assume_not";
          } else if (name == "int_wait_reset") {
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
    cstack.back().second = s;
    symbol action(operation, cstack, var_name, var, identifier_store, s);
    action.assume = assume;
    state_id next = thread.add_state(action, s);
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
        call_stack cs(cstack);
        cs.back().second = s;
        cs.push_back(make_pair(body, nullptr));
        cvisitor.process_block(cfg->getEntry(), cs, no_state);
        add_successor(cvisitor.entry_state());
        end_state = cvisitor.exit_state();
        
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

state_id statement_visitor::last_state()
{
  return end_state;
}

state_id statement_visitor::first_state()
{
  return start_state;
}

void statement_visitor::add_successor(state_id successor)
{
  if (end_state != no_state) {
    thread.add_edge(end_state, successor);
  } else {
    start_state = successor;
  }
  end_state = successor;
}


