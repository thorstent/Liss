#include "langinc.h"

lock_t l;
conditional_t open;
int power_on = 0;

void i2c_hid_open() {
    int x;

    if (nondet) {
        assume_not(open);
        power_on = 1;
    } else {
        assume(open);
    }
    notify(open);

    x = power_on;
}

void i2c_hid_close ()
{
    int x;

    reset(open);

    if (nondet) {
        assume_not(open);
        power_on = 0;    
    } 

    x = power_on;
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