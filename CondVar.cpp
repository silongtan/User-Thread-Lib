#include "CondVar.h"
#include "uthread_private.h"

// TODO
CondVar::CondVar() {
    lock_obj = NULL;
}

void CondVar::wait(Lock &lock) {
    disableInterrupts();
    // call wait -> put caller on signal queue -> unlock
    lock_obj = &lock;
    running->setState(BLOCK);
    signal_queue.push(running);
    lock._unlock();
    switchThreads();
    enableInterrupts();
}

void CondVar::signal() {
    disableInterrupts();
    if (lock_obj != NULL) {
        if (!signal_queue.empty()) {
            TCB *temp = signal_queue.front();
            signal_queue.pop();
            lock_obj->_signal(temp);
        }
    }
    enableInterrupts();
}

void CondVar::broadcast() {
    disableInterrupts();
    if (lock_obj != NULL) {
        for (int i = 0; i < signal_queue.size(); i++) {
            TCB *temp = signal_queue.front();
            signal_queue.pop();
            lock_obj->_signal(temp);
        }
    }
    enableInterrupts();
}