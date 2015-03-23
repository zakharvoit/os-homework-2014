#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <helpers.h>

#define BUFFER_SIZE 10000
#define MIDDLE (BUFFER_SIZE >> 1)
char len_buffer[20];

char* lenwords(char* begin, char* end)
{
    int len;
    char* next_space = begin - 1;

    for (; begin < end; begin = next_space + 1) {
        next_space = begin;
        len = 0;
        while (next_space < end
               && *next_space != ' ') {
            next_space++;
            len++;
        }

        if (next_space == end) {
            break;
        }

        sprintf(len_buffer, "%d\n", len);
        
        if (write_(STDOUT_FILENO,
                   len_buffer,
                   strlen(len_buffer)) < 0) {
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

        if ((last_word = lenwords(last_word,
                                  middle_buffer + count)) == NULL) {
            goto ERROR;
        }

        count = middle_buffer + count - last_word;
        memmove(middle_buffer - count, last_word, count);
        last_word = middle_buffer - count;
    }

    int last_len = middle_buffer - last_word;
    sprintf(len_buffer, "%d\n", last_len);
    if (write_(STDOUT_FILENO,
               len_buffer,
               strlen(len_buffer)) < 0) {
        goto ERROR;
    }

    return EXIT_SUCCESS;

 ERROR:
    perror("I/O error");
    return EXIT_FAILURE;
}
