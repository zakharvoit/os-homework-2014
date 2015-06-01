#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <helpers.h>
#include <bufio.h>

#define BUFFER_SIZE 10000
#define MIDDLE (BUFFER_SIZE >> 1)

struct process_data_t
{
    char* file;
    char** argv;
    int max_length;
};

typedef int(*string_processer_t)(char* buffer, void* data);

int for_each_line(const string_processer_t callback,
                  void* data)
{
    char dest[BUFFER_SIZE];
    struct buf_t* buf = buf_new(BUFFER_SIZE);
    ssize_t size;
    while ((size = buf_getline(STDIN_FILENO, buf, dest, 4096))) {
        if (size < 0) {
            return -1;
        }
        dest[size] = 0;
        callback(dest, data);
    }
    return 0;
}

int apply_crazy(char* buffer, void* data)
{
    struct process_data_t* process = (struct process_data_t*) data;
    char local_buffer[BUFFER_SIZE];
    int null_pos = 0;
    size_t buffer_len = strlen(buffer);
    int result = 0;

    if (buffer_len % 2 != 0) {
        return 0;
    }

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
        fprintf(stderr, "Usage: ./foreach <program> <arg0> <arg1> ... <argN>\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < argc - 1; i++) {
        argv_copy[i] = argv[i + 1];
    }

    argv_copy[argc - 1] = NULL;

    data.file = argv[1];
    data.argv = argv_copy;
    data.max_length = argc + 1;

    if (for_each_line(&apply_crazy, (void*) &data) < 0) {
        goto ERROR;
    }

    return EXIT_SUCCESS;

ERROR:
    perror("error");
    return EXIT_FAILURE;
}
