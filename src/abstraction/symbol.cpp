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
#include "options.h"

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


symbol::symbol(op_class operation, string variable_name, variable_type variable, identifier_store& is, clang::Stmt* stmt) :
operation(operation), variable_name(variable_name), variable(variable) {
  clang::SourceLocation loc = is.source_manager.getFileLoc(stmt->getLocStart());
  fileentry = is.source_manager.getFileEntryForID(is.source_manager.getFileID(loc));
  assert (fileentry);
  line_no = is.source_manager.getPresumedLineNumber(loc);
}

symbol::symbol(thread_id_type thread_id, state_id_type state_id, uint8_t branch) : operation(op_class::tag), loc(thread_id, state_id), tag_branch(branch)
{
  
}

symbol::symbol(thread_id_type thread_id, state_id_type state_id) : operation(op_class::epsilon), loc(thread_id, state_id) {
  
}

std::ostream& abstraction::operator<< (std::ostream &out, const abstraction::symbol &val) {
  val.operation;
  out << to_string(val.thread_id()) << "-";
  if (val.tag_branch != -1) {
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


bool symbol::is_preemption_point() const
{
  switch (operation) {
    case abstraction::op_class::read:
    case abstraction::op_class::write:
    case abstraction::op_class::unlock:
    case abstraction::op_class::notify:
    case abstraction::op_class::reset:
    case abstraction::op_class::epsilon:
    case abstraction::op_class::tag:
      break;
    case abstraction::op_class::yield:
      return true;
    case abstraction::op_class::lock:
      return !synthesised;
    case abstraction::op_class::wait_reset:
    case abstraction::op_class::wait_not:
    case abstraction::op_class::wait:
      return !assume || assumes_allow_switch;
  }
  return false;
}
