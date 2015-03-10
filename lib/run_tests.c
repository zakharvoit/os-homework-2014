#include "helpers.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TMP_FILENAME ".tmpfile"
#define FILE_SIZE (1024 * 1024)
#define RAND_SEED 0xDEADBEEF

static char buffer[FILE_SIZE];

void fill_buffer()
{
    int i;
    srand(RAND_SEED);
    for (i = 0; i < FILE_SIZE; i++) {
        buffer[i] = rand();
    }
}

int write_file()
{
    int fd;

    if ((fd = creat(TMP_FILENAME, 0644)) < 0) {
        return -1;
    }

    if (write_(fd, buffer, FILE_SIZE) < 0) {
        return -1;
    }

    close(fd);

    return 0;
}

/**
 * Tests whether read file is the same as the written one
 * Returns: 0 if the files are the same, -1 in case of error, 1 otherwise
 */ 
int read_file()
{
    char read_buffer[FILE_SIZE];
    int fd;
    int count;

    if ((fd = open(TMP_FILENAME, O_RDONLY)) < 0) {
        return -1;
    }
    
    if ((count = read(fd, read_buffer, FILE_SIZE)) < 0) {
        return -1;
    }

    if (count != FILE_SIZE) {
        return 1;
    }

    if ((memcmp(read_buffer, buffer, FILE_SIZE)) != 0) {
        return 1;
    }

    return 0;
}

int main()
{
    int exit_code;

    fill_buffer();

    if (write_file() < 0) {
        goto ERROR;
    }

    if ((exit_code = read_file()) < 0) {
        goto ERROR;
    }

    if (exit_code == 0) {
        puts("Test passed");
    } else {
        puts("Test failed");
    }

    unlink(TMP_FILENAME);

    return exit_code;

ERROR:
    perror("Error while processing files");
    unlink(TMP_FILENAME);

    return EXIT_FAILURE;
}
