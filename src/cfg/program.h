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

#ifndef CFG_PROGRAM_H
#define CFG_PROGRAM_H

#include<unordered_map>
#include "abstract_cfg.h"
#include <vector>
#include <string>
#include "abstraction/identifier_store.h"
#include "types.h"
#include <clang/Basic/SourceLocation.h>

namespace clang {
  class TranslationUnitDecl;
  class ASTContext;
}

namespace cfg {
  
  class program
  {
  private:
    std::vector<const abstract_cfg*> threads_;
    std::vector<const abstract_cfg*> minimised_threads_;
    abstraction::identifier_store identifier_store_;
  public:
    clang::TranslationUnitDecl* translation_unit;
    clang::SourceLocation first_function; // needed to put stuff in the beginning
    clang::ASTContext& ast_context;
    
    program(clang::ASTContext& context);
    program(const program& p);
    program & operator= (const program & other) = delete;
    ~program();
    
    void add_thread(abstract_cfg* thread);
    void clear_threads();
    
    inline const std::vector<const abstract_cfg*>& threads() const { return threads_;}
    inline const std::vector<const abstract_cfg*>& minimised_threads() const { return minimised_threads_;}
    //inline std::vector<abstract_cfg*>& threads() { return threads_;}
    inline unsigned no_threads() const { return threads_.size(); }
    inline const abstraction::identifier_store& identifiers() const { return identifier_store_; }
    inline abstraction::identifier_store& identifiers() { return identifier_store_; }
    
    static bool is_thread_name(std::string name);
  };
}

#endif // CFG_PROGRAM_H

