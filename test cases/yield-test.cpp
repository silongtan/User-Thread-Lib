#include "uthread.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#define SECOND 1000000
using namespace std;


void *slp(void *arg){
	static int i=0;
	sleep(2);
	if(i==2){
		i++;
		uthread_yield();//sleep 2 is yielded
	}
	cout<<"Sleep "<<i<<" done\n";
	i++;
	return nullptr;
}

int main(int argc, char *argv[]){
	// Default to 1 ms time quantum
    int quantum_usecs = 1000;

    if (argc < 2) {
        cerr << "Usage: ./pi <total points> [quantum_usecs]" << endl;
        cerr << "Example: ./pi 100000000 8" << endl;
        exit(1);
    } else if (argc == 3) {
        quantum_usecs = atoi(argv[3]);
    }
    unsigned long totalpoints = atol(argv[1]);
    int thread_count = 100;

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
    unsigned long *local_cnt;
    uthread_join(threads[0], (void**)&local_cnt);
    cout<<"All done\n";
    return 0;
}
