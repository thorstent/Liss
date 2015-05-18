#ifndef _LANGINC_H_
#define _LANGINC_H_

typedef int lock_t;
typedef int conditional_t;

void int_yield();
void int_lock(lock_t);
void int_unlock(lock_t);
void int_notify(conditional_t);
void int_wait(conditional_t);
void int_wait_not(conditional_t);
// assumes are different from waits in that they are not reported as deadlocks
void int_assume(conditional_t);
void int_assume_not(conditional_t);
void int_reset(conditional_t);
void int_wait_reset(conditional_t);

int nondet_int;

#define yield int_yield()
#define lock(X) int_lock(X)
#define lock_s(X) int_lock(X)
#define unlock(X) int_unlock(X)
#define unlock_s(X) int_unlock(X)
#define notify(X) int_notify(X)
#define wait(X) int_wait(X)
#define assume(X) int_assume(X)
#define wait_not(X) int_wait_not(X)
#define assume_not(X) int_assume_not(X)
#define reset(X) int_reset(X)
#define wait_reset(X) int_wait_reset(X)

#define nondet nondet_int

#endif // _LANGINC_H_
