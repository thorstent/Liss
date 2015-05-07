#include "automata/antichain_algo.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <cassert>
#include <iostream>

using namespace std;
using namespace automata;

const unsigned threads = 4;
const unsigned instructions = 12;

class TestState {
private:
  mutable size_t hash_ = 0;
public:
  
  int thread_locs[threads];
  
  bool equals(const TestState& other) const {
    const TestState& stat = static_cast<const TestState&>(other);
    if (hash() != stat.hash())
      return false;
    for (unsigned i = 0; i<threads; i++) {
      if (thread_locs[i] != stat.thread_locs[i]) {
        return false;
      }
    }
    return true;
  }
  
  size_t hash() const {
    if (hash_!=0) return hash_;
    for (unsigned i = 0; i<threads; i++) {
      hash_ = hash_ ^ thread_locs[i];
      hash_ = hash_ << 3;
    }
    return hash_;
  }
  
  TestState() {
    for (unsigned i = 0; i<threads; i++) {
      thread_locs[i] = 0;
    }
  }
  
  TestState(const TestState& old) {
    for (unsigned i = 0; i<threads; i++) {
      thread_locs[i] = old.thread_locs[i];
    }
  }
  
  std::string to_string() const {
    std::stringstream out;
    out << "( ";
    for (unsigned i = 0; i<threads; i++) {
      out << std::to_string(thread_locs[i]);
      if (i<threads-1)
        out << ", ";
    }
    out << " )";
    return out.str();
  }
};


class TestSymbol {
public:
  int id;
  TestSymbol(int id) : id(id) {}
  std::string to_string() const {
    return std::to_string(id);
  }
  size_t hash() const {
    return this->id;
  }
  bool equals(const TestSymbol& other) const {
    return this->id == (static_cast<const TestSymbol&>(other)).id;
  }
};

typedef std::shared_ptr<const TestSymbol> csymbol;
typedef std::shared_ptr<const TestState> cstate;


namespace std {
  template<> struct hash<::cstate> {
    size_t operator()(const cstate& val) const {
      return val->hash();
    }
  };
  template<> struct equal_to<::cstate> {
    size_t operator()(const cstate& a, const cstate& b) const {
      return a->equals(*b);
    }
  };
  
  template<> struct hash<::csymbol> {
    size_t operator()(const csymbol& val) const {
      return val->hash();
    }
  };
  template<> struct equal_to<::csymbol> {
    size_t operator()(const csymbol& a, const csymbol& b) const {
      return a->equals(*b);
    }
  };
}

class TestAutomaton : public automaton<cstate,csymbol,TestAutomaton> {
public:
  inline bool is_final_state(const cstate& state) const {
    return true;
  }
  
  State_set initial_states() const {
    State_set result;
    result.insert(make_shared<TestState>());
    return result;
  }
  
  State_set successors(const cstate& state, const csymbol& sigma) const {
    const TestSymbol& symbol = static_cast<const TestSymbol&>(*sigma);
    const TestState& stat = static_cast<const TestState&>(*state);
    State_set result;
    int i = symbol.id;
    shared_ptr<TestState> newstate = make_shared<TestState>(stat); // copy constructor
    newstate->thread_locs[i] = (stat.thread_locs[i] + 1) % instructions;
    result.insert(newstate);
    //cout << newstate->to_string() << endl;
    return result;
  }
  
  Symbol_list next_symbols(const cstate& state) const {
    Symbol_list result;
    // we can advance any one thread
    for (unsigned i = 0; i<threads; i++) {
      result.push_back(make_shared<TestSymbol>(i));
    }
    return result;
  }
};

class BState {
public:
  
  
  inline bool equals(const BState& other) const {
    return true;
  }
  
  inline  size_t hash() const {
    return 1;
  }
  
  BState() {
  }
  
  std::string to_string() const {
    return std::string("B");
  }
};

typedef std::shared_ptr<const BState> cbstate;

namespace std {
  template<> struct hash<::cbstate> {
    size_t operator()(const cbstate& val) const {
      return val->hash();
    }
  };
  template<> struct equal_to<::cbstate> {
    size_t operator()(const cbstate& a, const cbstate& b) const {
      return a->equals(*b);
    }
  };
}

class TestAutomatonB : public automaton<cbstate,csymbol,TestAutomatonB> {
public:
  inline bool is_final_state(const cbstate& state) const {
    return true;
  }
  
  inline State_set initial_states() const {
    State_set result;
    result.insert(make_shared<BState>());
    return result;
  }
  
  inline State_set successors(const cbstate& state, const csymbol& sigma) const {
    State_set result;
    result.insert(state);
    return result;
  }
  
  inline Symbol_list next_symbols(const cbstate& state) const {
    Symbol_list result;
    assert(false);
    return result;
  }
};

int main() {
  TestAutomaton a;
  TestAutomatonB b;
  
  antichain_algo<cstate,cbstate,csymbol,TestAutomaton,TestAutomatonB> algo;
  
  cout << algo.test_inclusion(a,b) << endl;
}