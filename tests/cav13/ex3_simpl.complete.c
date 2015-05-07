#include "langinc.h"

conditional_t napi_poll;
int shutdown = 0;
lock_t l1;

void thread_1() {
    int_lock(l1);
    shutdown = 1;
    int_notify(napi_poll);
    int_unlock(l1);
}


void thread_3() {
    int_lock(l1);
    int_wait_not(napi_poll);
    int x;
    x = shutdown;
    int_unlock(l1);
}


