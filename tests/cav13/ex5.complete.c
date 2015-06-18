/* Example based on a bug in the i2c-hid driver.
 * This example illustrates the LOCK pattern.
 *
 * commit# 7a7d6d9c5fcd4b674da38e814cfc0724c67731b2
 */

/* Error scenario:
 *
 * open_thread                    close_thread
 * -----------                    ------------
 * (open == 0) --> yes
 * power_on=1
 * open++ // open=1
 *                                open-- // open=0
 *                                (open==0) --> yes
 * (open == 0) --> yes
 * power_on=1
 * open++ // open=1
 *                                power_on=0
 * assert (power_on) // ERROR
 */

/* One possible fix is to put locks around the body of i2c_hid_open() or i2c_hid_close().
 */

#include "langinc.h"

lock_t synthlock_0;
lock_t l;
conditional_t open;
int power_on = 0;

/* A client wants to start using the device.
 * Powers up the device if it is currently closed. */
void i2c_hid_open() {
    int x;
//    lock(l);

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
    //assert (power_on != 0);

//    unlock(l);
unlock_s(synthlock_0);
}

/* A client has stopped using the device.
 * Power down the device if this is the last client.
 */
void i2c_hid_close ()
{
    int x;
    lock(l);
    lock_s(synthlock_0);

    reset(open);

    if (nondet) {
        assume_not(open);
        power_on = 0;    
    } 

    x = power_on;
    unlock_s(synthlock_0);
    //assert (power_on == 0);

    unlock(l);
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

//void main() {
//    open = 0;
//    power_on = 0;
//    thread_open();
//    thread_close();
//}
