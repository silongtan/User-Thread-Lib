#include "uthread.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#define SECOND 1000000
using namespace std;


void *slp(void *arg){
    static int i=1;
    unsigned long local_cnt = 0;
    sleep(1);
    if(i==2){
        uthread_suspend(6);
        // i++;
    }
    if(i==3){
        uthread_resume(6);
    }

    if(i==6){
        cout<<"Here\n";
    }
    cout<<"Sleep done\n";
    i++;
    unsigned long *return_buffer = new unsigned long;
    *return_buffer = local_cnt;
    return return_buffer;
}

int main(int argc, char *argv[]){
// Default to 1 ms time quantum
    int quantum_usecs = 1000;

    if (argc < 3) {
        cerr << "Usage: ./pi <total points> <threads> [quantum_usecs]" << endl;
        cerr << "Example: ./pi 100000000 8" << endl;
        exit(1);
    } else if (argc == 4) {
        quantum_usecs = atoi(argv[3]);
    }
    unsigned long totalpoints = atol(argv[1]);
    int thread_count = atoi(argv[2]);

    int *threads = new int[thread_count];
    int points_per_thread = totalpoints / thread_count;

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }

    srand(time(NULL));

    // Create threads
    for (int i = 0; i < thread_count-1; i++) {
        int tid = uthread_create(slp, nullptr);
        threads[i] = tid;
    }
    // uthread_join(threads[0], (void**)&local_cnt);
    for (int i = 0; i < thread_count; i++) {
        // Add thread result to global total
        unsigned long *local_cnt;
        uthread_join(threads[i], (void**)&local_cnt);

        // Deallocate thread result
        if (i < thread_count - 1) {
            delete local_cnt;
        }
    }
    cout<<"All done\n";
    return 0;
}
