## Overview

#### How to run:
```
./uthread-sync-demo <threads count A> <threads count B>
```
A+B < 99

### Additional Assumptions
1. When calling on signal() and wait(), assume the same lock is used
2. Async I/O: only one read/write operation is running at one time


### Priority Inversion Solution
We implemented the priority boosting solution. In uthread.cpp, we create a function named priority_boost().
This function will get the threads from low priority and medium priority queue. If the thread holds a lock or locks, 
a random flag will determine whether this thread will be boosted to nect priority. For the green queue, there will be two consecutive
random flags to determine whether this thread will be boosted to red priority.
If the thread has no lock, it will be put back to original queue. This function will be called from yield() before switching the 
threads, as each time yield() is called, there will be a new round of scheduling. 

### Lock vs. Spinlock
For this section, we found that under the current test settings (long critical sections and high contention), the lock is faster than the spinlock due to the fact that the spinlock will cause the thread to busy wait. The lock will put the thread to sleep and wake it up when the lock is available.

In a uniprocessor user-thread library like uthread, the performance of both types of locks may be similar due to the absence of parallel execution. However, in a multi-core system where threads can execute in parallel on different cores, the performance characteristics may differ:  
* Spin locks: In a multi-core system, spin locks may suffer from increased contention and cache coherency issues. Multiple spinning threads contending for the same lock can lead to cache thrashing and decreased scalability. 
* Traditional locks (mutexes): Mutexes, on the other hand, may exhibit better scalability in a multi-core system due to their ability to block and yield the CPU when contended. This allows other threads to make progress, reducing contention and potentially improving overall system throughput.

### Priority Inversion
The test case creates 4 threads, one low priority, 2 medium and one high. Assuming the threads is scheduled in sequence. Without priority boosting mechanism, the low priority thread will enter the critical section and grab the lock. As the low priority thread is excuting, the later 3 threads will be added to the ready queue. Since the high priority thread will wait for the lock, 2 medium priority threads will be scheduled and run before high priority thread. 

With priority boosting, it will search through the ready queue, for all the low priority threads that hold one or more locks, it will be randomly boosted to medium or high priority. Therefore the low priority threads in the critical section can be scheduled and allow waiting threads with higher priority to proceed. 

The implementation boosts both low and medium priority threads. The boosting will be called in the yield() function since each time a thread yields there will be a new round of scheduling.

### Asynchronization I/O

1. Asynchronization I/O will provide better performance. Synchronized I/O cause threads to wait for the I/O operation to finish before resuming, which means other threads will yield for I/O. For asynchronized I/O, the I/O operation could run seperately while the OS could schedule and run other threads.
2. As the amount of I/O work becomes larger, synchronized I/O will suffer more performance penalty beacuse the I/O will interrupt other threads. For asynchrozied I/O, it depends on the threads' works. If some threads are waiting for the read/write operation, the perfromance will likely to be bad. Generally the performance of async I/O depends on the other threads job length and their data dependencies.
3. As other thread work increases, async I/O will be faster than sync I/O because those threads can run when I/O is operating in the background


### Custom API
No additional custom APIs used

### Parameters Passing
Using stub, the stub function will call top-level thread function. For the return value, those are stored in the finished queue. 
Makecontext() needs to call stub function which will invoke the start_routine.

### Time Slices
using time command in the terminal, I did not notice a significant difference in the execution time. I assume the reason is that each task
for each thread is almost the same length so that round robin scheduling will not have much impalct on performance.

### Critical Sections
The critical sections include the atomic operations for thread control, including join, switch, create, yield, exit, suspend, and resume.
Those operations involves contxt switch and the thread may lost its state or corrupted if they can be interrupted during those operations.
