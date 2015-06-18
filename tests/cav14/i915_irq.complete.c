/* Based on e9d21d7f5ae1e625f3687d88bb50b00478c533ad
 *
 * There is no safe reordering in this example.
 * 
 * Race in the original implementation: 1->2->a->b->c->3->4(assertion failure)
 * One possible reordering that eliminates this error: move 3 above 1 and move a below c
 * New race: b->c->3->1->2->a(assertion failure)
 * 
 */

#include "langinc.h"

lock_t synthlock_1;
unsigned int irq_status;
unsigned int irq_enable;

#define EN_MASK   0x01
#define STAT_MASK 0x02

void thread_i915_enable_vblank() {
    unsigned int tmp; 

    // 1.
    lock_s(synthlock_1);
    tmp = irq_status;
    // 2.
    irq_status = (tmp | EN_MASK) & (~STAT_MASK);
    // 3.
    irq_enable = 1;
    unlock_s(synthlock_1);

    // 4.
//    assert (((irq_enable == 1) && (irq_status & EN_MASK)) || 
//            ((irq_enable == 0) && !(irq_status & EN_MASK)));
}

void thread_i915_disable_vblank() {
lock_s(synthlock_1);
    unsigned int tmp;

    // a.
    irq_enable = 0;
    // b.
    tmp = irq_status;
    // c.
    irq_status = tmp & (~EN_MASK) & (~STAT_MASK);

    // d.
//    assert (((irq_enable == 1) && (irq_status & EN_MASK)) || 
//            ((irq_enable == 0) && !(irq_status & EN_MASK)));
unlock_s(synthlock_1);
}

//void main () {
//    irq_status = 0;
//    irq_enable = 0;
//
//    i915_enable_vblank_thread ();
//    i915_disable_vblank_thread ();
//}
