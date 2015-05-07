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

#ifndef ACTIONS_PERF_TEST_H
#define ACTIONS_PERF_TEST_H

#include "action_base.h"

namespace actions {

class perf_test : public actions::action_base
{
private:
  double run_time(const cfg::program& program, bool concurrent, bool epsilon_removal);
  double run_time_old(const cfg::program& program, bool concurrent, bool epsilon_removal);
public:
    virtual void run(const cfg::program& program, clang::CompilerInstance& compiler);
};
}

#endif // ACTIONS_PERF_TEST_H
