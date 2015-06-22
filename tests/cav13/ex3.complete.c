/* This example from the r8169 driver illustrates the REORDER.LOCK pattern.

 The example consists of three threads.*/

#include <langinc.h>

lock_t synthlock_2;
conditional_t napi_poll;
int shutdown = 0;

void stuff1();
void stuff2();
void stuff3();

/*Thread 1 (Interface down thread)
==================================*/

void stuff1() {
	shutdown = 1;
}

// driver entry point
void thread_1()
{
/*(1)*/ lock_s(synthlock_2);
stuff1();
/*(2)*/ notify(napi_poll);
unlock_s(synthlock_2); // disable NAPI loop
}

/*Thread 2 (Shutdown thread)
===========================*/

void stuff2() {
}

// driver entry point
void rtl_shutdown()
{
	stuff2();
};


// OS model
void thread_2()
{
    // Call rtl_shutdown at random point
    rtl_shutdown();
}


/*Thread 3 (NAPI thread)
======================*/

void stuff3() {
    int x;
    x = shutdown;
}

// driver entry point
void rtl8169_poll()
{
	stuff3();
}

// OS model
void thread_3()
{
	lock_s(synthlock_2);
	assume_not(napi_poll);
	rtl8169_poll();
	unlock_s(synthlock_2);
}

//void main() {
//	napi_poll = 0;
//	shutdown = 0;
//	thread1();
//	thread2();
//	thread3();
//}

/*
The above code contains calls to functions stuff1(), stuff2(), 
stuff3().  These functions represent device-specific code that 
accesses device registers.  Since we do not have an accurate 
device model, we have to assume that these functions are not 
commutative and are only safe to execute in the order consistent 
with sequential execution.

Consider lines (1) and (2).  (2) disables the NAPI thread: 
after this lines has been executed, no new calls to the 
rtl8169_poll method will be performed.  In the simplified 
concurrency model the rtl8169_poll entry point cannot be invoked 
while rtl8169_down is running.  As a result, this model rules out 
any execution where stuff3() is called after stuff1().  

In the realistic concurrency model, a call to rtl8169_poll can 
occur between (1) and (2), thus violating the above invariant.  
This race can be eliminated by simply reordering lines (1) and 
(2), which is exactly what the RTL driver does.
*/
