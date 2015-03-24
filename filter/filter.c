#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <helpers.h>

#define BUFFER_SIZE 10000
#define MIDDLE (BUFFER_SIZE >> 1)
#define MAX_ARGV_LENGTH 256

struct process_data_t
{
    char* file;
    char* argv[MAX_ARGV_LENGTH];
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
    char* argv[MAX_ARGV_LENGTH];
    char local_buffer[BUFFER_SIZE];
    size_t null_pos = 0;
    size_t buffer_len = strlen(buffer);

#ifndef RUN_ON_EMPTY
    if (buffer_len == 0) {
        return 0;
    }
#endif

    memcpy(local_buffer, buffer, buffer_len + 1);
    local_buffer[buffer_len] = '\n';
    local_buffer[buffer_len + 1] = 0;

    memcpy(argv, process->argv, sizeof(argv));
    while (null_pos < MAX_ARGV_LENGTH && argv[null_pos] != NULL) {
        null_pos++;
    }

    if (null_pos >= MAX_ARGV_LENGTH - 1) {
        errno = E2BIG;
        return -1;
    }

    argv[null_pos] = buffer;
    argv[null_pos + 1] = NULL;

    if (spawn(process->file, argv) == 0) {
        if (write_(STDOUT_FILENO, local_buffer, buffer_len + 1) < 0) {
            return -1;
        }
    }

    return 0;
}

int main(int argc,
         char* argv[])
{
    struct process_data_t data;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./filter <program> <arg0> <arg1> ... <argN>\n");
        return EXIT_FAILURE;
    }

    data.file = argv[1];
    memcpy(data.argv,
           argv + 1,
           (argc - 1) * sizeof(char*));
    data.argv[argc - 1] = NULL;

    if (for_each_line(&filter, (void*) &data) < 0) {
        perror("I/O error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
