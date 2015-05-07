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

#ifndef CLANG_INTERF_ABSTRACTION_CONSUMER_H
#define CLANG_INTERF_ABSTRACTION_CONSUMER_H

#include "pch.h"
#include "thread_visitor.h"
#include "actions/action_base.h"

namespace clang_interf {
  
  class abstraction_consumer : public clang::ASTConsumer {
  public:
    explicit abstraction_consumer(clang::CompilerInstance& compiler, std::vector<actions::actionp> actions)
    : actions(actions), compiler(compiler) {}
    
    virtual void HandleTranslationUnit(clang::ASTContext &context);
  private:
    std::vector<actions::actionp> actions;
    
    clang::CompilerInstance& compiler;  
  };
  
}

#endif // CLANG_INTERF_ABSTRACTION_CONSUMER_H
