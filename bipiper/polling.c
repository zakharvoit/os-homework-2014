#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <signal.h>

#include <bufio.h>

#include "poller.h"

#define LOG(args...) do { fprintf(stderr, args); } while (0)
#define FAIL(args...) do { LOG(args);                               \
    LOG("At line %d\n", __LINE__);                                  \
    exit(EXIT_FAILURE);                                             \
  } while (0)
#define ASSERT(c, args...) while (!(c)) { FAIL(args); }

typedef struct serv_info_t
{
  int                   fd;
  int                   pos;
  char*                 port_number;
  struct addrinfo*      info;
  struct serv_info_t*   another_serv;
  struct client_info_t* connected;
} serv_info;

typedef struct client_info_t
{
  int                   fd;
  int                   pos;
  struct buf_t*         buffer;
  struct client_info_t* another_client;
  int closed_mask;
} client_info;

client_info* deleted_clients[256];
size_t deleted_clients_n;

poller_t* poller;
serv_info first, second;

enum {
  SHUT_ON_READ = 1,
  SHUT_ON_WRITE = 2
};
int shut_type[] = { SHUT_ON_READ, SHUT_ON_WRITE };

static void shutdown_client(client_info* client, int end)
{
  int type = shut_type[end];
  client->closed_mask |= type;
  shutdown(client->fd, end);
  LOG("Mask %d\n", client->closed_mask);
  if (client->closed_mask == (SHUT_ON_READ | SHUT_ON_WRITE)) {
    LOG("Adding to delete %d %d\n", client->fd, client->another_client->fd);
    deleted_clients[deleted_clients_n++] = client;
    deleted_clients[deleted_clients_n++] = client->another_client;
  }
}

static void client_callback(client_info* client, int revents)
{
  LOG("Client callback for %d which is connected to %d\n", client->fd, client->another_client->fd);
  if (revents & POLLIN) {
    if (buf_full(client->buffer)) {
      poller_remove_events(poller, client->pos, POLLIN);
    } else {
      int new = buf_fill(client->fd, client->buffer, 1);
      if (new <= 0) {
        poller_remove_events(poller, client->pos, POLLIN);
        LOG("Shutting down RD %d\n", client->fd);
        shutdown_client(client, SHUT_RD);
        shutdown_client(client->another_client, SHUT_WR);
      } else {
        poller_add_events(poller, client->another_client->pos, POLLOUT);
      }
    }
  }

  if (revents & POLLOUT) {
    client_info* another = client->another_client;

    if (buf_empty(another->buffer)) {
      poller_remove_events(poller, client->pos, POLLOUT);
    } else {
      int new = buf_flush(client->fd, another->buffer, 1);
      if (new <= 0) {
        poller_remove_events(poller, client->pos, POLLOUT);
        LOG("Shutting down WR %d\n", client->fd);
        shutdown_client(client, SHUT_WR);
        shutdown_client(another, SHUT_RD);
      } else {
        poller_add_events(poller, client->another_client->pos, POLLIN);
      }
    }
  }
}

static void register_client(client_info* client)
{
  handler_t handler = {
    .callback = (void (*)(void*, int)) client_callback,
    .data     = (void*) client
  };
  poller_add(poller,
             client->fd,
             handler,
             &(client->pos),
             POLLIN | POLLOUT);
}

static void start_new_pair(void)
{
  first.connected->another_client = second.connected;
  second.connected->another_client = first.connected;

  register_client(first.connected);
  register_client(second.connected);
  first.connected = NULL;
  second.connected = NULL;
}

static void server_callback(serv_info* server, int revents)
{
  if (revents & POLLIN) {
    int client = accept(server->fd, NULL, NULL);
    LOG("Client connected to fd %d with server %d\n", client, server->fd);
    client_info* info = malloc(sizeof(client_info));
    ASSERT(info, "Not enough memory");
    info->fd = client;
    info->buffer = buf_new(4096);
    info->closed_mask = 0;
    ASSERT(info->buffer, "Not enough memory");
    server->connected = info;
    if (first.connected && second.connected) {
      start_new_pair();
    }
  }
  if (revents & POLLOUT) {
    ASSERT(0, "WTF");
  }
  if (revents & POLLHUP) {
    ASSERT(0, "WTF");
  }
}

static void register_server(serv_info* server)
{
  handler_t handler = {
    .callback = (void (*)(void*, int)) server_callback,
    .data     = (void*) server
  };
  poller_add(poller,
             server->fd,
             handler,
             &(server->pos),
             POLLIN);
}

static void create_server(serv_info* server)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  int err = getaddrinfo("0.0.0.0",
                        server->port_number,
                        &hints,
                        &(server->info));
  ASSERT(err == 0,
         "getaddrinfo: %s\n",
         gai_strerror(err));
  ASSERT((server->fd = socket(server->info->ai_family,
                              server->info->ai_socktype,
                              server->info->ai_protocol)) != -1,
         "socket: %s\n", strerror(errno));
  ASSERT((bind(server->fd,
               server->info->ai_addr,
               server->info->ai_addrlen)) == 0,
         "bind: %s\n", strerror(errno));

  ASSERT(listen(server->fd, 128) == 0,
         "listen: %s\n", strerror(errno));
}

static void parse_args(int argc, char** argv)
{
  ASSERT(argc == 3, "Usage: polling FIRST_PORT SECOND_PORT\n");
  first.port_number = argv[1];
  second.port_number = argv[2];
}

static int cmp_ptrs(void** a, void** b)
{
  if (*a < *b) return -1;
  else if (*a > *b) return 1;
  else return 0;
}

static void unregister_client(client_info* client)
{
  LOG("Unregistering %d\n", client->fd);
  close(client->fd);
  buf_free(client->buffer);
  poller_remove(poller, client->pos);
  free(client);
}

static void main_loop()
{
  poller = poller_new();
  ASSERT(poller, "poller_new: %s\n", strerror(errno));

  register_server(&first);
  register_server(&second);

  while (1) {
    LOG("New poll execution!\n");
    int err = poller_run(poller);
    ASSERT(!err, "poller_run: %s\n", strerror(errno));
    qsort(deleted_clients,
          deleted_clients_n,
          sizeof(void*),
          (int(*)(void const*, void const*)) cmp_ptrs);
    if (deleted_clients_n > 0) {
      unregister_client(*deleted_clients);
    }
    for (size_t i = 1; i < deleted_clients_n; i++) {
      if (deleted_clients[i] != deleted_clients[i - 1]) {
        unregister_client(deleted_clients[i]);
      }
    }
    deleted_clients_n = 0;
  }
}

void handle_sigpipe(int sig)
{
  (void) sig;
  // а пофиг
}

int main(int argc, char** argv)
{
  signal(SIGPIPE, handle_sigpipe);
  parse_args(argc, argv);
  create_server(&first);
  create_server(&second);
  first.another_serv = (serv_info*) &second;
  second.another_serv = (serv_info*) &first;

  main_loop();

  return EXIT_SUCCESS;
}
