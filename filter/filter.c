#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <helpers.h>

#define BUFFER_SIZE 10000
#define MIDDLE (BUFFER_SIZE >> 1)

struct process_data_t
{
    char* file;
    char** argv;
    int max_length;
};

typedef int(*string_processer_t)(char* buffer, void* data);

char* process_lines(char* begin,
                    char* end,
                    string_processer_t callback,
                    void* data)
{
    char* next_line = begin - 1;

    for (; begin < end; begin = next_line + 1) {
        next_line = begin;
        while (next_line < end
                && *next_line != '\n') {
            next_line++;
        }

        if (next_line == end) {
            break;
        }

        *next_line = 0;
        callback(begin, data);
    }

    return begin;
}

int for_each_line(const string_processer_t callback,
                  void* data)
{
    char buffer[BUFFER_SIZE];
    char* middle_buffer = buffer + MIDDLE;
    char* last_line = middle_buffer;
    ssize_t count;

    while ((count = read_until(STDIN_FILENO,
                               middle_buffer,
                               MIDDLE, '\n')) != 0) {

        if (count < 0) {
            return -1;
        }

        if ((last_line = process_lines(last_line,
                                       middle_buffer + count + 1,
                                       callback,
                                       data)) == NULL) {
            return -1;
        }

        count = middle_buffer + count - last_line;
        memmove(middle_buffer - count, last_line, count);
        last_line = middle_buffer - count;
    }

    *middle_buffer = '\n';
    if (process_lines(last_line, middle_buffer + 1, callback, data) == NULL) {
        return -1;
    }

    return 0;
}

int filter(char* buffer, void* data)
{
    struct process_data_t* process = (struct process_data_t*) data;
    char local_buffer[BUFFER_SIZE];
    int null_pos = 0;
    size_t buffer_len = strlen(buffer);
    int result = 0;

#ifndef RUN_ON_EMPTY
    if (buffer_len == 0) {
        return 0;
    }
#endif

    memcpy(local_buffer, buffer, buffer_len + 1);
    local_buffer[buffer_len] = '\n';
    local_buffer[buffer_len + 1] = 0;

    while (null_pos < process->max_length
            && process->argv[null_pos] != NULL) {

        null_pos++;
    }

    if (null_pos >= process->max_length - 1) {
        errno = E2BIG;
        result = -1;
        goto EXIT;
    }

    process->argv[null_pos] = buffer;
    process->argv[null_pos + 1] = NULL;

    if (spawn(process->file, process->argv) == 0) {
        if (write_(STDOUT_FILENO, local_buffer, buffer_len + 1) < 0) {
            result = -1;
            goto EXIT;
        }
    }

EXIT:
    process->argv[null_pos] = NULL;

    return result;
}

int main(int argc,
         char* argv[])
{
    struct process_data_t data;
    char* argv_copy[argc + 1];
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./filter <program> <arg0> <arg1> ... <argN>\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < argc - 1; i++) {
        argv_copy[i] = argv[i + 1];
    }

    argv_copy[argc - 1] = NULL;

    data.file = argv[1];
    data.argv = argv_copy;
    data.max_length = argc + 1;

    if (for_each_line(&filter, (void*) &data) < 0) {
        goto ERROR;
    }

    return EXIT_SUCCESS;

ERROR:
    perror("error");
    return EXIT_FAILURE;
}
