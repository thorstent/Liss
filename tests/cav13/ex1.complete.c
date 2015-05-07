#include "langinc.h"

conditional_t IntrMask;
int intr_mask;
int handled;

void thread_1() {
    int_wait(IntrMask);
    if (intr_mask == 1) {
        handled = 1;
    } else {
        handled = 0;
    }
}


void thread_2() {
    intr_mask = 1;
    int_notify(IntrMask);
}


