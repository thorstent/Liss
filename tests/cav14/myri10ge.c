/* Based on 8c2f5fa51e1b22db53acf4f3918b6f590b4a35a1
 *
 * This example is somewhat interesting, as it models a race due to weakly
 * consistent memory model that can be solve by inserting memory barriers.
 *
 * The original implementation serialises writes to hardware registers using
 * a spinlock; however this does not enforce sequential ordering of writes performed on
 * different CPUs, leading to the following race:
 *
 * (irq_thread):        1->2->3(queue_active:=0)->4->5->6->7->
 * (tx_thread):         a->b(queue_active:=1)->c->d->e->
 * (cpu2_write_thread): i->X(hw_state:=RUNNING)->Y->
 * (cpu1_write_thread): P->X(hw_state:=STOPPED)->Y (assertion failure)
 *
 * The solution is to insert two memory barriers, which are equivalent to 
 * the two statements labelled "!!!"
 */

#include "langinc.h"

#define STOPPED 1
#define RUNNING 2

#define send_stop 1
#define send_go   2

lock_t l;
int queue_active;
int hw_state;

int addr1;
int val1;
conditional_t write_in_progress1;

int addr2;
int val2;
conditional_t write_in_progress2;

// write functions queue write to the corresponding CPU, but don't perform the actual write
void write_cpu1 (int val, int addr) {
    // 4.
    addr1 = addr;
    // 5.
    val1  = val;
    // 6.
    notify(write_in_progress1);
}

void write_cpu2 (int val, int addr) {
    // c.
    addr2 = addr;
    // d.
    val2  = val;
    // e.
    notify(write_in_progress2);
}


// write to hw register
void write(int val, int addr) {
    // X. 
    {
      if (val==1) {
        if (addr == send_stop)
            hw_state = STOPPED;
        else if (addr == send_go)
            hw_state = RUNNING;
      }
    }

    // Y. software and hw state must be consistent
    //assert ((hw_state == RUNNING && queue_active) || (hw_state == STOPPED && !queue_active));
}

// irq thread disables hw transmit engine in when there is nothing to send
// by writing a doorbell register
void thread_irq() {
    

    // 1.
    lock(l);
//    atomicBegin();
//    assume(lock==0);
//    lock = 1;
//    atomicEnd();

    // 2.
    if (nondet) {
        // 3.
        queue_active = 0;
        write_cpu1(1, send_stop);
        // !!! 
        //wait_not (write_in_progress1);
    };

    // 7.
    unlock(l);
}

// tx_thread re-enables the transmit engine by writing another doorbell register
void thread_tx () {

    // a.
    lock(l);

    // b.
    queue_active = 1;
    write_cpu2(1, send_go);
    // !!! 
    //wait_not (write_in_progress2);

    // f.
    unlock(l);    
}


// models CPU1 write buffer: wait for a write command then execute it
void thread_cpu1_write() {
    while (nondet) {
        // P.
        assume(write_in_progress1);
        write (val1, addr1);
        // Q.
        reset(write_in_progress1);
    };
}

// CPU2 write buffer
void thread_cpu2_write() {
    while (nondet) {
        // i.
        assume(write_in_progress2);
        write (val2, addr2);
        // ii.
        reset(write_in_progress2);
    };
}


//void main () {
//
//    lock = 0;
//    queue_active = 0;
//    hw_state = RUNNING;
//
//    addr1 = 0;
//    val1  = 0;
//    write_in_progress1 = 0;
//
//    addr2 = 0;
//    val2  = 0;
//    write_in_progress2 = 0;
//
//    irq_thread ();
//    tx_thread ();
//    cpu1_write_thread();
//    cpu2_write_thread();
//}
