/* Based on 13c76aba7846647f86d479293ae0a0adc1ca840a
 *
 * Fix: lock around drbd
 */

#define R_SECONDARY 1
#define R_PRIMARY   2

#include <langinc.h>

lock_t l;
int STATE_SENT;
int old_role;
int role;
int old_uuid;
int uuid;
int sent_uuid;

void thread_drbd_set_role () {
  int x;
    // 1.
    lock(l);

    while (1) {
      x = !STATE_SENT;
        // 2.
        if (x) {
            // 3.
            role = R_PRIMARY;
            // 4.
            uuid = uuid+1;
        }
        if (x) {
          break;
        }
    }

    // 5.
    unlock(l);
}

void thread_drbd () {
    //lock(l);
    // a.
    STATE_SENT = 1;

    // b.
    sent_uuid = uuid;
    // c.
//    assert ((role == old_role) || (sent_uuid != old_uuid));

    // d.
    STATE_SENT = 0;
    //unlock(l);
}

//void main() {
//    role = R_SECONDARY;
//    uuid = nondet();
//    old_role = role;
//    old_uuid = uuid;
//
//    lock = 0;
//    STATE_SENT = 0;
//
//    thread_drbd_set_role ();
//    thread_drbd_connect ();
//}
