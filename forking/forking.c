#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <helpers.h>
#include <bufio.h>

#define LOG(args...) fprintf(stderr, args)
#define FAIL(args...) do { fprintf(stderr, args); exit(1); cleanup(); } while (0)

typedef struct
{
  char* port_number;
  struct addrinfo* info;
  int fd;
} serv_info;

#define NULL_INFO {                             \
    .port_number = NULL,                        \
    .info = NULL,                               \
    .fd = -1                                    \
    };                                          \


static serv_info first = NULL_INFO;
static serv_info second = NULL_INFO;

static int left_client = -1;
static int right_client = -1;

static void close_serv(serv_info* info)
{
  if (info->info) {
    freeaddrinfo(info->info);
  }

  if (info->fd != -1) {
    close(info->fd);
  }
}

static void cleanup(void)
{
  close_serv(&first);
  close_serv(&second);
  if (left_client != -1) {
    close(left_client);
  }
  if (right_client != -1) {
    close(right_client);
  }
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
  int err = getaddrinfo("localhost", server->port_number, NULL, &(server->info));
  if (err) {
    FAIL("Getaddrinfo: %s\n", gai_strerror(err));
  }
}

static void create_server(serv_info* server)
{
  parse_address_info(server);
  int sock = socket(server->info->ai_family,
                    server->info->ai_socktype,
                    server->info->ai_protocol);
  int family = server->info->ai_family;
  if (family == AF_INET6) {
    LOG("Using IPv6\n");
  } else if (family == AF_INET) {
    LOG("Using IPv4\n");
  } else {
    FAIL("Unknown family: %d\n", family);
  }
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
}

static void redirect(int from, int to)
{
  struct buf_t* buf = buf_new(4096);
  if (!buf) {
    FAIL("Cannot create buffer\n");
  }

  ssize_t size;
  while ((size = buf_fill(from, buf, 1))) {
    if (size < 0) {
      FAIL("Fill error: %s\n", strerror(errno));
    }

    if (buf_flush(to, buf, buf_size(buf)) < 0) {
      FAIL("Flush error: %s\n", strerror(errno));
    }
  }

  buf_free(buf);
}

static void process_clients(int left, int right)
{
  int err;
  if ((err = fork())) {
    if (err < 0) {
      FAIL("Fork: %s\n", strerror(errno));
    }

    close(left);
    close(right);
  } else {
    close(first.fd);
    close(second.fd);
    left_client = left;
    right_client = right;
    if ((err = fork())) {
      if (err < 0) {
        FAIL("Fork: %s\n", strerror(errno));
      }

      redirect(left_client, right_client);
    } else {
      redirect(right_client, left_client);
    }
    // here goes processing
    close(left_client);
    close(right_client);
    LOG("Ended clients %d %d\n", left_client, right_client);
    exit(0);
  }
}

static void main_loop(void)
{
  int client;
  LOG("Start listening clients\n");
  while ((client = accept(first.fd, NULL, NULL)) > 0) {
    LOG("Accepted %d\n", client);
    LOG("Waiting for the right hand client\n");
    int right = accept(second.fd, NULL, NULL);
    if (right < 0) {
      if (errno == EINVAL) {
        fprintf(stderr, "INVALINVALINVAL\n");
        FAIL("Accept right: %s\n", strerror(errno));
      }
    }
    process_clients(client, right);
  }
  if (errno == EINVAL) {
    fprintf(stderr, "INVAL Received\n");
  }
  FAIL("Accept: %s\n", strerror(errno));
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);
  create_server(&first);
  create_server(&second);
  main_loop();
  return 0;
}
