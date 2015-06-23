#include "poller.h"

#include <errno.h>
#include <stdio.h>

poller_t* poller_new(void)
{
  poller_t* res;
  res = malloc(sizeof(poller_t));
  if (!res) return NULL;

  memset(res, 0, sizeof(poller_t));
  return res;
}

void poller_free(poller_t* poller)
{
  free(poller);
}

int poller_add(poller_t* poller,
               int fd,
               handler_t handler,
               int* position,
               int events)
{
  if (poller->size == POLLER_MAX_FDS) {
    errno = ENOMEM;
    return -1;
  }
  poller->fds[poller->size].fd = fd;
  poller->fds[poller->size].events = events;
  poller->handlers[poller->size] = handler;
  poller->positions[poller->size] = position;
  *position = poller->size;
  ++poller->size;
  fprintf(stderr, "New size=%d\n", (int) poller->size);
  return 0;
}

void poller_remove(poller_t* poller,
                   int pos)
{
  fprintf(stderr, "Removing pos=%d fd=%d\n", pos, poller->fds[pos].fd);
  poller->fds[pos] = poller->fds[poller->size - 1];
  poller->handlers[pos] = poller->handlers[poller->size - 1];
  poller->positions[pos] = poller->positions[poller->size - 1];
  *(poller->positions[pos]) = pos;
  fprintf(stderr, "New on pos=%d fd=%d\n", pos, poller->fds[pos].fd);
  --poller->size;
  fprintf(stderr, "New size=%d\n", (int) poller->size);
}

int poller_run(poller_t* poller)
{
  int res = poll(poller->fds, (nfds_t) poller->size, -1);
  if (res < 0) {
    return -1;
  }
  for (size_t i = 0; i < poller->size; i++) {
    int revents;
    if ((revents = poller->fds[i].revents)) {
      poller->handlers[i].callback(poller->handlers[i].data, revents);
    }
  }
  return 0;
}

void poller_add_events(poller_t* poller,
                       int pos,
                       int events)
{
  poller->fds[pos].events |= events;
}

void poller_remove_events(poller_t* poller,
                          int pos,
                          int events)
{
  poller->fds[pos].events &= ~events;
}
