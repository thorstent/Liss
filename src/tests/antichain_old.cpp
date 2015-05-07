#include "automata/antichain_algo.h"
#include <iostream>
#include <memory>

#define MAX 10

using namespace std;
using namespace automata;

class TestState : public state {
public:
  int id;
  virtual bool equals(const state& other) const override {
    return this->id == (static_cast<const TestState&>(other)).id;
  }
  virtual size_t hash() const override {
    return this->id;
  }
  TestState(int id) : id(id) {}
  virtual std::string to_string() const override {
    return std::to_string(id);
  }
  
};

class TestSymbol : public symbol {
public:
  int id;
  TestSymbol(int id) : id(id) {}
  virtual std::string to_string() const override {
    return std::to_string(id);
  }
  virtual size_t hash() const override {
    return this->id;
  }
  virtual bool equals(const state& other) const override {
    return this->id == (static_cast<const TestState&>(other)).id;
  }
};

class TestAutomaton : public automaton {
public:
  virtual bool is_final_state(const cstate& state) const override {
    return true;
  }
  
  virtual cstate_set initial_states() const override {
    cstate_set result;
    result.insert(make_shared<TestState>(1));
    return result;
  }
  
  virtual cstate_set successors(const cstate& state, const csymbol& sigma) const override {
    const TestSymbol& symbol = static_cast<const TestSymbol&>(*sigma);
    const TestState& stat = static_cast<const TestState&>(*state);
    cstate_set result;
    if (symbol.id == stat.id || symbol.id == stat.id+1)
      result.insert(make_shared<TestState>(symbol.id+stat.id));
    return result;
  }
  
  virtual csymbol_list next_symbols(const cstate& state) const override {
    const TestState& stat = static_cast<const TestState&>(*state);
    csymbol_list result;
    if (stat.id < MAX/2) {
      result.push_back(make_shared<TestSymbol>(stat.id));
      result.push_back(make_shared<TestSymbol>(stat.id+1));
    }
    return result;
  }
};

class TestAutomaton2 : public automaton {
public:
  virtual bool is_final_state(const cstate& state) const override {
    return true;
  }
  
  virtual cstate_set initial_states() const override {
    cstate_set result;
    result.insert(make_shared<TestState>(1));
    return result;
  }
  
  virtual cstate_set successors(const cstate& state, const csymbol& sigma) const override {
    const TestSymbol& symbol = static_cast<const TestSymbol&>(*sigma);
    const TestState& stat = static_cast<const TestState&>(*state);
    cstate_set result;
    if (symbol.id == stat.id || symbol.id == stat.id+1 || symbol.id == stat.id+2)
      result.insert(make_shared<TestState>(symbol.id+stat.id));
    return result;
  }
  
  virtual csymbol_list next_symbols(const cstate& state) const override {
    const TestState& stat = static_cast<const TestState&>(*state);
    csymbol_list result;
    if (stat.id < MAX/2) {
      result.push_back(make_shared<TestSymbol>(stat.id));
      result.push_back(make_shared<TestSymbol>(stat.id+1));
      result.push_back(make_shared<TestSymbol>(stat.id+2));
    }
    return result;
  }
};

int main() {
  TestAutomaton a;
  TestAutomaton2 b;
  
  antichain_algo algo;
  
  cout << algo.test_inclusion(a,b) << endl;
  cout << algo.test_inclusion(b,a) << endl;
}