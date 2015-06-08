#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <helpers.h>
#include <bufio.h>

#ifdef DEBUG
#define LOG(args...) fprintf(stderr, args)
#else
#define LOG(args...)
#endif
#define FAIL(args...) do { LOG(args); exit(1); cleanup(); } while (0)
#define ASSERT(c, args...) while (!(c)) FAIL(args);

#define MAX_FDS 256
#define BUFFER_SIZE 4096

typedef struct
{
  char* port_number;
  struct addrinfo* info;
  int fd;
  int fd_pos;
  int pending;
} serv_info;

#define CLOSED_IN 1
#define CLOSED_OUT (1 << 1)

typedef struct
{
  struct buf_t* buf;
  int closed_mask;
} buffer;

#define NULL_INFO {                             \
    .port_number = NULL,                        \
    .info = NULL,                               \
    .fd = -1,                                   \
    };                                          \

static serv_info first = NULL_INFO;
static serv_info second = NULL_INFO;

static struct pollfd fds[MAX_FDS];
static buffer bufs[MAX_FDS];
static nfds_t read_from[MAX_FDS];

static nfds_t size;

static void close_serv(serv_info* info)
{
  if (info->info) {
    freeaddrinfo(info->info);
  }

  if (info->fd != -1) {
    close(info->fd);
  }
}

static void close_in(nfds_t pos)
{
  fds[pos].events &= ~POLLIN;
  shutdown(fds[pos].fd, SHUT_RD);
  bufs[pos].closed_mask |= CLOSED_IN;
}

static void close_out(nfds_t pos)
{
  fds[pos].events &= ~POLLOUT;
  shutdown(fds[pos].fd, SHUT_WR);
  bufs[pos].closed_mask |= CLOSED_OUT;
}

static void cleanup(void)
{
  close_serv(&first);
  close_serv(&second);

  for (nfds_t i = 0; i < size; i++) {
    close(fds[i].fd);
  }
  for (nfds_t i = 0; i < MAX_FDS; i++) {
    if (bufs[i].buf) {
      buf_free(bufs[i].buf);
    } else {
      // We did not ever allocate next buffers
      break;
    }
  }
}

static nfds_t add_fd(int fd, int mask)
{
  read_from[size] = -1;
  if (!bufs[size].buf) {
    LOG("Creating buffer\n");
    bufs[size].buf = buf_new(BUFFER_SIZE);
    ASSERT(bufs[size].buf, "Malloc: %s", strerror(errno));
  }
  bufs[size].closed_mask = 0;
  fds[size].fd = fd;
  fds[size].events = mask;
  return size++;
}

static void remove_fd(int pos)
{
  close(fds[pos].fd);
  fds[pos] = fds[--size];
}

static void parse_args(int argc, char** argv)
{
  if (argc < 3) {
    FAIL("Usage: %s FIRST_PORT SECOND_PORT\n", argv[0]);
  }

  first.port_number = argv[1];
  second.port_number = argv[2];
}

static void parse_address_info(serv_info* server)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  int err = getaddrinfo("0.0.0.0", server->port_number,
                        &hints,
                        &(server->info));
  if (err) {
    FAIL("Getaddrinfo: %s\n", gai_strerror(err));
  }
}

static void create_server(serv_info* server)
{
  parse_address_info(server);
  int family = server->info->ai_family;
  if (family == AF_INET6) {
    LOG("Using IPv6\n");
  } else if (family == AF_INET) {
    LOG("Using IPv4\n");
  } else {
    FAIL("Unknown family: %d\n", family);
  }

  int sock = socket(server->info->ai_family,
                    server->info->ai_socktype,
                    server->info->ai_protocol);
  server->fd_pos = add_fd(sock, 0);
  if (sock < 0) {
    FAIL("Socket: %s\n", strerror(errno));
  }
  server->fd = sock;
  if (bind(sock,
           server->info->ai_addr,
           server->info->ai_addrlen) < 0) {
    FAIL("Bind: %s\n", strerror(errno));
  }

  if (listen(sock, 128) < 0) {
    FAIL("Listen: %s\n", strerror(errno));
  }
  server->pending = -1;
}

static void enable_server(serv_info* serv)
{
  fds[serv->fd_pos].events |= POLLIN;
}

static void disable_server(serv_info* serv)
{
  fds[serv->fd_pos].events &= ~POLLIN;
}

static void process_server(serv_info* serv, nfds_t event)
{
  int revs = fds[event].revents;
  if (revs & POLLIN) {
    LOG("Accepting on %d\n", fds[event].fd);
    int client = accept(serv->fd, NULL, NULL);
    ASSERT(client > 0, "Accept: %s", strerror(errno));
    serv->pending = client;
    disable_server(serv);
  }
}

static void process_client(nfds_t event)
{
  int revs = fds[event].revents;
  buffer* buf = bufs + event;
  if (revs & POLLIN) {
    LOG("Processing input for %d\n", fds[event].fd);
    ssize_t just_read;
    ASSERT((just_read = buf_fill(fds[event].fd, buf->buf, 1)) >= 0,
           "buf_fill: %s",
           strerror(errno));
    if (just_read == 0) {
      // socket closed
      LOG("Closing on input %d\n", fds[event].fd);
      close_in(event);
      close_out(read_from[event]);
    } else {
      fds[read_from[event]].events |= POLLOUT;
      if (buf_full(buf->buf)) {
        fds[event].events ^= POLLIN;
      }
    }
  } else if (revs & POLLOUT) {
    LOG("Processing output for %d\n", fds[event].fd);
    int fd_pos = read_from[event];
    struct buf_t* read_buf = bufs[fd_pos].buf;
    ssize_t just_wrote;
    ASSERT((just_wrote = buf_flush(fds[event].fd, read_buf, 1)) >= 0,
           "buf_flush: %s",
           strerror(errno));
    if (just_wrote == BUFIO_EOF) {
      LOG("Closing on output %d\n", fds[event].fd);
      close_out(event);
      close_in(read_from[event]);
    } else {
      fds[fd_pos].events |= POLLIN;
      if (buf_empty(read_buf)) {
        fds[event].events ^= POLLOUT;
      } else if (revs & POLLERR) {
        FAIL("Poll: %s", strerror(errno));
      }
    }
  }
}

static void main_loop(void)
{
  int err;
  LOG("Start polling servers\n");
  while ((err = poll(fds, size, -1)) >= 0) {
    LOG("Poll: %d new events\n", err);
    for (nfds_t i = 0; i < size; i++) {
      if (fds[i].fd == first.fd) {
        process_server(&first, i);
        if (first.pending != -1) {
          enable_server(&second);
        }
      } else if (fds[i].fd == second.fd) process_server(&second, i);
      else process_client(i);
    }
    if (first.pending != -1 && second.pending != -1) {
      nfds_t f_p = add_fd(first.pending, POLLIN | POLLOUT);
      nfds_t s_p = add_fd(second.pending, POLLIN | POLLOUT);
      read_from[f_p] = s_p;
      read_from[s_p] = f_p;

      enable_server(&first);
      disable_server(&second);
      first.pending = -1;
      second.pending = -1;
    }
    for (int i = 0; i < (int) size; i++) {
      if (bufs[i].closed_mask == (CLOSED_IN | CLOSED_OUT)) {
        LOG("Removing: %d\n", fds[i].fd);
        remove_fd(i);
        i--;
      }
    }
  }
  FAIL("Poll: %s\n", strerror(errno));
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);
  create_server(&first);
  create_server(&second);
  enable_server(&first);
  main_loop();
  return 0;
}
