#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <helpers.h>

#define BUFFER_SIZE 10000
#define MIDDLE (BUFFER_SIZE >> 1)

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

char* revwords(char* begin, char* end)
{
    char* next_space = begin - 1;

    for (; begin < end; begin = next_space + 1) {
        next_space = begin;
        while (next_space < end
               && *next_space != ' ') {
            next_space++;
        }

        if (next_space == end) {
            break;
        }

        reverse(begin, next_space);
        if (write_(STDOUT_FILENO,
                   begin, next_space - begin + 1) < 0) {
            return NULL;
        }
    }

    return begin;
}

int main()
{
    char buffer[BUFFER_SIZE];
    char* last_word = buffer + MIDDLE;
    ssize_t count;

    while ((count = read_until(STDIN_FILENO,
                               buffer + MIDDLE,
                               MIDDLE, ' ')) != 0) {

        if (count < 0) {
            goto ERROR;
        }

        if ((last_word = revwords(last_word,
                                  buffer + MIDDLE + count)) == NULL) {

            goto ERROR;
        }

        last_word -= count;

        memcpy(buffer + MIDDLE - count, buffer + MIDDLE, count);
    }

    reverse(last_word, buffer + MIDDLE);
    if (write_(STDOUT_FILENO, last_word, buffer - last_word + MIDDLE) < 0) {
        goto ERROR;
    }

    return EXIT_SUCCESS;

ERROR:
    perror("I/O error");
    return EXIT_FAILURE;
}
