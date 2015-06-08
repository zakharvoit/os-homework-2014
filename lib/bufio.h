#ifndef _BUFIO_H
#define _BUFIO_H

#include <sys/types.h>

struct buf_t
{
    size_t capacity;
    size_t size;
    char data[];
};

#define BUFIO_EOF -100500

struct buf_t* buf_new(size_t capacity);
void buf_free(struct buf_t* buffer);

size_t buf_capacity(struct buf_t* buffer);
size_t buf_size(struct buf_t* buffer);

void buf_clear(struct buf_t* buf);
_Bool buf_empty(struct buf_t* buf);
_Bool buf_full(struct buf_t* buf);

ssize_t buf_fill(int fd,
                 struct buf_t* buf,
                 size_t required);

ssize_t buf_flush(int fd,
                  struct buf_t* buf,
                  size_t required);

ssize_t buf_getline(int fd,
                    struct buf_t* buf,
                    char* dest,
                    size_t size);

#endif
