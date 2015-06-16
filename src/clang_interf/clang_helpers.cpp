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

#include "clang_helpers.h"

#include <clang/AST/Stmt.h>
#include <clang/AST/ParentMap.h>
#include <clang/AST/Expr.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/ASTContext.h>

using namespace clang;

namespace clang_interf {
  bool ends_semicolon(const Stmt* stmt) {
    if (isa<IfStmt>(stmt)) 
      return false;
    if (isa<WhileStmt>(stmt)) 
      return false;
    return true;
  }
  
  parent_result find_stmt_parent(Stmt* stmt, Stmt* function)
  {
    ParentMap map(function);
    while (true) {
      assert(stmt);
      Stmt* parent = map.getParent(stmt);
      if (isa<CompoundStmt>(parent)) {
        return parent_result(stmt, false);
      } else if (IfStmt* ifs = dyn_cast<IfStmt>(parent)) {
        if (stmt!=ifs->getCond())
          return parent_result(stmt, true);
        else
          return parent_result();
      } else if (WhileStmt* whiles = dyn_cast<WhileStmt>(parent)) {
        if (stmt!=whiles->getCond())
          return parent_result(stmt, true);
        else 
          return parent_result();
      } else if (DoStmt* dos = dyn_cast<DoStmt>(parent)) {
        if (stmt!=dos->getCond()) 
          return parent_result(stmt, true);
        else 
          return parent_result();
      } else if (isa<ReturnStmt>(parent)) {
        return parent_result();
      } else if (isa<LabelStmt>(parent)) {
        return parent_result(stmt, false);
      }
      stmt = parent;
    }
  }


// Part below taken from LLVM/Clang
//===--- Transforms.cpp - Transformations to ARC mode ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/// \brief 'Loc' is the end of a statement range. This returns the location
/// immediately after the semicolon following the statement.
/// If no semicolon is found or the location is inside a macro, the returned
/// source location will be invalid.
SourceLocation findLocationAfterSemi(SourceLocation loc,
                                            ASTContext &Ctx, bool IsDecl) {
  SourceLocation SemiLoc = findSemiAfterLocation(loc, Ctx, IsDecl);
  if (SemiLoc.isInvalid())
    return SourceLocation();
  return SemiLoc.getLocWithOffset(1);
}

/// \brief \arg Loc is the end of a statement range. This returns the location
/// of the semicolon following the statement.
/// If no semicolon is found or the location is inside a macro, the returned
/// source location will be invalid.
SourceLocation findSemiAfterLocation(SourceLocation loc,
                                            ASTContext &Ctx,
                                            bool IsDecl) {
  SourceManager &SM = Ctx.getSourceManager();
  if (loc.isMacroID()) {
    if (!Lexer::isAtEndOfMacroExpansion(loc, SM, Ctx.getLangOpts(), &loc))
      return SourceLocation();
  }
  loc = Lexer::getLocForEndOfToken(loc, /*Offset=*/0, SM, Ctx.getLangOpts());

  // Break down the source location.
  std::pair<FileID, unsigned> locInfo = SM.getDecomposedLoc(loc);

  // Try to load the file buffer.
  bool invalidTemp = false;
  StringRef file = SM.getBufferData(locInfo.first, &invalidTemp);
  if (invalidTemp)
    return SourceLocation();

  const char *tokenBegin = file.data() + locInfo.second;

  // Lex from the start of the given location.
  Lexer lexer(SM.getLocForStartOfFile(locInfo.first),
              Ctx.getLangOpts(),
              file.begin(), tokenBegin, file.end());
  Token tok;
  lexer.LexFromRawLexer(tok);
  if (tok.isNot(tok::semi)) {
    if (!IsDecl)
      return SourceLocation();
    // Declaration may be followed with other tokens; such as an __attribute,
    // before ending with a semicolon.
    return findSemiAfterLocation(tok.getLocation(), Ctx, /*IsDecl*/true);
  }

  return tok.getLocation();
}

}