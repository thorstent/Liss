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

#include "options.h"

#include <iostream>
#include <boost/filesystem.hpp>

int verbosity = 0;
unsigned max_bound = 20;
std::ostream& debug = std::cout;
std::string debug_folder;
std::string main_filename;
std::string main_file_path;
std::string start_file_code;
std::string output_file_log;

void create_debug_folder()
{
  boost::filesystem::create_directory(debug_folder);
}

std::string output_file_code(std::string strategy_name) {
  std::string output_file_code = main_file_path;
  output_file_code.replace(output_file_code.length()-2,2, "." + strategy_name + ".c");
  return output_file_code;
}