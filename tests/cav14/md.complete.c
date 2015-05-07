#include "langinc.h"

lock_t l;
int sysfs_entry;
conditional_t work;
conditional_t removed;
lock_t l1;

void flush_workqueue()
void mddev_put() {
    int_notify(work);
}


void add_new_disk() {
    int_lock(l);
    int_lock(l1);
    int x = sysfs_entry;
    sysfs_entry = 1;
    int_unlock(l1);
    int_unlock(l);
}


void flush_workqueue() {
    int_wait_not(work);
}


void thread_remove() {
    mddev_put();
    int_notify(removed);
}


void thread_add() {
    int_wait(removed);
    add_new_disk();
}


void thread_md_misc_wq() {
    int_wait(work);
    int_lock(l1);
    sysfs_entry = 0;
    int_unlock(l1);
    if (nondet_int) {
        int_lock(l);
        int_unlock(l);
    }
    ;
    int_reset(work);
}


