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

#include "insertion.h"
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ParentMap.h>
#include <clang/AST/Expr.h>
#include <algorithm>
#include <stdexcept>

using namespace synthesis;
using namespace clang;
using namespace std;

insertion::insertion(cfg::program& program):
program(program), symbol_printer(Limi::printer<abstraction::psymbol>())
{
  
}

void insertion::add_locks(std::list< lock >& locks)
{
  for (auto it = locks.begin(); it!=locks.end(); ) {
    lock& l = *it;
    bool deleted = false;
    for (lock_location& lp : l.locations) {
      if (!find_parent(lp)) {
        deleted = true;
        it = locks.erase(it);
        break;
        //throw logic_error("No parent found");
      }
    }
    if (!deleted) ++it;
  }
  
  merge_locks(locks);
  
  for (lock& l : locks) {
    if (!l.declaration)
      l.declaration = add_declaration(l.name, "lock_t");
    // register the lock
    //abstraction::lock_type lock = program.identifiers().insert_lock(l.name);
    for (lock_location& lp : l.locations) {
      //insert_lock_thread(lp.start->thread_id(), lp, lock);
      
      lp.inserted_start = insert_call("int_lock", l.declaration, lp.parent, false, lp.starts);
      lp.inserted_end =  insert_call("int_unlock", l.declaration, lp.parent, true, lp.ends);
      lp.start.symbol = nullptr; // these pointers are invalid later on
      lp.end.symbol = nullptr;
    }
  }
}

void insertion::remove_locks(list< lock >& locks)
{
  for (lock& l : locks) {
    for (lock_location& lp : l.locations) {
      remove_stmt(lp.parent, lp.inserted_start);
      remove_stmt(lp.parent, lp.inserted_end);
    }
  }
}


void insertion::apply_reorderings(list< reordering >& reorderings)
{
  for (auto it=reorderings.begin(); it!=reorderings.end();) {
    lock_location& lp = it->location;
    // ignore those that cross iterations
    if (lp.start.iteration == lp.end.iteration && find_parent(lp)) {
      ++it;
    } else {
      it = reorderings.erase(it);
    }    
  }
  merge_reorderings(reorderings);
  for (reordering& r : reorderings) {
    lock_location& lp = r.location;
    move_down(lp.parent, lp.starts, lp.ends);
    //move_down_thread(lp.start->symbol.thread_id, lp);
    //common_parent->dumpPretty(program.ast_context);
  }
}


Decl* insertion::find_declaration(string name)
{
  IdentifierInfo& ii = program.ast_context.Idents.get(name);
  DeclarationName dn = program.ast_context.DeclarationNames.getIdentifier(&ii);
  DeclContext::lookup_result lr = program.translation_unit->lookup(dn);
  assert(lr.size()>0);
  return lr.front();
}

VarDecl* insertion::add_declaration(string name, string type)
{
  IdentifierInfo& ii = program.ast_context.Idents.getOwn(name);
  Decl* typedefd = find_declaration(type);
  TypeDecl* td = cast<TypeDecl>(typedefd);
  QualType qt(td->getTypeForDecl(), 0);
  TypeSourceInfo* tsi = program.ast_context.getTrivialTypeSourceInfo(qt);
  VarDecl* vd = VarDecl::Create(program.ast_context, program.translation_unit, SourceLocation(), SourceLocation(), &ii, qt, tsi , SC_None);
  program.translation_unit->addDecl(vd);
  return vd;
}

Stmt* insertion::insert_call(string function, VarDecl* var_decl, CompoundStmt* parent, bool after, Stmt* ref_point)
{
  Decl* d = find_declaration(function);
  FunctionDecl* fd = cast<FunctionDecl>(d);
  DeclRefExpr* fun = DeclRefExpr::Create(program.ast_context, NestedNameSpecifierLoc(), SourceLocation(), fd, false, SourceLocation(),
                                         fd->getType(), VK_LValue
  );
  DeclRefExpr* var = DeclRefExpr::Create(program.ast_context, NestedNameSpecifierLoc(), SourceLocation(), var_decl, false, SourceLocation(),
                                         var_decl->getType(), VK_LValue
  );
  ImplicitCastExpr* func = ImplicitCastExpr::Create(program.ast_context, program.ast_context.getDecayedType(fd->getType()), CK_FunctionToPointerDecay, fun, nullptr, VK_LValue);
  ImplicitCastExpr* varc = ImplicitCastExpr::Create(program.ast_context, var_decl->getType(), clang::CK_LValueToRValue, var, nullptr, VK_RValue);
  CallExpr* ce = new (program.ast_context) CallExpr(program.ast_context, func, { varc }, QualType(), VK_LValue, SourceLocation());
  // insert this into the compoundstmt
  Stmt** array = new Stmt*[parent->size()+1];
  unsigned counter = 0;
  
  for (Stmt* s : parent->body()) {
    if (!after && s==ref_point) {
      array[counter] = ce;
      ++counter;
    }
    array[counter] = s;
    ++counter;
    if (after && s==ref_point) {
      array[counter] = ce;
      ++counter;
    }
  }
  parent->setStmts(program.ast_context, array, parent->size()+1);
  delete[] array;
  return ce;
}

