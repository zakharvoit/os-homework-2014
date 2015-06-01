#include "helpers.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ssize_t read_(int fd,
              void* buf,
              size_t count)
{
    size_t already_read = 0;

    while (already_read < count) {
        int new = read(fd,
                       (char*) buf + already_read,
                       count - already_read);

        if (new < 0) {
            return -1;
        } else if (new == 0) {
            return already_read;
        } else {
            already_read += new;
        }
    }

    return already_read;
}

ssize_t write_(int fd,
               const void* buf,
               size_t count)
{
    size_t already_wrote = 0;

    while (already_wrote < count) {
        int new = write(fd,
                        (char*) buf + already_wrote,
                        count - already_wrote);

        if (new < 0) {
            return -1;
        } else {
            already_wrote += new;
        }
    }

    return already_wrote;
}

ssize_t read_until(int fd,
                   void* buf,
                   size_t count,
                   char delimiter)
{
    size_t already_read = 0;
    size_t i = 0;

    while (already_read < count) {
        int new = read(fd,
                       (char*) buf + already_read,
                       count - already_read);

        if (new < 0) {
            return -1;
        } else if (new == 0) {
            return already_read;
        } else {
            already_read += new;
            for (; i < already_read; i++) {
                if (*((char*) buf + i) == delimiter) {
                    return already_read;
                }
            }
        }
    }

    return already_read;
}

int spawn(const char* file,
          char* const argv[])
{
    pid_t child;
    int status;

    if ((child = fork())) {
        if (child == -1) {
            return -1;
        }

        if (waitpid(child, &status, 0) < 0) {
            return -1;
        }

        if (status != 0) {
            return -1;
        }
    } else {
        if (execvp(file, argv) < 0) {
            _exit(-1);
        }
        _exit(0);
    }

    return 0;
}

static char* substring(char* s, size_t len)
{
  char* result = malloc(len + 1);
  if (!result) {
    return NULL;
  }
  memcpy(result, s, len);
  result[len] = 0;
  return result;
};

#define INCREASE_CAP(cap) cap = cap ? cap << 1 : 1;
#define PUSH_BACK(v, e, cap, size)                    \
  do {                                                \
    if (size + 1 > cap) {                             \
      INCREASE_CAP(cap);                              \
      char** new_v = realloc(v, sizeof(char*) * cap); \
      if (!new_v) {                                   \
        goto ERROR;                                   \
      }                                               \
      v = new_v;                                      \
    }                                                 \
    v[size++] = e;                                    \
  } while (0)

char** split(char* s, char c, _Bool skip_sep)
{
  char** result = NULL;
  size_t result_cap = 0;
  size_t result_n = 0;
  size_t len = strlen(s);
  char* buffer = malloc(len + 2);
  if (!buffer) {
    goto ERROR;
  }
  strcpy(buffer, s);
  buffer[len] = c;
  buffer[len + 1] = 0;
  char* last = buffer - 1;
  char* current = buffer;
  while ((current = strchr(current, c))) {
    if (!skip_sep || current != last + 1) {
      char* op = substring(last + 1, current - last - 1);
      if (!op) {
        goto ERROR;
      }
      PUSH_BACK(result, op, result_cap, result_n);
    }
    last = current;
    ++current;
  }

  PUSH_BACK(result, NULL, result_cap, result_n);

  free(buffer);
  return result;

 ERROR:
  free(buffer);
  free(result);
  return NULL;
}

execargs_t* execargs_parse(char* s)
{
  return split(s, ' ', 1);
}

void execargs_free(execargs_t* args)
{
  for (char** arg = args; *arg; arg++) {
    free(*arg);
  }
  free(args);
}

int runpiped(execargs_t** programs, int n)
{
  pid_t children[n];
  int input = STDIN_FILENO;
  int next_pipe[2];

  for (int i = 0; i < n; i++) {
    if (i < n - 1 && pipe(next_pipe) < 0) {
      return -1;
    } else if (i == n - 1) {
      next_pipe[1] = STDOUT_FILENO;
    }

    if ((children[i] = fork())) {
      if (children[i] < 0) {
        for (int j = 0; j < i; j++) {
          kill(children[j], SIGKILL);
        }

        if (input != STDIN_FILENO) {
          close(input);
        }
        if (next_pipe[1] != STDOUT_FILENO) {
          close(next_pipe[1]);
        }
        return -1;
      }

      if ((input != STDIN_FILENO && close(input) < 0)
          || (next_pipe[1] != STDOUT_FILENO && close(next_pipe[1]) < 0)) {
        return -1;
      }

      input = next_pipe[0];
    } else {
      int output = next_pipe[1];
      if ((i != 0 && i < n - 1 && close(next_pipe[0]) < 0)
          || dup2(input, STDIN_FILENO) < 0
          || (input != STDIN_FILENO && close(input) < 0)
          || dup2(output, STDOUT_FILENO) < 0
          || (output != STDOUT_FILENO && close(output) < 0)
          || (execvp(programs[i][0], programs[i]) < 0)
          ) {
        _exit(EXIT_FAILURE);
      }

      // not reachable
    }
  }

  for (int i = 0; i < n; i++) {
    int status;
    waitpid(children[i], &status, 0);
    if (WEXITSTATUS(status)) {
      return -1;
    }
  }

  return 0;
}
