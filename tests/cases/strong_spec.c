#include "langinc.h"

int x,y,z,a;

void common() {
    x = 1;
  y = 1;
}

void thread_1() {
  common();
}

void thread_2() {
  common();
}