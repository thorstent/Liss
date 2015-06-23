#include "langinc.h"

int x,y,z,a;


void thread_1() {
  int i;
  while(nondet) {
    x = 1;
  }
  y = 1;
}

void thread_2() {
  int i;
  while(nondet) {
    z = 1;
  }
}