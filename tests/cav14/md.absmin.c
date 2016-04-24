/* Based on a7a3f08dc24690ae00c75cfe4b4701a970dd0155
 *
 * This example requires the use of the flush_workqueue() barrier to wait for an async 
 * activity to complete.
 *
 * Race in the original implementation:
 *
 * 1->2->3->P->Q->R->S (assertion failure)
 *
 * The race is fixed by adding the line labelled with "!!!", which waits for the
 * work queue to complete processing device removal
 *
 * Note that the flush_workqueue() barrier must be placed before acquiring the 
 * lock to avoid deadlock.  Otherwise, if the flush_workqueue() is placed after
 * the lock has been acquired, the following becomes possible:
 *
 * 1->2->3->P->Q->R(lock:=1)->V(wait_for_workqueue:=1)->a->b->c->d (deadlock)
 *
 */

// Language inclusion detects race

#include <langinc.h>

lock_t synthlock_0;
lock_t l;
int sysfs_entry;
conditional_t work;
conditional_t removed;
//int wait_for_workqueue;


void flush_workqueue();
    
void mddev_put() {
    // 2. complete request asynchronously, as this function is not allowed to block
    notify(work);
}

void add_new_disk() {
    // need a barrier here:
    // !!!
    //flush_workqueue();

    // R. lock mutex
    lock(l);

    // putting the barrier here causes deadlock

    // S.disk removal must be finished by now
    lock_s(synthlock_0);
    int x = sysfs_entry;
    //assert (sysfs_entry == 0);
    // T.
    sysfs_entry = 1;

    // U.
    unlock_s(synthlock_0);
    unlock(l);
}

void flush_workqueue() {
    //noreorderBegin ();

    // V.
    //wait_for_workqueue = 1;
    // W.
    wait_not (work);
    // X.
    //wait_for_workqueue = 0;
    //noreorderEnd ();
}

void thread_remove () {
    // 1. remove disk
    mddev_put();
    // 3.
    notify(removed);
}

void thread_add () {
    // P.
    wait (removed);
    // Q.
    add_new_disk ();
}

// workqueue thread that completes the disk remove task
void thread_md_misc_wq() {
    //noreorderBegin();

    // a. wait for a work item to be queued
    wait(work);
    // b. remove sysfs entry
    lock_s(synthlock_0);
    sysfs_entry = 0;
    unlock_s(synthlock_0);

    // c. under some circumstances, this code may need to take the lock
    if (nondet)  {
        // d. deadlock check
        //assert (lock == 0 | wait_for_workqueue == 0);
        // e.
        lock(l);
        // f.
        unlock(l);
    };
    // g.
    reset(work);

    //noreorderEnd();
}


//void main () {
//    lock = 0;
//    sysfs_entry = 1;
//    work = 0;
//    removed = 0;
//    wait_for_workqueue = 0;
//
//    add_thread ();
//    remove_thread ();
//    md_misc_wq_thread ();
//}
