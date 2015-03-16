#include "helpers.h"

ssize_t read_(int fd,
              void* buf,
              size_t count)
{
    size_t already_read = 0;

    while (already_read < count) {
        int new = read(fd, 
                       (char*) buf + already_read, 
                       count - already_read);

        if (new < 0) {
            return -1;
        } else if (new == 0) {
            return already_read;
        } else {
            already_read += new;
        }
    }

    return already_read;
}

ssize_t write_(int fd,
               const void* buf,
               size_t count)
{
    size_t already_wrote = 0;

    while (already_wrote < count) {
        int new = write(fd,
                        (char*) buf + already_wrote,
                        count - already_wrote);

        if (new < 0) {
            return -1;
        } else {
            already_wrote += new;
        }
    }

    return already_wrote;
}

ssize_t read_until(int fd,
                   void* buf,
                   size_t count,
                   char delimiter)
{
    size_t already_read = 0;
    size_t i = 0;

    while (already_read < count) {
        int new = read(fd, 
                       (char*) buf + already_read, 
                       count - already_read);

        if (new < 0) {
            return -1;
        } else if (new == 0) {
            return already_read;
        } else {
            already_read += new;
            for (; i < already_read; i++) {
                if (*((char*) buf + i) == delimiter) {
                    return already_read;
                }
            }
        }
    }

    return already_read;
}
