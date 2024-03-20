#include "Lock.h"
#include "uthread_private.h"

#define BUSY 1
#define FREE 0
// TODO


Lock::Lock() {
    lock_flag = FREE;  // initialze the lock
    std::queue<TCB*> lock_queue;
    signaller = NULL;
}

void Lock::lock() {
    disableInterrupts();
    // attempt to acqurie
    if (lock_flag == BUSY) {
        running->setState(BLOCK);
        lock_queue.push(running);
        switchThreads();
    } else {
        lock_flag = BUSY;
        running->increaseLockCount();
    }
    enableInterrupts();
}

void Lock::unlock() { // switch to the thread that called signal()
    disableInterrupts();
    _unlock();
    enableInterrupts();
}

void Lock::_unlock() {
    if (signaller != NULL) { // switch to the thread that called signal()
        addToReady(running);
        TCB *temp = signaller;
        signaller = NULL;
        running->decreaseLockCount();
        switchToThread(temp);
    } else if (!lock_queue.empty()) { //switch to the waiting thread
		TCB *wait = lock_queue.front();
		lock_queue.pop();
        wait->setState(READY);
        addToReady(wait);
        running->decreaseLockCount();
	} else {
        running->decreaseLockCount();
        lock_flag = FREE;
    }
}

void Lock::_signal(TCB *tcb) {
    signaller = running;
    running->decreaseLockCount();
    switchToThread(tcb);
}

