#include "langinc.h"

conditional_t napi_poll;
int shutdown = 0;
;
lock_t l1;

void stuff1()
void stuff2()
void stuff3()
void stuff1() {
    shutdown = 1;
}


void thread_1() {
    int_lock(l1);
    stuff1();
    int_notify(napi_poll);
    int_unlock(l1);
}


void stuff2() {
}


void rtl_shutdown() {
    stuff2();
}


void thread_2() {
    rtl_shutdown();
}


void stuff3() {
    int x;
    x = shutdown;
}


void rtl8169_poll() {
    stuff3();
}


void thread_3() {
    int_lock(l1);
    int_assume_not(napi_poll);
    rtl8169_poll();
    int_unlock(l1);
}


