/* Based on 8449fc3ae58bf8ee5acbd2280754cde67b5db128
 *
 * Race in the initial implementation (ABBA deadlock): 1 -> 2 -> 3 -> 4 -> 5 (want_mtx:=MMAP_THREAD) -> A -> B -> C -> D -> E (assertion failure)
 * Possible fix: Move J after D. 
 * New race: A -> B -> C -> D -> J -> i -> ii (assertion failure)
 *
 * In this example there does not exist a fix based on reordering.  The actual patch
 * replaces lock acquisition in 6 with non-blocking try-lock.
 *
 * Language inclusion does not catch the deadlock, but does catch the race when moving J after D.
 */

#include <langinc.h>

lock_t mtx;
//int want_mtx;
lock_t sem;
//int want_sem;

#define MMAP_THREAD  1
#define IOCTL_THREAD 2

#define STATE0       0
#define INITIALISED  1
#define MAPPED       2
#define INCONSISTENT 3

int state;
int vm_consistent;

void thread_mmap () {

    // 1. lock semaphore
    lock(sem);

    // 2.
    //assert (vm_consistent == 1);
    // 3.
    vm_consistent = 0;

    // This should hopefully prevent the tool from moving statements
    // across the boundaries of the if-block.
    if (1) {
        // 4.
        //assert ((want_sem == 0) || (mtx == 0));
        // 5.
        //want_mtx = MMAP_THREAD;

        // 6. lock mutex
        lock(mtx);

        // 7.
        //assert (state != INCONSISTENT);
        // 8.
        state = INCONSISTENT;
        // 9.
        state = INITIALISED;
        // 10.
        state = INCONSISTENT;
        // 11.
        state = MAPPED;

        // 12.
        unlock(mtx);
    }

    // 13.
    vm_consistent = 1;
    // 14.
    unlock(sem);
}

void thread_ioctl () {
    int old_state;

    // A.
    lock(mtx);

    // B.
    //assert (state != INCONSISTENT);
    // C.
    old_state = state;
    // D.
    state = INCONSISTENT;

    // J.
    unlock(mtx);
    if (nondet) {
        // E.
        //assert ((want_mtx == 0) || (sem == 1));
        //want_sem = IOCTL_THREAD;
        
        // F.
        lock(sem);

        // G.
        //assert (vm_consistent);

        // H.
        unlock(sem);
    };

    // I.
    state = old_state;

}

//void thread_rw () {
//    // i.
//    lock(mtx);
//
//    // ii.
//    assert (state != INCONSISTENT);
//
//    // iii.
//    unlock(mtx);
//}

//void main () {
//
//    mtx = 0;
//    want_mtx = 0;
//    sem = 1;
//    want_sem = 0;
//    state = STATE0;
//    vm_consistent = 1;
//
//    mmap_thread ();
//    ioctl_thread ();
//    rw_thread ();
//}
