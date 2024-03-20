#include "async_io.h"
#include "uthread.h"
#include <aio.h>
#include <errno.h>

// TODO
ssize_t async_read(int fd, void *buf, size_t count, int offset) {
    struct aiocb *my_aio = {0};
    my_aio->aio_fildes = fd;
    my_aio->aio_buf = buf;
    my_aio->aio_nbytes = count;
    my_aio->aio_offset = offset;
    
    if (aio_read(my_aio) == -1) {
        return errno;
    }

    int ret_val = aio_error(my_aio);
    while (ret_val == EINPROGRESS) {
        uthread_yield();
    } // wait until the read completes

    if (ret_val == ECANCELED) {
        return errno;
    } 

    if (ret_val != 0) {
        return errno;
    }

    ssize_t result = aio_return(my_aio);
    if (result == -1) {
        return errno;
    }
    return result;
}

ssize_t async_write(int fd, void *buf, size_t count, int offset) {
    struct aiocb *my_aio = {0};
    my_aio->aio_fildes = fd;
    my_aio->aio_buf = buf;
    my_aio->aio_nbytes = count;
    my_aio->aio_offset = offset;
    
    if (aio_write(my_aio) == -1) {
        return errno;
    }

    int ret_val = aio_error(my_aio);
    while (ret_val == EINPROGRESS) {
        uthread_yield();
    } // wait until the read completes

    if (ret_val == ECANCELED) {
        return errno;
    } 

    if (ret_val != 0) {
        return errno;
    }

    ssize_t result = aio_return(my_aio);
    if (result == -1) {
        return errno;
    }
    return result;
}