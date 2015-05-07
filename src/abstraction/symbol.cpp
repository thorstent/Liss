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

#include "symbol.h"
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace abstraction;
using namespace std;

std::ostream& abstraction::operator<< (std::ostream& out, const abstraction::op_class& op) {
  switch (op) {
    case abstraction::op_class::read:
      out << "r";
      break;
    case abstraction::op_class::write:
      out << "w";
      break;
    case abstraction::op_class::lock:
      out << "lo";
      break;
    case abstraction::op_class::unlock:
      out << "un";
      break;
    case abstraction::op_class::notify:
      out << "no";
      break;
    case abstraction::op_class::wait:
      out << "wa";
      break;
    case abstraction::op_class::wait_not:
      out << "wn";
      break;
    case abstraction::op_class::wait_reset:
      out << "wr";
      break;
    case abstraction::op_class::reset:
      out << "rs";
      break;
    case abstraction::op_class::yield:
      out << "yi";
      break;
    case abstraction::op_class::epsilon:
      out << "eps";
      break;
    case abstraction::op_class::tag:
      out << "t";
      break;
  }
  return out;
}


symbol::symbol(op_class operation, call_stack cstack, string variable_name, variable_type variable, identifier_store& is, const clang::Stmt* stmt) :
operation(operation), cstack(cstack), variable_name(variable_name), variable(variable), stmt(stmt) {
  assert(cstack.back().second);
  assert(cstack.back().second==stmt);
  clang::SourceLocation loc = is.source_manager.getFileLoc(instr_id()->getLocStart());
  fileentry = is.source_manager.getFileEntryForID(is.source_manager.getFileID(loc));
  if (!fileentry) synthesised = true;
  line_no = is.source_manager.getPresumedLineNumber(loc);
}

std::ostream& abstraction::operator<< (std::ostream &out, const abstraction::symbol &val) {
  val.operation;
  if (val.tag_state != no_state) {
    out << to_string(val.tag_branch);
  } else {
    out << val.operation;
    if (!val.variable_name.empty())
      out << "(" << val.variable_name << ")";
    if (val.fileentry) {
      out << "[";
      string filename = string(val.fileentry->getName()).erase(0, strlen(val.fileentry->getDir()->getName())+1);
      out << filename << ":" << val.line_no;
      out << "]";
    }
  }
  return out;
}

symbol::symbol(state_id state, uint8_t branch) : operation(op_class::tag), tag_state(state), tag_branch(branch)
{
  
}
