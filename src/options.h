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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <ostream>

extern int verbosity;
extern unsigned lock_limit; // a limit on how many locks to synthesise (performance impact)
extern unsigned max_bound; // bound for the antichain algorithm
extern std::ostream& debug;
extern std::string debug_folder;
std::string output_file_code(std::string strategy_name);
extern std::string start_file_code;
extern std::string main_file_path;
extern std::string output_file_log;
extern std::string main_filename;
constexpr bool assumes_allow_switch = false;

constexpr bool condyield_is_always_yield = true;

void create_debug_folder();

#endif // OPTIONS_H
