#include "helpers.h"
#include "bufio.h"

#include <signal.h>

#include <stdlib.h>
#include <stdio.h>

const size_t MAX_SIZE = 4096;

#define QUIT(code) do { exit_code = code; goto EXIT; } while (0)

#define trace(args...) fprintf(stderr, args)

int print_prompt()
{
  return write_(STDOUT_FILENO, "$ ", 2);
}

static void signal_handler(int sig)
{
  (void)sig;
  write_(STDOUT_FILENO, "\n", 1);
  print_prompt();
}

int main()
{
  signal(SIGINT, signal_handler);
  int exit_code = EXIT_SUCCESS;
  struct buf_t* buf = buf_new(MAX_SIZE);
  char s[MAX_SIZE + 1];

  if (print_prompt() < 0) {
    QUIT(EXIT_FAILURE);
  }
  ssize_t size;
  while ((size = buf_getline(STDOUT_FILENO,
                             buf,
                             s,
                             MAX_SIZE))) {
    if (size < 0) {
      QUIT(EXIT_FAILURE);
    }

    s[size] = 0;

    char** piped_programs = split(s, '|', 0);
    if (!piped_programs) {
      goto ERROR;
    }
    char** p;
    for (p = piped_programs; *p; p++)
      ;
    int len = p - piped_programs;

    execargs_t** programs = malloc(len * sizeof(execargs_t*));
    if (!programs) {
      for (char** p = piped_programs; *p; p++) {
        free(*p);
      }
      free(piped_programs);
      goto ERROR;
    }

    for (int i = 0; i < len; i++) {
      programs[i] = execargs_parse(piped_programs[i]);
      if (!programs[i]) {
        for (int j = 0; j < i; j++) {
          free(programs[j]);
        }
        for (int j = 0; j < len; j++) {
          free(piped_programs[j]);
        }
        free(programs);
        free(piped_programs);
        goto ERROR;
      }
    }

    if (runpiped(programs, len) < 0) {
      goto ERROR;
    }

    if (print_prompt() < 0) {
      QUIT(EXIT_FAILURE);
    }

    continue;
  ERROR:
    fprintf(stderr, "Error\n");

    if (print_prompt() < 0) {
      QUIT(EXIT_FAILURE);
    }
  }

 EXIT:
  buf_free(buf);
  return exit_code;
}
