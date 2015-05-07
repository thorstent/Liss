/* Based on d54bc4e3fc5c56600a13c9ebc0a7e1077ac05d59
 *
 * Initial race is an ABBA deadlock: A(lock:=1)->B->C(want_rtnl_lock := START_THREAD)->1->2->3 (assertion failure)
 * The simplest reordering that eliminates this race is to move the block of statements CDEFG before A.
 * This creates a new race: C->D->E->F(notify:=1)->i->ii (assertion failure)
 *
 * Correct reordering: ABH{CDEFG}
 */
// Language inclusion detects the race, but not the deadlock

#define CONFIG_THREAD 1
#define START_THREAD  2

#include <langinc.h>

// Two locks involved in ABBA deadlock
lock_t l;
lock_t rtnl;

// Aux variables used to check for the deadlock
//int want_lock;
//int want_rtnl_lock;

int restarted;
conditional_t n;

void thread_config () {
    //begin_noreorder ();

    // 1.
//    {
//        begin_atomic();
//        assume(rtnl_lock == 0);
//        rtnl_lock = 1;
//        end_atomic ();
//    };
    lock(rtnl);

    // 2.
    //want_lock = CONFIG_THREAD;
    // 3. check for ABBA deadlock
    //assert((lock == 0) || (want_rtnl_lock != START_THREAD));

    // 4.
    lock(l);
//    {
//        begin_atomic ();
//        assume (lock == 0);
//        lock = 1;
//        want_lock = 0;
//        end_atomic();
//    };

    // This thread is only needed to model ABBA deadlock,
    // we don't model what it actually does.

    // 5.
    unlock(l);
    // 6.
    unlock(rtnl);

    //end_noreorder ();
}

void thread_iwl3945_bg_alive_start () {
    // A.
//    {
//        begin_atomic();
//        assume(lock==0);
//        lock = 1;
//        end_atomic();
//    };
    //begin_noreorder();
    // C.
    //want_rtnl_lock = START_THREAD;
    // D. check for ABBA deadlock
    //assert((rtnl_lock == 0) || (want_lock != CONFIG_THREAD));

    // E.
//    {
//        begin_atomic();
//        assume(rtnl_lock == 0);
//        rtnl_lock = 1;
//        want_rtnl_lock = 0;
//        end_atomic();
//    };
    lock(rtnl);

    // F. tell reassoc_thread that we are ready
    notify(n);

    // G.
    unlock(rtnl);
    //rtnl_lock = 0;
    //end_noreorder();

    lock(l);

    // B.
    restarted = 1;

    // H.
    unlock(l);
}

void thread_reassoc () {
    int x;
    // i.
    wait (n);
    // ii.
    //assert (restarted == 1);    
    x = restarted;
}

//void main () {
//    int lock = 0;
//    int want_lock = 0;
//    int rtnl_lock = 0;
//    int want_rtnl_lock = 0;
//    int restarted = 0;
//    int notify = 0;
//    
//   
//    config_thread();
//    iwl3945_bg_alive_start_trhead ();
//    reassoc_thread();
//}
