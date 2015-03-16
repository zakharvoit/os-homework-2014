#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <helpers.h>

#define BUFFER_SIZE 8192

void swap(char* a, char* b)
{
    char c = *a;
    *a = *b;
    *b = c;
}

void reverse(char* begin, char* end)
{
    int i;
    for (i = 0; i < (end - begin) / 2; i++) {
        swap(begin + i, end - i - 1);
    }
}

int output_reversed(char* buffer, ssize_t last_i, ssize_t i)
{
    reverse(buffer + last_i + (last_i != 0), buffer + i);
    if (write_(STDOUT_FILENO,
               buffer + last_i,
               i - last_i) < 0) {
        return -1;
    }
    return 0;
}

int main()
{
    char buffer[BUFFER_SIZE];
    ssize_t i, last_i;
    ssize_t count;

    while ((count = read_until(STDIN_FILENO,
                               buffer,
                               BUFFER_SIZE,
                               ' ')) != 0) {
        if (count < 0) {
            goto ERROR;
        }

        last_i = 0;
        for (i = 0; i < count; i++) {
            if (buffer[i] == ' ') {
                if (output_reversed(buffer, last_i, i) < 0) {
                    goto ERROR;
                }
                last_i = i;
            }
        }
        if (output_reversed(buffer, last_i, i) < 0) {
            goto ERROR;
        }
    }

    return EXIT_SUCCESS;

ERROR:
    perror("I/O error");
    return EXIT_FAILURE;
}
