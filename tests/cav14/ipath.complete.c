#include "langinc.h"

void *ipath_pd;
lock_t l;
int block;
lock_t l1;

void thread_irq() {
    void *x;
    int_lock(l);
    int_lock(l1);
    if (ipath_pd) {
        x = ipath_pd;
    }
    int_unlock(l1);
    ;
    int_unlock(l);
}


void thread_user() {
    void *tmp;
    int_lock(l);
    tmp = ipath_pd;
    int_unlock(l);
    if (tmp) {
        block = 1;
        block = 0;
    }
    ;
    int_lock(l1);
    ipath_pd = (void *)0;
    int_unlock(l1);
}


