#include "uthread.h"
#include "Lock.h"
#include "CondVar.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define SHARED_BUFFER_SIZE 10
#define PRINT_FREQUENCY 100000
#define RANDOM_YIELD_PERCENT 50

// Shared buffer
static int buffer[SHARED_BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int item_count = 0;

// Shared buffer synchronization
static Lock buffer_lock;

void* low(void *arg) {
  while (true) {
    buffer_lock.lock();
    for (int i = 0; i < 50000000; i++) {
    
    }
    cout << "Low\n";
    uthread_yield();
    for (int i = 0; i < 50000000; i++) {

    }
    cout << "Low\n";
    buffer_lock.unlock();
  }

  return nullptr;
}

void* medium(void *arg) {
  while (true) {
    for (int i = 0; i < 100000000; i++) {
        
    }
    cout << "Medium\n";
  }

  return nullptr;
}

void* high(void *arg) {
  while (true) {
    buffer_lock.lock();
    for (int i = 0; i < 100000000; i++) {
        
    }
    cout << "High\n";
    buffer_lock.unlock();
  }

  return nullptr;
}

int priority_test() {
  // Init user thread library
  int ret = uthread_init(UTHREAD_TIME_QUANTUM);
  if (ret != 0) {
    cerr << "Error: uthread_init" << endl;
    exit(1);
  }

  // Create producer threads
  int green = uthread_create_low(low, nullptr);
  if (green < 0) {
    cerr << "Error: uthread_create low" << endl;
  }
  

  int orange1 = uthread_create(medium, nullptr); 
  if (orange1 < 0) {
    cerr << "Error: uthread_create medium" << endl;
  }

  int orange2 = uthread_create(medium, nullptr); 
  if (orange2 < 0) {
    cerr << "Error: uthread_create medium" << endl;
  }

  int red = uthread_create_high(high, nullptr); 
  if (red < 0) {
    cerr << "Error: uthread_create high" << endl;
  }

  // make those threads run in sequence
  return 0;
}

// without boost: low -> medium -> medium -> low -> high
// Expected behavior: 
// cannot guarantee because of randomness
// low -> low -> high -> medium -> medium