#include <langinc.h>

lock_t mtx;
lock_t sem;

int state;
int vm_consistent;

void thread_mmap () {
    lock(sem);
    vm_consistent = 0;

    lock(mtx);
    state = 3;
    state = 1;
    state = 3;
    state = 2;
    unlock(mtx);
	
    vm_consistent = 1;
    unlock(sem);
}

void thread_ioctl () {
    int old_state;

    lock(mtx);
    old_state = state;
    state = 3;
    unlock(mtx);
    
    // preemption point (because nondet)
    if (nondet) {
        lock(sem);
        unlock(sem);
    };

    state = old_state; // critical point
}