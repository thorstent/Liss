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

#include "timbuk/parsed_automaton.h"
#include "timbuk/automaton.h"
#include "automata/antichain_algo.h"
#include "options.h"
#include "automata/dot_printer.h"

#include <chrono>

#include <string>
#include <iomanip>

using namespace std;

int main(int argc, const char **argv) {
  string filename(argv[1]);
  string filename2(argv[2]);
  timbuk::symbol_table st;
  debug << "Parsing" << endl;
  timbuk::parsed_automaton aut(st,filename);
  timbuk::parsed_automaton aut2(st,filename2);
  
  timbuk::automaton auti(aut);
  timbuk::automaton auti2(aut2);
  
  
  ofstream out("automaton.dot");
  //automata::print_dot(auti, out);
  out.close();
  
  
  verbosity=2;
  debug << "Language inclusion check..." << endl;
  
  auto start = chrono::steady_clock::now();
  
  datastructures::independence< timbuk::symbol > independence(st);
  automata::antichain_algo<timbuk::state,timbuk::state,timbuk::symbol,timbuk::automaton,timbuk::automaton> algo(auti, auti2, 6, independence);
  automata::inclusion_result<timbuk::symbol> result = algo.run();
  auto stop = chrono::steady_clock::now();
    
  result.print_long(cout, auti.symbol_printer());
  
  chrono::milliseconds passed = std::chrono::duration_cast<chrono::milliseconds>(stop - start);
  cout << "TIME: " << std::setprecision(3) << std::fixed << (double)passed.count()/1000 << " s" << endl;
}