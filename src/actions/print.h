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

#ifndef ACTIONS_PRINT_H
#define ACTIONS_PRINT_H

#include "action_base.h"

namespace actions {

void print_program(const cfg::program& program, bool timbuk = false, bool only_threads = false, std::string prefix = std::string());
  
class print : public actions::action_base
{
public:
  bool print_timbuk = false;
  bool only_threads = false;
  print();
  virtual void run(const cfg::program& program, clang::CompilerInstance& compiler) override;
};
}

#endif // ACTIONS_PRINT_H