void insertion::remove_stmt(CompoundStmt* parent, Stmt* stmt)
{
  Stmt** array = new Stmt*[parent->size()-1];
  unsigned counter = 0;
  bool found = false;
  for (Stmt* s : parent->body()) {
    if (s!=stmt) {
      array[counter] = s;
      ++counter;
      found = true;
    }
  }
  assert(found);
  parent->setStmts(program.ast_context, array, parent->size()-1);
  delete[] array;
}


void insertion::move_down(CompoundStmt* parent, Stmt* first, Stmt* second)
{
  Stmt** array = new Stmt*[parent->size()];
  unsigned counter = 0;
  
  bool found_first = false;
  bool found_second = false;
  for (Stmt* s : parent->body()) {
    if (s==first) {
      assert(!found_first);
      found_first = true;
    } else if (s==second) {
      assert(!found_second);
      array[counter++] = second;
      array[counter++] = first;
      found_second = true;
    } else {
      array[counter++] = s;
    }
  }
  assert(found_first && found_second);
  parent->setStmts(program.ast_context, array, parent->size());
  delete[] array;
}

bool insertion::find_parent(lock_location& lp)
{
  if (lp.starts) return true; // we already processed this one
  std::unordered_set<Stmt*> locs;
  Stmt* start; Stmt* end;
  CompoundStmt* common_parent;
  call_stack stack1 = lp.start.symbol->cstack;
  call_stack stack2 = lp.end.symbol->cstack;
  if (!find_parent(stack1, stack2, locs, common_parent, start, end))
    return false;
  lp.ends = end;
  lp.starts = start;
  lp.parent = common_parent;
  lp.locations = locs;
  lp.start_stack = stack1;
  lp.end_stack = stack2;
  return true;
}


bool insertion::find_parent(call_stack& stack1, call_stack& stack2, std::unordered_set< clang::Stmt* >& locked_list, clang::CompoundStmt*& common_parent, clang::Stmt*& start, clang::Stmt*& end)
{
  // find common function
  // look which one of stack2 is also in stack1
  stmt_loc loc1;
  stmt_loc loc2;
  bool found = false;
  for (auto it = stack2.rbegin(); it!=stack2.rend(); ++it) {
    auto it2 = find_if(stack1.rbegin(), stack1.rend(), [it](stmt_loc l) {return l.first==it->first;});
    if (it2!=stack1.rend()) {
      found = true;
      loc1 = *it2;
      loc2 = *it;
      break;
    }
  }
  assert (loc1.first==loc2.first); // they are in the same function
  if (!found) 
    return false;
  // remove symbols after this one
  for (auto itr = find(stack1.begin(), stack1.end(), loc1); itr != stack1.end(); itr = stack1.erase(itr)) {}
  for (auto itr = find(stack2.begin(), stack2.end(), loc1); itr != stack2.end(); itr = stack2.erase(itr)) {}
  
  // now find common compound parent
  ParentMap map(loc1.first);
  
  Stmt* below = loc1.second; // statement below the compound parent
  call_stack parents1;
  while(below) {
    Stmt* parent = map.getParent(below);
    if (parent && isa<CompoundStmt>(parent))
      parents1.push_back(make_pair(parent, below));
    below = parent;
  }
  
  below = loc2.second;
  while(below) {
    Stmt* parent = map.getParent(below);
    if (parent && isa<CompoundStmt>(parent)) {
      auto it = find_if(parents1.rbegin(), parents1.rend(), [parent](stmt_loc l) {return l.first==parent;});
      if (it!=parents1.rend()) {
                common_parent = cast<CompoundStmt>(parent);
        start = it->second;
        end = below;
        bool started = false;
        for (Stmt* s : common_parent->body()) {
          if (s == it->second) {
            started = true;
          }
          if (started) {
            locked_list.insert(s);
          }
          if (s == below) {
            if (!started)
              return false;
            started = false;
          }
        }
        assert (started == false);
        assert (!locked_list.empty());
        return true;
      }
    }
    below = parent;
  }
  return false;
}

