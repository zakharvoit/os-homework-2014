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
    char* middle_buffer = buffer + MIDDLE;
    char* last_word = middle_buffer;
    ssize_t count;

    while ((count = read_until(STDIN_FILENO,
                               middle_buffer,
                               MIDDLE, ' ')) != 0) {

        if (count < 0) {
            goto ERROR;
        }

        if ((last_word = revwords(last_word,
                                  middle_buffer + count)) == NULL) {
            goto ERROR;
        }

        count = middle_buffer + count - last_word;
        memmove(middle_buffer - count, last_word, count);
        last_word = middle_buffer - count;
    }

    reverse(last_word, middle_buffer);
    if (write_(STDOUT_FILENO, last_word, buffer - last_word + MIDDLE) < 0) {
        goto ERROR;
    }

    return EXIT_SUCCESS;

ERROR:
    perror("I/O error");
    return EXIT_FAILURE;
}
