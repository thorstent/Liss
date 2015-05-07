/* This example from the r8169 driver illustrates the REORDER.LOCK pattern.

 The example consists of three threads.*/

#include <langinc.h>

conditional_t napi_poll;
int shutdown = 0;


/*Thread 1 (Interface down thread)
==================================*/

// driver entry point
void thread_1()
{
/*(1)*/ shutdown = 1;
/*(2)*/ notify(napi_poll); // disable NAPI loop
}


/*Thread 3 (NAPI thread)
======================*/


// OS model
void thread_3()
{
	wait_not(napi_poll);
  int x;
  x = shutdown;
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
