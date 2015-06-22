#include "langinc.h"

lock_t synthlock_2;
int x,y,z;

void thread_1() {
  int i;
  lock_s(synthlock_2);
  x = 1;
  y = 1;
  x = 1;
  unlock_s(synthlock_2);
}

void thread_2() {
  int i;
  lock_s(synthlock_2);
  x = 1;
  unlock_s(synthlock_2);
}

void thread_3() {
  lock_s(synthlock_2);
  y = 1;
  y = 1;
  unlock_s(synthlock_2);
}