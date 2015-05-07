#include "langinc.h"

lock_t l;
lock_t rtnl;
int restarted;
conditional_t n;

void thread_config() {
    int_lock(rtnl);
    int_lock(l);
    int_unlock(l);
    int_unlock(rtnl);
}


void thread_iwl3945_bg_alive_start() {
    int_lock(rtnl);
    int_unlock(rtnl);
    int_lock(l);
    restarted = 1;
    int_notify(n);
    int_unlock(l);
}


void thread_reassoc() {
    int x;
    int_wait(n);
    x = restarted;
}


