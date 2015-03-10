#include <stdio.h>
#include <stdlib.h>
#include <helpers.h>

#define BUFFER_SIZE 4096

int main()
{
    char buffer[BUFFER_SIZE];
    ssize_t count;

    while ((count = read_(STDIN_FILENO, buffer, BUFFER_SIZE)) != 0) {
        if (count < 0) {
            goto ERROR;
        }
        if (write_(STDOUT_FILENO, buffer, count) < 0) {
            goto ERROR;
        }
    }

    return EXIT_SUCCESS;

ERROR:
    perror("Error while processing files");

    return EXIT_FAILURE;
}
