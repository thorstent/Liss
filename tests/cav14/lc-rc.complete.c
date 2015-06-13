/* Based on 58be81ed301d96045bca2b85f3b838910efcfde4
 * (this patch actually fixes several related races; we only model one of them here)
 *
 * The essence of this example is that there are three worker threads that depend on each other.
 * The threads must be killed in the right order to avoid dangling references.
 * 
 * Race in the original implementation: 1->i->ii->2->P(assertion failure)
 * There are many instruction reorderings that fix this race, e.g., 5;6;1;2;3;4, but
 * it introduces a new race: 5->P->Q->R->a (assertion failure).
 * The only correct rearrangement is: 3->4->5->6->1->2
 */
// Language inclusion does not detect the race, as it is already present in the sequential version

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

// this thread sends stop signals to each of the other three threads
void thread_shutdown () {
    // tell workers to stop and wait for them to terminate
    // 1.
    notify (stop_rsv);
    // 2.
    wait (rsv_stopped);
    // 3.
    notify(stop_rc);
    // 4.
    wait (rc_stopped);
    // 5.
    notify(stop_neh);
    // 6.
    wait (neh_stopped);
}

void thread_rc () {
    // a.
    //assert (neh_stopped);
    int x = neh_state;
    wait (stop_rc);
    rc_state = x;
    // b.
    // c.
    notify(rc_stopped);
}

void thread_neh () {
    // P.
    //assert (rsv_stopped);
    int x = rsv_state;
    wait (stop_neh);
    neh_state = x;
    // Q.
    // R.
    notify(neh_stopped);
}

void thread_rsv () {
    // i
    wait (stop_rsv);
    rsv_state = 1;
    // ii
    notify(rsv_stopped);
}

//void main () {
//    neh_stopped = 1;
//    stop_neh = 0;
//
//    rc_stopped = 1;
//    stop_rc = 0;
//
//    rsv_stopped = 1;
//    stop_rsv = 0;
//
//    shutdown_thread ();
//    rc_thread ();
//    neh_thread ();
//    rsv_thread ();
//};
