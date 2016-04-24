#include "langinc.h"

int x,y,z;

void thread_1() {
  int i;
  x = 1;
  if (i == 1) {
    y = 1;
  }
  x = 1;
}

void thread_2() {
  int i;
  i = x;
}