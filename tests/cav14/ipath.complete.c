/* Based on 3d0890985ac4dff781b7feba19fedda547314749
 *
 * This models part of the patch.  The patch introduces a lock, but then the lock 
 * must be correctly placed to protect sensitive code but avoid deadlock.  We model
 * the latter part only.
 *
 * The original lock placement introduces deadlock:
 * a -> b -> c -> d -> i (deadlock)
 *
 * The smallest possible fix is to move (g) before (c),
 * which introduces another race:
 * a->b->g->c->d->e->1->2(ipath_pd != NULL)->f->3(assertion failure)
 * 
 * a correct fix is to move both (f) and (g) before (c)
 */

// Language inclusion catches the race, but no the deadlock

#include  <langinc.h>

lock_t synthlock_1;
void * ipath_pd;
lock_t l; // spinlock does not allow blocking
int block;  // indicates that a potentially blocking operation is in progress

void thread_irq () {
    void * x;

    // 1.
    lock(l);
    lock_s(synthlock_1);

    // 2. check that ipath_pd is not NULL
    if (ipath_pd) {
        // 3. use ipath_pd
        //assert (ipath_pd);
        x = ipath_pd;
    };

    // 4.
    unlock_s(synthlock_1);
    unlock(l);
}

void thread_user () {
    void * tmp;

    // a.
    lock(l);

    // b. make a copy of ipath_pd
    tmp = ipath_pd;

    unlock(l);

    // c. perform some blocking operations on it
    if (tmp) {
        // d.
        block = 1;
        // e.
        block = 0;
    };
        
    // f. deallocate
    lock_s(synthlock_1);
    ipath_pd = (void *)0;
    unlock_s(synthlock_1);

    // g.
}

//void thread_checker() {
//    // i.
//    assert (block == 0 || lock == 0);
//}

//void main () {
//
//    ipath_pd = (void *) 1;
//    lock = 0;
//    block = 0;
//
//    checker_thread();
//    irq_thread();
//    user_thread();
//}
