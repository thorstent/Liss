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

#include "constraint.h"

using namespace synthesis;

z3::expr synthesis::make_constraint(z3::context& ctx, conj c) {
  z3::expr result = ctx.bool_val(true);
  for (const constraint_atom& l : c) {
    result = result && l;
  }
  return result;
}

z3::expr synthesis::make_constraint(z3::context& ctx, dnf d) {
  z3::expr result = ctx.bool_val(false);
  for (const conj& l : d) {
    result = result || make_constraint(ctx, l);
  }
  return result;
}

void print_constraint(const conj& c, const Limi::printer< abstraction::psymbol >& symbol_printer, std::ostream& out, std::string seperator)
{
  if (c.size()==0)
    out << "true";
  for (unsigned i = 0; i < c.size(); ++i) {
    print_location(c[i].before, symbol_printer, out);
    out << " < ";
    print_location(c[i].after, symbol_printer, out);
    if (i < c.size() - 1) 
      out << " " << seperator << " ";
  }
}

void synthesis::print_constraint(const conj& c, const Limi::printer< abstraction::psymbol >& symbol_printer, std::ostream& out)
{
  ::print_constraint(c, symbol_printer, out, "/\\");
}

void synthesis::print_constraint_cnf(const disj& d, const Limi::printer< abstraction::psymbol >& symbol_printer, std::ostream& out)
{
  ::print_constraint(d, symbol_printer, out, "\\/");
}

void synthesis::print_constraint(const dnf& d, const Limi::printer< abstraction::psymbol >& symbol_printer, std::ostream& out)
{
  if (d.size()==0)
    out << "false";
  for (unsigned i = 0; i < d.size(); ++i) {
    print_constraint(d[i], symbol_printer, out);
    if (i < d.size() - 1) 
      out << " \\/" << std::endl;
  }
}


cnf synthesis::negate_dnf(const dnf& dnf)
{
  cnf cnf(dnf);
  for(conj& c : cnf) {
    for(constraint_atom& ca : c) {
      auto temp = ca.after;
      ca.after = ca.before;
      ca.before = temp;
    }
  }
  return cnf;
}
