#include "langinc.h"

int x,y,z;

conditional_t c;

void thread_1() {
  int i;
  while (nondet) {
    x = 1;
  }
}

void thread_2() {
  int i;
  wait(c);
}