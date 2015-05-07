#include "langinc.h"

lock_t mtx;
lock_t sem;
int state;
int vm_consistent;
lock_t l1;

void thread_mmap() {
    int_lock(sem);
    vm_consistent = 0;
    if (1) {
        int_lock(mtx);
        int_lock(l1);
        state = 3;
        state = 1;
        int_unlock(l1);
        state = 3;
        state = 2;
        int_unlock(mtx);
    }
    vm_consistent = 1;
    int_unlock(sem);
}


void thread_ioctl() {
    int old_state;
    int_lock(mtx);
    old_state = state;
    int_lock(l1);
    state = 3;
    int_unlock(mtx);
    if (nondet_int) {
        int_lock(sem);
        int_unlock(sem);
    }
    ;
    state = old_state;
    int_unlock(l1);
}


