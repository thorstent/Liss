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

#include "action_base.h"

#include "print.h"
#include "inclusion_test.h"
#include "synthesis.h"
#include "deadlock_check.h"
#include "perf_test.h"
#include "print_cfg.h"

using namespace actions;
using namespace std;

actions::actionp actions::create_action(action_names action){
  switch (action) {
    case action_names::print:
      return make_shared<print>();
    case action_names::printtim: {
      auto printc = make_shared<print>();
      printc->print_timbuk = true;
      return printc;}
    case action_names::printthreads: {
      auto printc = make_shared<print>();
      printc->only_threads = true;
      return printc;}
    case action_names::inclusion_test:
      return make_shared<inclusion_test>();
    case action_names::synthesis:
      return make_shared<synthesis>();
    case action_names::deadlock:
      return make_shared<deadlock_check>();
    case action_names::perf_test:
      return make_shared<perf_test>();
    case action_names::printcfg:
      return make_shared<print_cfg>();
  }
  assert(false);
  return nullptr;
}
