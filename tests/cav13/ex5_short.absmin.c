#include "langinc.h"

lock_t synthlock_0;
lock_t l;
conditional_t open;
int power_on = 0;

void i2c_hid_open() {
    int x;

    if (nondet) {
        lock_s(synthlock_0);
        assume_not(open);
        power_on = 1;
    } else {
        lock_s(synthlock_0);
        assume(open);
    }
    notify(open);

    x = power_on;
    unlock_s(synthlock_0);
}

void i2c_hid_close ()
{
    int x;

    lock_s(synthlock_0);
    reset(open);

    if (nondet) {
        assume_not(open);
        power_on = 0;    
    } 

    x = power_on;
    unlock_s(synthlock_0);
}

void thread_open() {
    while (nondet)
    {
        yield();
        i2c_hid_open();
    }
}

void thread_close() {
    while (nondet)
    {
      assume(open);
      i2c_hid_close();
      yield();
    }
}