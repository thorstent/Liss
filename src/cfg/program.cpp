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

#include "program.h"
#include <stdexcept>
#include <clang/AST/ASTContext.h>

using namespace cfg;
using namespace std;

const std::string thread_prefix = std::string("thread_");
const std::string thread_suffix = std::string("_thread");


void program::add_thread(abstract_cfg* thread)
{
  if (threads_.size() == max_threads)
    throw range_error("Too many threads in program");
  thread->minimise(true);
  thread->compact();
  threads_.push_back(thread);
  abstract_cfg* new_thread = new abstract_cfg(*thread, true);
  new_thread->minimise(false);
  //new_thread->add_tags();
  minimised_threads_.push_back(new_thread);
}

program::~program()
{
  for (const abstract_cfg* thread : threads_) {
    delete thread;
  }
  for (const abstract_cfg* thread : minimised_threads_) {
    delete thread;
  }
}

void program::clear_threads()
{
  for (const abstract_cfg* thread : threads_) {
    delete thread;
  }
  for (const abstract_cfg* thread : minimised_threads_) {
    delete thread;
  }
  threads_.clear();
  minimised_threads_.clear();
}


program::program(const program& other) : 
identifier_store_(other.identifier_store_), translation_unit(other.translation_unit), first_function(other.first_function), ast_context(other.ast_context)
{
  for (const abstract_cfg* t : other.threads_) {
    abstract_cfg* new_thread = new abstract_cfg(*t);
    threads_.push_back(new_thread);
  }
  for (const abstract_cfg* t : other.minimised_threads_) {
    abstract_cfg* new_thread = new abstract_cfg(*t);
    minimised_threads_.push_back(new_thread);
  }
}


bool program::is_thread_name(string name)
{
  return name.length() > thread_suffix.length() &&
  (name.compare(0, thread_prefix.length(), thread_prefix) == 0 ||
  name.compare(name.length() - thread_suffix.length(), thread_suffix.length(), thread_suffix) == 0);
}

program::program(clang::ASTContext& context) : identifier_store_(context.getSourceManager()),
translation_unit(context.getTranslationUnitDecl()), ast_context(context)
{
  
}