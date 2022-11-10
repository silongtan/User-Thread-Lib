#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>

using namespace std;

TCB *curr; // current running thread
// Finished queue entry type
typedef struct finished_queue_entry {
  TCB *tcb;             // Pointer to TCB
  void *result;         // Pointer to thread result (output)
} finished_queue_entry_t;
finished_queue_entry_t finished;

// Join queue entry type
typedef struct join_queue_entry {
  TCB *tcb;             // Pointer to TCB
  int waiting_for_tid;  // TID this thread is waiting on
} join_queue_entry_t;
join_queue_entry_t join;


volatile int total_quantum = 0;
// You will need to maintain structures to track the state of threads
// - uthread library functions refer to threads by their TID so you will want
//   to be able to access a TCB given a thread ID
// - Threads move between different states in their lifetime (READY, BLOCK,
//   FINISH). You will want to maintain separate "queues" (doesn't have to
//   be that data structure) to move TCBs between different thread queues.
//   Starter code for a ready queue is provided to you
// - Separate join and finished "queues" can also help when supporting joining.
//   Example join and finished queue entry types are provided above

// Queues
static deque<TCB*> ready_queue;
static deque<finished_queue_entry_t> finished_queue;
static deque<TCB*> blocked_queue;
static deque<join_queue_entry_t> joined_queue;
//static deque<TCB*> finished_queue;

TCB* arr[MAX_THREAD_NUM];
// Interrupt Management --------------------------------------------------------
static struct sigaction _sigAction;
static bool interrupts_enabled = true;
struct itimerval timer;
// Start a countdown timer to fire an interrupt
static void startInterruptTimer()
{
        // TODO
	if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) == -1) {
		perror("setitimer error");
	}
}

// Block signals from firing timer interrupt
static void disableInterrupts()
{
        // TODO
	// sigaddset(&_context.uc_sigmask, SIGVTALRM);
	assert(interrupts_enabled);
	sigprocmask(SIG_BLOCK, &_sigAction.sa_mask, NULL);
	interrupts_enabled = false;
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
        // TODO
	// sigdelset(&_context.uc_sigmask, SIGVTALRM);
	assert(!interrupts_enabled);
    interrupts_enabled = true;
	sigprocmask(SIG_UNBLOCK, &_sigAction.sa_mask, NULL);
}

// Queue Management ------------------------------------------------------------

// Add TCB to the back of the ready queue
void addToReadyQueue(TCB *tcb) {
    ready_queue.push_back(tcb);
}

void addToFinishedQueue(finished_queue_entry_t tcb) {
    finished_queue.push_back(tcb);
}

void addToBlockedQueue(TCB *tcb) {
    blocked_queue.push_back(tcb);
}

void addToJoinQueue(join_queue_entry_t tcb) {
    joined_queue.push_back(tcb);
}
// Removes and returns the first TCB on the ready queue
// NOTE: Assumes at least one thread on the ready queue
TCB* popFromReadyQueue() {
	assert(!ready_queue.empty());

	TCB *ready_queue_head = ready_queue.front();
	ready_queue.pop_front();
	return ready_queue_head;
}

// Removes the thread specified by the TID provided from the ready queue
// Returns 0 on success, and -1 on failure (thread not in ready queue)
int removeFromReadyQueue(int tid) {
	for (deque<TCB*>::iterator iter = ready_queue.begin(); iter != ready_queue.end(); ++iter) {
		if (tid == (*iter)->getId()) {
				ready_queue.erase(iter);
				return 0;
		}
	}

	// Thread not found
	return -1;
}

int removeFromBlokcedQueue(int tid) {
	for (deque<TCB*>::iterator iter = blocked_queue.begin(); iter != blocked_queue.end(); ++iter) {
		if (tid == (*iter)->getId()) {
				blocked_queue.erase(iter);
				return 0;
		}
	}

	// Thread not found
	return -1;
}

int findFromFinishedQueue(int tid) {
    for (int i = 0; i < finished_queue.size(); i++) {
		if (tid == finished_queue[i].tcb->getId()) {
			return 0;
		}
	}
		
        // Thread not found
    return -1;
}

// Helper functions ------------------------------------------------------------

void* findRetInFinishedQueue(int tid) {
    for (int i = 0; i < finished_queue.size(); i++) {
		if (tid == finished_queue[i].tcb->getId()) {
			return finished_queue[i].result;
		}
	}
		
        // Thread not found
    return nullptr;
}

// Switch to the next ready thread
static void switchThreads() {
	// cout<<"Switch\n";
        // TODO
	volatile int flag = 0;
	if (!ready_queue.size()) {
		return;
	}
	TCB *tcb = popFromReadyQueue();
	int ret_val = getcontext(&curr->_context); // get context to current running thread
	if (ret_val < 0) {
		perror("getContext error");
		exit(1);
	}
	if(flag == 1) {
		startInterruptTimer();
		curr->increaseQuantum();
		return;
	}
	flag = 1;
	tcb->setState(RUNNING);
	curr = tcb;
	startInterruptTimer();
	tcb->increaseQuantum();
	setcontext(&tcb->_context);
}


// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do
// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg) {
        // TODO
	//  call the top-level thread function
    uthread_exit(start_routine(arg));
}

