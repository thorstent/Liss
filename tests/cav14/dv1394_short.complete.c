#include <langinc.h>

lock_t synthlock_1;
lock_t mtx;
lock_t sem;

int state;
int vm_consistent;

void thread_mmap () {
    lock(sem);
    vm_consistent = 0;

    lock(mtx);
    lock_s(synthlock_1);
    state = 3;
    state = 1;
    state = 3;
    state = 2;
    unlock_s(synthlock_1);
    unlock(mtx);
	
    vm_consistent = 1;
    unlock(sem);
}

void thread_ioctl () {
    int old_state;

    lock(mtx);
    old_state = state;
    lock_s(synthlock_1);
    state = 3;
    unlock(mtx);
    
    // preemption point (because nondet)
    if (nondet) {
        unlock_s(synthlock_1);
        lock(sem);
        unlock(sem);
        lock_s(synthlock_1);
    };

    state = old_state; // critical point
unlock_s(synthlock_1);
}