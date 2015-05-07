#include "langinc.h"

lock_t l;
int queue_active;
int hw_state;
int addr1;
int val1;
conditional_t write_in_progress1;
int addr2;
int val2;
conditional_t write_in_progress2;

void write_cpu1(int val, int addr) {
    addr1 = addr;
    val1 = val;
    int_notify(write_in_progress1);
}


void write_cpu2(int val, int addr) {
    addr2 = addr;
    val2 = val;
    int_notify(write_in_progress2);
}


void write(int val, int addr) {
    {
        if (val == 1) {
            if (addr == 1)
                hw_state = 1;
            else if (addr == 2)
                hw_state = 2;
        }
    }
}


void thread_irq() {
    int_lock(l);
    if (nondet_int) {
        queue_active = 0;
        write_cpu1(1, 1);
    }
    ;
    int_unlock(l);
}


void thread_tx() {
    int_lock(l);
    queue_active = 1;
    write_cpu2(1, 2);
    int_unlock(l);
}


void thread_cpu1_write() {
    while (nondet_int)
        {
            int_assume(write_in_progress1);
            write(val1, addr1);
            int_reset(write_in_progress1);
        }
    ;
}


void thread_cpu2_write() {
    while (nondet_int)
        {
            int_assume(write_in_progress2);
            write(val2, addr2);
            int_reset(write_in_progress2);
        }
    ;
}


