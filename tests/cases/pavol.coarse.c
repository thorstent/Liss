#include "langinc.h"

lock_t synthlock_0;
int x,y,z;

void thread_1() {
lock_s(synthlock_0);
  x = 1;
  x = 1;
  y = 1;
  unlock_s(synthlock_0);
}

void thread_2() {
lock_s(synthlock_0);
  x = 1;
  unlock_s(synthlock_0);
}

void thread_3() {
lock_s(synthlock_0);
  y = 1;
  y = 1;
  unlock_s(synthlock_0);
}