struct frontier_item {
  state_id state;
  bool previous_locked;
  abstraction::psymbol last_symbol;
  frontier_item(state_id state, bool previous_locked, abstraction::psymbol last_symbol) : state(state), previous_locked(previous_locked), last_symbol(last_symbol) {}
};

void insertion::print_lp(const lock_location& lp, ostream& out) {
  const SourceManager& sm = program.identifiers().source_manager;
  clang::SourceLocation loc = sm.getFileLoc(lp.starts->getLocStart());
  const clang::FileEntry* fileentry = sm.getFileEntryForID(sm.getFileID(loc));
  string filename = string(fileentry->getName()).erase(0, strlen(fileentry->getDir()->getName())+1);
  out << filename << ":" << sm.getPresumedLineNumber(loc);
  loc = sm.getFileLoc(lp.ends->getLocEnd());
  out << "-" << sm.getPresumedLineNumber(loc);
}

void insertion::print_locks(const list< lock >& locks, ostream& out)
{
  for (const lock& l : locks) {
    out << l.name << ": ";
    for (const lock_location lp : l.locations) {
      print_lp(lp, out);
      out << " ";
    }
    out << endl;
  }
}


void insertion::merge_locks(list< lock >& locks)
{
  cout << "Before:" << endl;
  print_locks(locks, cout);
  for (auto it = locks.begin(); it!=locks.end(); ++it) {
    bool overlap = false;
    auto it2 = it;
    for (++it2; it2 != locks.end(); ) {
      for (auto iti = it2->locations.begin(); iti!=it2->locations.end(); ++iti) {
        bool merged = false;
        for (lock_location& l1 : it->locations) {
          print_lp(l1, cout);
          cout << " -- ";
          print_lp(*iti, cout);
          cout << endl;
          if (merge_overlap(l1, *iti)) {
            if (!overlap)
              // copy the previous locations
              it->locations.insert(it->locations.begin(), it2->locations.begin(), iti);
            overlap = true;
            merged = true;
            cout << "--> ";
            print_lp(l1, cout);
            cout << endl;
            break;
          }
        }
        if (!merged && overlap)
          it->locations.push_front(*iti);
      }
      if (overlap) {
        it2 = locks.erase(it2);
      } else 
        ++it2;
    }
  }
  // merge locks inside (in case they default to the same position)
  for (lock& l : locks) {
    for (auto it = l.locations.begin(); it != l.locations.end();) {
      bool found = false;
      for (auto it2 = l.locations.begin(); it2 != it; ++it2) {
        if (merge_overlap(*it, *it2)) {
          it = l.locations.erase(it);
          found = true;
          break;
        }
      }
      if (!found) ++it;
    }
  }
  cout << "After:" << endl;
  print_locks(locks, cout);
}

bool parent_relation(Stmt* stmt, Stmt* parent, call_stack stack, unordered_set<Stmt*> locations) {
  stack.reverse();
  ParentMap map(parent);
  Stmt* below = nullptr;
  while (stmt) {
    if (parent == stmt) {
      // test we have the same set of locations
      if (locations.find(below) != locations.end())
        return true;
      else return false;
    }
    below = stmt;
    stmt = map.getParent(stmt);
    if (!stmt && !stack.empty()) {
      stmt = stack.front().second;
      stack.erase(stack.begin());
    }
  }
  return false;
}

bool insertion::merge_overlap(lock_location& l1, const lock_location& l2)
{
  if (l2.locations.find(l1.starts) != l2.locations.end() || l1.locations.find(l2.starts) != l1.locations.end()) {
    if (l2.locations.find(l1.starts) != l2.locations.end()) {
      // we need to start with l2
      l1.starts = l2.starts;
    }
    if (l2.locations.find(l1.ends) != l2.locations.end()) {
      // we need to end with l2
      l1.ends = l2.ends;
    }
    l1.locations.insert(l2.locations.begin(), l2.locations.end());
    return true;
  }
  // check if l2 inside l1 lock
  if (parent_relation(l2.starts, l1.parent, l2.start_stack, l1.locations))
    return true;
    
  // is l1 in l2
  if (parent_relation(l1.starts, l2.parent, l1.start_stack, l2.locations)) {
    l1 = l2;
    return true;
  }
  ParentMap map2(l2.parent);
  
  return false;
}


void insertion::merge_reorderings(list< reordering >& reorderings)
{
  for (auto it = reorderings.begin(); it!=reorderings.end(); ++it) {
    for (auto it2 = reorderings.begin(); it2 != it; ) {
      if (it->location.starts == it2->location.starts || it->location.ends == it2->location.ends)
        it2 = reorderings.erase(it2);
      else 
        ++it2;
    }
  }
}

