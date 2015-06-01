#include "bufio.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef DEBUG
#define NDEBUG
#endif
#include <assert.h>

struct buf_t* buf_new(size_t capacity)
{
    struct buf_t* buffer = malloc(sizeof(struct buf_t) + capacity);
    if (buffer == NULL) {
        return NULL;
    }

    buffer->capacity = capacity;
    buffer->size = 0;

    return buffer;
}

void buf_free(struct buf_t* buffer)
{
    assert(buffer);
    free(buffer);
}

size_t buf_capacity(struct buf_t* buffer)
{
    assert(buffer);
    return buffer->capacity;
}

size_t buf_size(struct buf_t* buffer)
{
    assert(buffer);
    return buffer->size;
}

ssize_t buf_fill(int fd,
                 struct buf_t* buf,
                 size_t required)
{
    assert(buf && required <= buf->capacity - buf->size);

    ssize_t current = 0;
    size_t last_size = buf->size;

    while ((current = read(fd,
                           buf->data + buf->size,
                           buf->capacity - buf->size))) {
        if (current < 0) {
            buf->size = last_size;
            return -1;
        }
        buf->size += current;
        if (buf->size >= required) {
            break;
        }
    }

    return buf->size;
}

ssize_t buf_flush(int fd,
                  struct buf_t* buf,
                  size_t required)
{
    assert(buf);

    ssize_t current = 0;
    size_t written = 0;

    while ((current = write(fd,
                            buf->data + written,
                            buf->size - written))) {
        if (current < 0) {
            /* Buffer state undefined ? */
            return -1;
        }

        written += current;
        if (written >= required) {
            break;
        }
    }

    memmove(buf->data, buf->data + written, buf->size - written);
    buf->size -= written;

    return written;
}

ssize_t buf_getline(int fd,
                    struct buf_t* buf,
                    char* dest,
                    size_t size)
{
    assert(buf);

    ssize_t current = 0;
    ssize_t new_size = 0;
    size_t i = 0;

    do {
        int finished = 0;
        for (i = 0; i < buf->size; i++) {
            if (buf->data[i] == '\n') {
                if (new_size + i > size) {
                    return -1;
                }
                memcpy(dest + new_size, buf->data, i);
                memmove(buf->data, buf->data + i + 1, buf->size - i - 1);
                buf->size -= i + 1;
                new_size += i;
                finished = 1;
                break;
            }
        }
        if (finished) {
            break;
        }
        if (current < 0) {
            return -1;
        }
        memcpy(dest + new_size, buf->data, buf->size);
        new_size += buf->size;
        buf->size = 0;
    } while ((current = buf_fill(fd, buf, 1)));

    return new_size;
}
