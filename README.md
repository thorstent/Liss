New Semantics branch
====================

This branch implements two changes in the non-preemtive semantics:

1) Assumes do not allow for context switches if they cannot continue
2) When a lock or wait is reached a context switch is always allowed (past behaviour was that it was only allowed if the lock was taken or the wait not notified)

Liss (Language Inclusion-based Synchronisation Synthesis)
=======

Liss is the implementation that accompanies our paper [1]. It is a synthesis tool that is given a concurrent C program and will insert synchronisation primitives until the behaviour of the concurrent version of the program is identical to the sequential version. It is currently a research prototype.


[1] Pavol Cerny, Edmund C. Clarke, Thomas A. Henzinger, Arjun Radhakrishna, Leonid Ryzhyk, Roopsha Samanta, Thorsten Tarrach. From Non-preemptive to Preemptive Scheduling using Synchronization Synthesis. In CAV 2015

Compiling Liss
==============

Cloning the GIT
---------------

When cloning this GIT repository the submodules need to be initialised as well. Use

	git clone --recursive https://github.com/thorstent/Liss.git

If you already cloned Liss you can initialise the submodules by running

	git submodule update --init --recursive

Dependencies
------------

As dependencies LLVM/Clang 3.6 and Boost are required. Under **Ubuntu Vivid Vervet (15.04)** necessary dependencies can be installed with

	sudo apt-get install git g++ cmake libboost-filesystem-dev libboost-system-dev 
	sudo apt-get install clang-3.6 libclang-3.6-dev libz-dev libedit-dev

<a name="tests"></a>Test cases in the archive
-------------------------

The `tests` folder contains our test cases. `CAV13` and `CAV14` are test cases from our previous papers. `linux_drivers` contains the cpmac driver that models a real Linux device drivers with some simplifications. For each example there are three files and one folder generated:

| File extension | Meaning |
|----------------|---------|
| .c             | This is the test file that serves as input to Liss. |
| .start.c       | This is the input file printed by Liss after it was parsed. This is useful because when Liss prints C code the code may not be formatted exactly like the input. |
| .complete.c    | This is the file with the synthesised synchronisation primitives. The changes can be easily visualised by diffing this file with the .start.c file. |
| .log           | The log file contains various debug output produced by Liss, among other things the fixes applied in each iteration. |

The .output folder generated for each example contains a C file with the code after each iteration. It also contains dot graphs of the automata of each thread. The complete automaton is not printed by default as it too large for any reasonable program. It can be generated with the print option.

