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

#ifndef SYNTHESIS_CONSTRAINT_H
#define SYNTHESIS_CONSTRAINT_H

#include <list>
#include "location.h"
#include <z3++.h>
#include <vector>
#include <ostream>
#include "types.h"

namespace synthesis {
  
struct constraint_atom {
  location before;
  location after;
  bool from_wait_notify; // this is an artifical constraint from a wait-notify in the code
  constraint_atom(location before, location after, bool from_wait_notify = false) : before(before), after(after), from_wait_notify(from_wait_notify) {}
  operator z3::expr () const { return static_cast<z3::expr>(before) < after; }
};

std::ostream& operator<<(std::ostream& out, const constraint_atom& ca);


using cnf_constr = cnf<constraint_atom>;
using dnf_constr = dnf<constraint_atom>;
using disj_constr = disj<constraint_atom>;
using conj_constr = conj<constraint_atom>;

z3::expr make_constraint(z3::context& ctx, conj_constr c);
z3::expr make_constraint(z3::context& ctx, dnf_constr d);

cnf_constr negate_dnf(const dnf_constr& dnf);

}

#endif 