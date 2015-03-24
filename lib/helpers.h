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

#endif 
