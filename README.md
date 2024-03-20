## Overview
#### How to run:
```
./uthread-demo 100000000 <threads count> <optional time slice>
```
threads count < 100

### Known issues
1. Can only create 100 or less threads, no recycle mechanism. Thread number >100 will cause seg fault


### Additional Assumptions
1. One 100 threads could exist in this program.
2. No yield was called in the calculation
3. Resume is move the blocked (suspended) thread from blocked queue back to ready queue.
4. Suspend moves thread to blokced queue.

### Test cases
The test cases are stored in the test case folder.
there is addtional cpp files, copy the content to the main and run as normal to show the test result.

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

### Potential Improvements
One possible solution would be SJF: Shortest Job First. This would prevent other threads waiting for too long, which could achieve the 
minimum waiting time. One problem of this solution is that the longest thread may have to wait utill all other threads finished. In addition,
we do not know how to implement an algorithm to calculate the length of a thread excution.
