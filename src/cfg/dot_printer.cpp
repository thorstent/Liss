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

#include "dot_printer.h"
#include <fstream>
#include <unordered_set>
#include <deque>

using namespace cfg;

void cfg::print_dot(const abstract_cfg& cfg, std::ostream& out)
{
  Limi::printer<state_id_type> printer(cfg);
  out << "digraph automaton {" << std::endl;
  std::unordered_set<state_id_type> seen;
  std::deque<state_id_type> frontier;
  for (const state_id_type s : cfg.initial_states()) {
    frontier.push_back(s);
    out << "begin" << s << " [shape=none,label=\"\"]" << std::endl;
    out << "begin" << s << " -> " << s << std::endl;
  }
  
  while (!frontier.empty()) {
    state_id_type next = frontier.front();
    frontier.pop_front();
    if (seen.find(next) == seen.end()) {
      seen.insert(next);
      
      out << std::to_string(next) << " [shape=box,label=\"" << printer(next) << "\"";
      out << "]" << std::endl;
      for (const edge& edge : cfg.get_successors(next)) {
        frontier.push_back(edge.to);
        out << std::to_string(next) << " -> " << std::to_string(edge.to) << " [label=\"";
        out << edge;
        out << "\"";
        if (edge.back_edge)
          out << ",style=dashed";
        out << "]" << std::endl;
      }
    }
  }
  
  out << "}" << std::endl;
}

void cfg::print_dot(const abstract_cfg& cfg, const std::string& filename)
{
  std::ofstream myfile;
  myfile.open(filename);
  print_dot(cfg, myfile);
  myfile.close();
}
