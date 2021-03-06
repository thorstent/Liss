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

#ifndef CFG_DOT_PRINTER_H
#define CFG_DOT_PRINTER_H

#include "abstract_cfg.h"

namespace cfg {
  
  void print_dot(const abstract_cfg& cfg, std::ostream& out);
  void print_dot(const abstract_cfg& cfg, const std::string& filename);

}

#endif // CFG_DOT_PRINTER_H
