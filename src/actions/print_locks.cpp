/*
 * Copyright 2016, IST Austria
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

#include "print_locks.h"
#include "placement/print_program.h"
#include "options.h"

using namespace actions;

void print_locks::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  create_debug_folder();
  placement::print_program printer(program);
  printer.print_original(debug_folder + "program_orig.c");
  printer.print_test_locations(debug_folder + "program_locks.c");
}
