#include <stdio.h>
#include <stdlib.h>
#include <bufio.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main()
{
    struct buf_t* buf = buf_new(BUFFER_SIZE);
    ssize_t written;

    while (buf_fill(STDIN_FILENO,
                    buf,
                    BUFFER_SIZE) >= BUFFER_SIZE) {
        written = buf_flush(STDOUT_FILENO,
                            buf,
                            buf->size);
        if (written < 0) {
            goto ERROR;
        }

        if (written < BUFFER_SIZE) {
            break;
        }
    }

    if ((written = buf_flush(STDOUT_FILENO,
                             buf,
                             buf->size)) < 0) {
        goto ERROR;
    }

    return EXIT_SUCCESS;

ERROR:
    perror("I/O error");
    return EXIT_FAILURE;
}
