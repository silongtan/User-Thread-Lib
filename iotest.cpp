#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>
#include "async_io.h"
#include "uthread.h"
#include <aio.h>
#include <errno.h>
#include <cassert>
#include <cstdlib>
#include <istream>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
#define UTHREAD_TIME_QUANTUM 10000

void* foo(void *arg) {
    for (int i = 0; i < 100000000; i++) {
    }

    return;
}

int main(int argc, char *argv[]) {

    int ret = uthread_init(UTHREAD_TIME_QUANTUM);
    if (ret != 0) {
        cerr << "Error: uthread_init" << endl;
        exit(1);
    }

    void *buf = new void*[2000];
    int offset = 0;
    int fd = open("test.txt", O_RDWR);
    int count = 2000;

    // normal read write

    auto start = chrono::steady_clock::now();

    std::string str(2000, '*');
    if (write(fd, &str, count) < 0) {
        cout << "write failed";
        return -1;
    }
    int th = uthread_create(foo, nullptr); 

    if (read(fd, buf, count) < 0) {
        cout << "read failed";
        return -1;
    }

    auto end = chrono::steady_clock::now();
    cout << "Elapsed time in nanoseconds : "
    << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
    << " ns" << endl;


    auto start = chrono::steady_clock::now();

    std::string str(2000, '*');
    if (async_write(fd, &str, count, 0) < 0) {
        cout << "write failed";
        return -1;
    }
    int th = uthread_create(foo, nullptr); 

    if (async_read(fd, buf, count, 0) < 0) {
        cout << "read failed";
        return -1;
    }

    auto end = chrono::steady_clock::now();
    cout << "Elapsed time in nanoseconds : "
    << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
    << " ns" << endl;

    return;
} 