#include "langinc.h"

int devlist_nonempty;
struct {
    int vdev;
    int vbi_dev;
};
struct (anonymous struct at /home/ttarrach/Documents/Work/7Liss/tests/cav14/em28xx.c:16:1) devlist;
conditional_t registered;

void thread_init() {
    devlist_nonempty = 1;
    devlist.vdev = 1;
    devlist.vbi_dev = 1;
    int_notify(registered);
}


void thread_client() {
    int c1, c2, c3;
    int_wait(registered);
    c1 = devlist_nonempty;
    c2 = (devlist.vdev != -1);
    c3 = (devlist.vbi_dev != -1);
}


