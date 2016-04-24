#ifndef _LANGINC_H_
#define _LANGINC_H_

typedef int lock_t;
typedef int conditional_t;

void yield();
void lock(lock_t);
void unlock(lock_t);
void lock_s(lock_t);
void unlock_s(lock_t);
void notify(conditional_t);
void wait(conditional_t);
void wait_not(conditional_t);
// assumes are different from waits in that they are not reported as deadlocks
void assume(conditional_t);
void assume_not(conditional_t);
void reset(conditional_t);
void wait_reset(conditional_t);

int nondet;

int dummy_var;

inline void dummy_() {
  int i;
  i = dummy_var;
}


#endif // _LANGINC_H_