int uthread_init(int quantum_usecs) {
        // Initialize any data structures
	_sigAction.sa_handler = (sighandler_t)switchThreads;
    if (sigemptyset(&_sigAction.sa_mask) < -1) {
		perror("Failed to empty set");
		exit(1);
	}
	if (sigaddset(&_sigAction.sa_mask, SIGVTALRM)) {
		perror("Failed to add to set");
		exit(1);
	}
    _sigAction.sa_flags = 0;
        // Setup timer interrupt and handler
    if (sigaction(SIGVTALRM, &_sigAction, nullptr) < 0) {
		perror("sigaction error");
		return -1;
	}
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = quantum_usecs;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = quantum_usecs;
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) == -1) {
		perror("setitimer error");
		return -1;
	}
        // Create a thread for the caller (main) thread
	TCB *main = new TCB(0, nullptr, nullptr, RUNNING);
	curr = main;
	arr[0] = main;
	getcontext(&main->_context);
	main->increaseQuantum();
	return 0;
}

int uthread_create(void* (*start_routine)(void*), void* arg) {
	// Create a new thread and add it to the ready queue	
	static int id = 1;
	if(id <= MAX_THREAD_NUM) {
		// curr->disableInt();
		disableInterrupts();
		TCB *tcb = new TCB(id, start_routine, arg, READY);
		addToReadyQueue(tcb);
		arr[id] = tcb;
		id++;
		// curr->enableInt();
		enableInterrupts();
		return id-1;
	} else {
		return -1;
	}
}

int uthread_join(int tid, void **retval) {
        // If the thread specified by tid is still running, block caller until it terminates
	// cout<<"Join\n";
	if (interrupts_enabled){
		disableInterrupts();
	}
    if (arr[tid]->getState() == READY) {
		curr->setState(BLOCK);
		join.tcb = curr;
		join.waiting_for_tid = tid;
		addToJoinQueue(join);
		enableInterrupts();
		switchThreads(); // switch to next available thread
	}
	    // If the thread specified by tid is already terminated, clean up and return
    if (findFromFinishedQueue(tid) == 0) {
		TCB* terminated = arr[tid];
		if (findRetInFinishedQueue(tid) == nullptr){
			cout<<"Return NUll\n";
			return -1;
		}
		*retval = findRetInFinishedQueue(tid);
		terminated->~TCB();
	}
	return 0;
}

int uthread_yield(void) {
        // TODO
    cout<<"Yield\n";
	if (interrupts_enabled){
		disableInterrupts();
	}
	assert(!interrupts_enabled);
    if (ready_queue.empty()) {
		cout<<"No thread to run";
		return -1;
	}

	curr->setState(READY);
    addToReadyQueue(curr);
	enableInterrupts();
	assert(interrupts_enabled);
    switchThreads();
    return 0;
}

void uthread_exit(void *retval) {
	// Quit thread and clean up, wake up joiner if any
        // If this is the main thread, exit the program
	// cout<<"Exit\n";
	disableInterrupts();
	assert(!interrupts_enabled);
    if (uthread_self() == 0) {
		exit(0);
	}
        // Move any threads joined on this thread back to the ready queue
    for (int i = 0; i < joined_queue.size(); i++) {
		if (joined_queue[i].waiting_for_tid == curr->getId()) {
			joined_queue[i].tcb->setState(READY);
			addToReadyQueue(joined_queue[i].tcb);
			arr[joined_queue[i].tcb->getId()] = joined_queue[i].tcb;
		}
	}
	
        // Move this thread to the finished queue
    finished.tcb = curr;
    finished.result = retval;
    addToFinishedQueue(finished);
	total_quantum += finished.tcb->getQuantum();
	enableInterrupts();
	assert(interrupts_enabled);
    switchThreads();
}

int uthread_suspend(int tid) {
        // Move the thread specified by tid from whatever state it is
        // in to the block queue
	cout<<"Suspend\n";
    TCB* tcb = arr[tid];
	disableInterrupts();
	// reschedule time slice
	if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) == -1)
	{
		perror("setitimer reset error");
		// tcb->enableInt();
		enableInterrupts();
		return -1;
	} 
	removeFromReadyQueue(tid);
	tcb->setState(BLOCK);
	addToBlockedQueue(tcb);
	arr[tid] = tcb;
	enableInterrupts();
	if(curr->getId() == tid){
		switchThreads();
	}
	return 0;
}


int uthread_resume(int tid) {
        // Move the thread specified by tid back to the ready queue
	cout<<"Resume\n";
	disableInterrupts();
	TCB* tcb = arr[tid];
	removeFromBlokcedQueue(tid);
	addToReadyQueue(tcb);
	tcb->setState(READY);
	arr[tid] = tcb;
	enableInterrupts();
	return 0;
}

int uthread_self() {
        // TODO
	return curr->getId();
}

int uthread_get_total_quantums()
{
        // TODO
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) {
		if (arr[i] != nullptr){
			total_quantum += uthread_get_quantums(i);
		}
	}
	return total_quantum;
}

int uthread_get_quantums(int tid) {
        // TODO
	return arr[tid]->getQuantum();
}