How to generate these files is described [below](#synth). The .complete files are also checked into the repository.

Source code
-----------

The source code is split into two folders: `libs/Limi` holds the language inclusion code. We treat it as a library, but it is completely developed by us as part of this project. `src` holds the remaining code.

Running
=======

<a name="synth"></a>./run_synth.sh
--------------

The `./run_synth.sh` script will run all the tests. It produces the output files described [above](#tests).

Command Line
------------

To do a simple language inclusion test between the non-preemptive and the preemptive semantics one can use

	./liss -inclusion -v 2 input.c --

It is important to keep the `--` in the end of the command line. The option `-v` sets the verbosity level. Level 2 offers some debug information.

In the place of `-inclusion` any of the following actions may be used:

| Action       | Description                                                                                              |
|--------------|----------------------------------------------------------------------------------------------------------|
| `-inclusion` | Tests if all preemptive traces are also preemption-free traces. Prints a counter-example if one exists. |
| `-synthesis` | Invoke the synthesis and output a fixed file. Use the deadlock switch to find out if deadlocks were created by the synthesis. |
| `-deadlock`  | Test for potential deadlocks and output a trace if one is found.                                         |
| `-print`     | Output a .dot file with the non-preemptive and preemptive automaton (only useful for small programs).        |

Common error messages
------

### LLVM ERROR: Could not auto-detect compilation database for file

This error means the `--` at the end of the command line is missing

### 'langinc.h' file not found with angled include; use "quotes" instead

This error can be ignored if `langinc.h` is in the same directory as the c file, so that clang can find the include file. As an alternative an include directory can be specified using `-I/path/where/langinc.h` after the `--`.

### 1 error generated. Error while processing {file}

This is output by clang and indicates a compilation error (see beginning of the output).


Input Files
-----

As input a C file should be provided that includes `langinc.h`. The C file must follow these conventions:

- Locks must have type `lock_t`
- Conditional variables must have type `conditional_t`
- Conditionals and locks must not be referred to in ordinary C expressions, but only through the functions provided in `langinc.h`.
- Threads are functions with a name that either starts or ends with "thread". Our semantics assumes a fixed set of threads that start simultaneously, when the program is started. There is no main thread. The program ends when all threads have terminated.
- No recursive functions are allowed

The `langinc.h` header file specifies a number of functions that can be used in the source code for synchronisation. Note that pthread synchronisation is ignored.

| Function        | Meaning                                                                 |
|-----------------|-------------------------------------------------------------------------|
| `lock(l)`       | Lock the lock `l`                                                       |
| `unlock(l)`     | Unlock the lock `l`                                                     |
| `notify(c)`     | Set `c` to notified                                                     |
| `reset(c)`      | Set `c` to unnotified                                                   |
| `wait(c)`       | Wait until `c` is notified                                              |
| `wait_reset(c)` | Wait until `c` is notified and set atomically to unnotified             |
| `wait_not(c)`   | Wait until `c` is unnotified                                            |
| `assume(c)`     | Consider only traces where `c` is notified at this point                |
| `yield`         | Allow a context-switch in the preemption-free execution                 |
| `nondet`        | Special variable indicating non-determinism (to be used in if or while) |

`c` is a variable of type `conditional_t` and `l` is a variable of type `lock_t`.

See the paper for a precise definition of the semantics of these constructs. Here is the intuitive version:

### Program model

Liss allows the programmer to program a parallel program with a non-preemptive scheduler in mind. Liss then adds enough synchronisation to make the program safe for a standard preemptive scheduler. 

### Locks and Waits

Lock and wait will block the thread until the condition is fulfilled. This can of course produce deadlocks. The only difference between assumes and waits is that assumes are not considered deadlocks (they are not reported by the `-deadlock` switch (see below)). 

Liss compares the preemption-free execution of the program to the preemptive execution. The preemptive execution may context-switch after each instruction. Note that `x=x+1` is not atomic and a context-switch may occur between reading and writing `x`. The preemption-free version can choose in the beginning the starting thread freely, but after that is only allowed to switch in certain situations. If a `yield` is encountered the scheduler may choose non-deterministically to either keep executing the current thread or switch to any other thread. If a `lock` or `wait` is encountered the scheduler will switch iff the lock it not available or the wait is not satisfied. If the scheduler switches it may switch to any other thread.

### Assumes and Waits

When using the `-synthesis` switch assumes and waits are identical. One can either think of them as assumes, meaning any trace violating the assume is discarded or as waits, meaning the waiting thread is descheduled and rescheduled when the condition is fulfilled. Since we reason over all possible traces these views are identical.

However, in the context of deadlock detection these make a difference. If a trace is discarded due to a failing assume it is not reported as a deadlock. If a thread cannot progress due to a wait and all other threads are either blocked or finished then a deadlock is reported.

Therefore assumes are used to guard conditional branches or loops, whereas waits are used for scenarios where a thread should wait for a condition.

The output
-----------------

The `-inclusion` and `-deadlock` switches output a trace as a counter-example.
Traces consist of the symbols that were executed in the preemptive automaton of the form `0lo(l)[f_acm_modified.c:43]`. The first integer is the thread-id. The ids are assigned to thread functions in alphabetical order. Next is a label and in parenthesis the variable the label operates on. Last is the filename and line number. This is a complete list of labels:

| Label | Meaning               |
|-------|-----------------------|
| w     | Write                 |
| r     | Read                  |
| lo    | Lock                  |
| un    | Unlock                |
| no    | Notify                |
| rs    | Reset                 |
| wr    | Wait-Reset            |
| wa    | Wait / Assume         |
| wn    | Wait not / Assume not |
| yi    | Yield                 |

All but w, r are considered epsilon transitions for language inclusion purposes.

In the dot files additionally also the states are named:

A state name consists of a list of ids, one for each thread.
Then follows a list in brackets of locks held and notifies active.
