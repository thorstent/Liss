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

#ifndef CLANG_INTERF_PARSE_ERROR_H
#define CLANG_INTERF_PARSE_ERROR_H

#include <stdexcept>
#include <string>

class parse_error : public std::runtime_error {
public:
  explicit parse_error (const std::string& what_arg) : runtime_error(what_arg) {}
  explicit parse_error (const char* what_arg) : runtime_error(what_arg) {}
};

#endif
