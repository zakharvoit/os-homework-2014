#include "bufio.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct buf_t* buf_new(size_t capacity)
{
    struct buf_t* buffer = (struct buf_t*) malloc(sizeof(struct buf_t)
                                                  + capacity);
    if (buffer == NULL) {
        return NULL;
    }

    buffer->capacity = capacity;
    buffer->size = 0;

    return buffer;
}

void buf_free(struct buf_t* buffer)
{
    free(buffer);
}

size_t buf_capacity(struct buf_t* buffer)
{
    return buffer->capacity;
}

size_t buf_size(struct buf_t* buffer)
{
    return buffer->size;
}

ssize_t buf_fill(int fd,
                 struct buf_t* buf,
                 size_t required)
{
    ssize_t current = 0;
    size_t last_size = buf->size;

    while ((current = read(fd,
                          DATA(buf) + buf->size,
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
    ssize_t current = 0;
    size_t written = 0;

    while ((current = write(fd,
                           DATA(buf) + written,
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

    memmove(DATA(buf), DATA(buf) + written, buf->size - written);
    buf->size -= written;

    return written;
}
