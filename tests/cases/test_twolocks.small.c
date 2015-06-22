#include "langinc.h"

lock_t synthlock_1;
lock_t synthlock_0;
int x,y,z;

void thread_1() {
  int i;
  lock_s(synthlock_1);
  x = 1;
  lock_s(synthlock_0);
  y = 1;
  unlock_s(synthlock_0);
  x = 1;
  unlock_s(synthlock_1);
}

void thread_2() {
  int i;
  lock_s(synthlock_1);
  x = 1;
unlock_s(synthlock_1);
}

void thread_3() {
  lock_s(synthlock_0);
  y = 1;
  y = 1;
unlock_s(synthlock_0);
}