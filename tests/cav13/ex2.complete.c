#include "langinc.h"

conditional_t init_sem;
int (*hw_start)() = 0;

int start_device() {
    return 0;
}


void register_netdev() {
    int_notify(init_sem);
}


void rtl8169_open() {
    (*hw_start)();
}


void thread_1() {
    hw_start = &start_device;
    register_netdev();
}


void thread_2() {
    int_wait(init_sem);
    rtl8169_open();
}


