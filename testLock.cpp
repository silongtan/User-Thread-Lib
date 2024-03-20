#include "uthread.h"
#include "Lock.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;
#define RANDOM_YIELD_PERCENT 50

// shared variable
static int target = 0;

static Lock buffer_lock;

static int add_count = 0;
static int minus_count = 0;
static bool add_in_critical_section = false;
static bool minus_in_critical_section = false;


void* add(void *arg) {
  while (true) {
    buffer_lock.lock();


    // Make sure synchronization is working correctly
    assert(!add_in_critical_section);
    add_in_critical_section = true;

    // Place an item in the buffer
    target++;


    add_in_critical_section = false;
    buffer_lock.unlock();

    // Randomly give another thread a chance
    if ((rand() % 100) < RANDOM_YIELD_PERCENT) {
      uthread_yield();
    }
  }

  return nullptr;
}

void* decrease(void *arg) {
  while (true) {
    buffer_lock.lock();


    // Make sure synchronization is working correctly
    assert(!minus_in_critical_section);
    minus_in_critical_section = true;

    // Place an item in the buffer
    target--;


    minus_in_critical_section = false;
    buffer_lock.unlock();

    // Randomly give another thread a chance
    if ((rand() % 100) < RANDOM_YIELD_PERCENT) {
      uthread_yield();
    }
  }

  return nullptr;
}

int testLock() {

  int *add_threads = new int[10];
  for (int i = 0; i < 10; i++) {
    add_threads[i] = uthread_create(add, nullptr);
      if (add_threads[i] < 0) {
        cerr << "Error: uthread_create add" << endl;
      }
  }

  int *decrease_threads = new int[10];
  for (int i = 0; i < 10; i++) {
      decrease_threads[i] = uthread_create(decrease, nullptr);
      if (decrease_threads[i] < 0) {
          cerr << "Error: uthread_create minus" << endl;
      }
  }

  for (int i = 0; i < 10; i++) {
    int result = uthread_join(add_threads[i], nullptr);
    if (result < 0) {
      cerr << "Error: uthread_join add" << endl;
    }
  }

  for (int i = 0; i < 10; i++) {
    int result = uthread_join(add_threads[i], nullptr);
    if (result < 0) {
      cerr << "Error: uthread_join decrease" << endl;
    }
  }

  delete[] add_threads;
  delete[] decrease_threads;

  return 0;
}