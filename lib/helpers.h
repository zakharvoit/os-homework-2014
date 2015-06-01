#ifndef _HELPERS_H
#define _HELPERS_H

#include <unistd.h>

ssize_t read_(int fd,
              void* buf,
              size_t count);

ssize_t write_(int fd,
               const void* buf,
               size_t count);

ssize_t read_until(int fd,
                   void* buf,
                   size_t count,
                   char delimiter);

int spawn(const char* file,
          char* const argv[]);

// So execargs_t* is char** (NULL terminated array of strings)
typedef char* execargs_t;

execargs_t* execargs_parse(char*);
void execargs_free(execargs_t*);

int runpiped(execargs_t** programs, int n);
char** split(char* s, char c, _Bool skip_seq);

#endif
