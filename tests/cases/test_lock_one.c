#include "langinc.h"

int x,y,z;

void thread_1() {
  int i;
  i = x + x;
}

void thread_2() {
  int i;
  x = 1;
}