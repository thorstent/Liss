#define _GNU_SOURCE

#include "langinc.h"

#include <pthread.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h>	

lock_t synthlock_0;
lock_t synthlock_1;
lock_t synthlock_2;
atomic_ushort contention_counter = 0;

#define ncontended 1000l
#define nfalse 1000l
#define nfalse1 1l
#define niter 1000l
pthread_mutex_t l;
int coarse;

volatile long u, v, w;

static void work1() {
lock_s(synthlock_0);
    v=v+1;
    unlock_s(synthlock_0);
}

static void work3() {
lock_s(synthlock_2);
    w=w+1;
unlock_s(synthlock_2);
}

static void work2() {
    lock_s(synthlock_1);
    u = u + 1;
unlock_s(synthlock_1);
}

static void worki() {
    int i;
    int a;
    i = a + 1;
}

/*static void lock () {
    atomic_fetch_add(&contention_counter, 1);
*//*    tracepoint(memcached, lock);*/
    /*pthread_mutex_lock(&l);
}*/

/*static void unlock () {
    pthread_mutex_unlock(&l);
    atomic_fetch_sub(&contention_counter, 1);
*//*    tracepoint(memcached, unlock);*/
//}

#define ndelay 1000

static void delay () {
    //long i;
    //for (i = 0; i < ndelay; i++) {work2();}
  yield();
}

static void* worker_thread_coarse(void*arg) {
    long i;
    long j;
    for (j = 0; j < niter; j++) {
        //lock();
        //tracepoint(memcached, begin, "c");
        //tracepoint(memcached, contention, atomic_load(&contention_counter));
        work1();
	yield();
        for (i = 0; i < nfalse; i++) {worki();}
        work2();
	yield();
        for (i = 0; i < nfalse1; i++) {worki();}
        work3();
        //tracepoint(memcached, end, "c");
        //unlock();
        delay ();
    };
    pthread_exit(NULL);
}

void thread_1() {
  int arg;
  worker_thread_coarse(&arg);
}


void thread_2() {
  int arg;
  worker_thread_coarse(&arg);
}

static void* worker_thread_fine(void*arg) {
    long i;
    long j;
    for (j = 0; j < niter; j++) {
        //lock();
        //tracepoint(memcached, begin, "c");
        for (i = 0; i < ncontended; i++, work1());
        //tracepoint(memcached, end, "c");
        //tracepoint(memcached, contention, atomic_load(&contention_counter));
        //unlock();

        for (i = 0; i < nfalse; i++, work2());

        //lock();
        //tracepoint(memcached, begin, "c");
        for (i = 0; i < ncontended; i++, work1());
        //tracepoint(memcached, end, "c");
        //unlock();

        for (i = 0; i < nfalse; i++, work2());
        delay();
    };
    pthread_exit(NULL);
}

static void usage(char* argv[]) {
    fprintf(stderr, "usage: %s <num_threads> [c|f] <num_iterations> <num_racing_cycles> <num_independent_cycles>\n", argv[0]);
}

int main(int argc, char* argv[]) {
    int i;
    int rc;
    pthread_t * threads;
    int nthreads;
    cpu_set_t cpuset;
    char* locking;

    if (argc != 6) {
        usage(argv);
        exit(-1);
    };
    nthreads   = atoi(argv[1]);
    if (*argv[2] == 'c') {
        coarse = 1;
        locking = "coarse-grained";
    } else if (*argv[2] == 'f') {
        coarse = 0;
        locking = "fine-grained";
    } else {
        usage(argv);
        exit(-1);
    };
    //niter      = atoi(argv[3]);
    //ncontended = atoi(argv[4]);
    //nfalse     = atoi(argv[5]);
    printf("Test run with %i threads, %s locking, %ld iterations, %ld racing cycles, %ld independent cycles\n", 
                            nthreads, locking   , niter        , ncontended      , nfalse);

    threads = (pthread_t*) calloc(nthreads, sizeof(pthread_t));
    pthread_mutex_init(&l, NULL);

    //tracepoint(memcached, begin, "dummy_run");
    for (i = 0; i < nthreads; i++) {
        if (coarse) {
            rc = pthread_create(&threads[i], NULL, worker_thread_coarse, (void *)NULL);
        } else {
            rc = pthread_create(&threads[i], NULL, worker_thread_fine, (void *)NULL);
        };
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        };
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset); 
        rc = pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_setaffinity_np(%i) is %d\n", i, rc);
            exit(-1);
        };
    };
    for (i = 0; i < nthreads; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "ERROR; return code from pthread_join(%i) is %d\n", i, rc);
            exit(-1);
        };
    };
    //tracepoint(memcached, end, "dummy_run");
    free(threads);
    return 0;
}