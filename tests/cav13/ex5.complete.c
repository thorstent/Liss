#include "langinc.h"

lock_t l;
conditional_t open;
int power_on = 0;
lock_t l1;

void i2c_hid_open() {
    int x;
    int_lock(l1);
    if (nondet_int) {
        int_assume_not(open);
        power_on = 1;
    } else {
        int_assume(open);
    }
    int_notify(open);
    x = power_on;
    int_unlock(l1);
}


void i2c_hid_close() {
    int x;
    int_lock(l);
    int_reset(open);
    if (nondet_int) {
        int_assume_not(open);
        power_on = 0;
    }
    x = power_on;
    int_unlock(l);
}


void thread_open() {
    while (nondet_int)
        {
            int_yield();
            i2c_hid_open();
        }
}


void thread_close() {
    while (nondet_int)
        {
            int_lock(l1);
            int_assume(open);
            i2c_hid_close();
            int_unlock(l1);
            int_yield();
        }
}


