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

#include "print_cfg.h"
#include "cfg/dot_printer.h"
#include "options.h"

using namespace actions;

void print_cfg::run(const cfg::program& program, clang::CompilerInstance& compiler)
{
  for (const cfg::abstract_cfg* thread : program.threads()) {
    cfg::print_dot(*thread, debug_folder + thread->name + "_cfg.dot");
  }
  for (const cfg::abstract_cfg* thread : program.minimised_threads()) {
    cfg::print_dot(*thread, debug_folder + thread->name + "_min_cfg.dot");
  }
}
