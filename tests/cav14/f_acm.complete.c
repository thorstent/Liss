#include "langinc.h"

lock_t l;
int bsy;
conditional_t pending;
conditional_t request;
lock_t l1;

void acm_cdc_notify()
int usb_ep_queue()
void acm_cdc_notify() {
    int_lock(l);
    int_lock(l1);
    int_reset(pending);
    if (!bsy) {
        bsy = 1;
        int_unlock(l);
        if (usb_ep_queue() == -1)
            bsy = 0;
    } else {
        int_notify(pending);
        int_unlock(l);
    }
    int_unlock(l1);
}


int usb_ep_queue() {
    if (nondet_int) {
        int_notify(request);
        return 0;
    } else {
        return -1;
    }
}


void thread_client1() {
    acm_cdc_notify();
}


void thread_client2() {
    acm_cdc_notify();
}


void thread_worker() {
    while (nondet_int)
        {
            int_assume(request);
            int_lock(l);
            int_lock(l1);
            bsy = 0;
            int_reset(request);
            int_unlock(l);
            if (nondet_int) {
                int_yield();
                int_assume(pending);
                acm_cdc_notify();
            } else {
                int_yield();
                int_assume_not(pending);
            }
            int_unlock(l1);
        }
}


