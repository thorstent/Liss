#include "langinc.h"

int x,y,z;

void thread_1() {
  x = 1;
  x = 1;
  y = 1;
}

void thread_2() {
  x = 1;
}

void thread_3() {
  y = 1;
  y = 1;
}