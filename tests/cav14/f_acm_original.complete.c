/* A simplified version of e50ae572b33646656fa7037541613834dcadedfb
 */

// Language inclusion catches the race, but not the deadlock

// faulty execution (w = worker,  c1 = client1, c2 = client2):
// c1.0 (lock:=CLIENT1) -> c1.1 -> c1.2 -> c1.3 -> c1.4 -> c1.i (request:=CLIENT1) -> w.A -> w.B (assertion failure)
//
// possible fix that eliminates this particular race: move statement 6 to just before 4.  This creates another race:
// c1.0 (lock:=CLIENT1) -> c1.1 -> c1.2 -> c1.3 -> c1.6 (lock:=0) -> c1.4 -> c1.i -> c2.0 (lock:=CLIENT2) -> c2.1 -> c2.2 (bsy==1) -> c2.7 -> c1.5 (bsy:=0) -> c2.8 (assertion failure)
//
// there does not exist a correct fix based solely on reordering.  One solution is to drop the lock before 4 and re-acquire it just before 5 and drop again after 5.
// Alternatively, just add an atomic section arond 4, 5, and 6.

#include "langinc.h"

// thread id's
#define CLIENT1 1
#define CLIENT2 2
#define WORKER  3

// ensures atomic access to other variables
lock_t synthlock_2;
lock_t l; 

// bsy flag.  No new request can be started when this is true
int bsy;

// if a new request arrives when bsy==1 then this flag is set to 1;
// the pending request will be executed once the current request completes
conditional_t pending;

// signals to the worker thread that there is a request for it to handle
conditional_t request;


void acm_cdc_notify ();
int  usb_ep_queue ();

// queue request
void acm_cdc_notify () {
  
  // 0. acquire lock
  lock(l);
  
  // 1. if we are invoked to handle a pending request, clear the pending flag.
  lock_s(synthlock_2);
  reset(pending);
  
  // 2.
  if (!bsy) {
    // 3. send request to worker thread
    bsy = 1;
    
    // 6.
    unlock(l);
    // 4.
    if (usb_ep_queue() == -1) 
      // 5.
      bsy = 0;
    
    
  } else {
    // 7. if the worker thread is busy, mark request as pending
    notify(pending);
    // 8. catch potential race: request has been marked as pending, but it will never be executed,
    //    as the worker thread has gone idle
    //assert (bsy);
    
    // 9.
    unlock(l);
  }
unlock_s(synthlock_2);
}

// attempt to submit a request to the worker thread; fail nondeterministically
int usb_ep_queue () {
  if (nondet) {
    // i.
    notify(request);
    return 0;
  } else {
    // ii.
    return -1;
  }
}


// Client thread 1
void thread_client1() {
  acm_cdc_notify ();
}

// Client thread 2
void thread_client2() {
  acm_cdc_notify ();
}


// Worker thread
void thread_worker () {
  while (nondet) {
    // A.
    lock_s(synthlock_2);
    assume (request);
    
    // B. not allowed to wait here
    //        assert (lock != request);
    unlock_s(synthlock_2);
    lock(l);
    
    // C. handle the request and update state variables
    lock_s(synthlock_2);
    bsy = 0;
    reset(request);
    
    // D.
    unlock_s(synthlock_2);
    unlock(l);
    
    // E. if there are more requests pending, schedule them now
    if (nondet) {
      // Without this yield, sequential semantics does not allow a preemption before next lock acquisition
      yield();
      assume(pending);
      acm_cdc_notify();
    } else {
      yield();
      assume_not(pending);
      //unlock(l);
    }
  }
}

//void main () {
//    lock = 0;
//    bsy = 0;
//    pending = 0;
//    request = 0;
//
//    client_thread1 ();
//    client_thread2 ();
//    worker_thread ();
//}
