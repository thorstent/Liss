#include "langinc.h"

unsigned int irq_status;
unsigned int irq_enable;
lock_t l1;

void thread_i915_enable_vblank() {
    unsigned int tmp;
    int_lock(l1);
    tmp = irq_status;
    irq_status = (tmp | 1) & (~2);
    irq_enable = 1;
    int_unlock(l1);
}


void thread_i915_disable_vblank() {
    unsigned int tmp;
    int_lock(l1);
    irq_enable = 0;
    tmp = irq_status;
    irq_status = tmp & (~1) & (~2);
    int_unlock(l1);
}


