#include "langinc.h"

conditional_t neh_stopped;
conditional_t stop_neh;
int neh_state;
conditional_t rc_stopped;
conditional_t stop_rc;
int rc_state;
conditional_t rsv_stopped;
conditional_t stop_rsv;
int rsv_state;

void thread_shutdown() {
    int_notify(stop_rsv);
    int_wait(rsv_stopped);
    int_notify(stop_rc);
    int_wait(rc_stopped);
    int_notify(stop_neh);
    int_wait(neh_stopped);
}


void thread_rc() {
    int x = neh_state;
    int_wait(stop_rc);
    rc_state = x;
    int_notify(rc_stopped);
}


void thread_neh() {
    int x = rsv_state;
    int_wait(stop_neh);
    neh_state = x;
    int_notify(neh_stopped);
}


void thread_rsv() {
    int_wait(stop_rsv);
    rsv_state = 1;
    int_notify(rsv_stopped);
}


