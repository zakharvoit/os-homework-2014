#ifndef POLLER_H
#define POLLER_H

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>

#define POLLER_MAX_FDS 256

typedef struct
{
  void (*callback)(void* data, int revents);
  void* data;
} handler_t;

typedef struct
{
  struct pollfd fds[POLLER_MAX_FDS];
  handler_t     handlers[POLLER_MAX_FDS];
  int*          positions[POLLER_MAX_FDS];
  size_t        size;
} poller_t;

poller_t* poller_new(void);

void poller_free(poller_t* poller);

int poller_add(poller_t* poller,
               int fd,
               handler_t handler,
               int* position,
               int events);

int poller_run(poller_t* poller);

void poller_remove(poller_t* poller,
                   int pos);

void poller_add_events(poller_t*,
                       int pos, int events);
void poller_remove_events(poller_t*,
                          int pos, int events);

#endif
