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

#ifdef DEBUG
#define LOG(args...) fprintf(stderr, args)
#else
#define LOG(args...)
#endif
#define FAIL(args...) do { fprintf(stderr, args); exit(1); cleanup(); } while (0)

static char* port_number;
static char* filename;
static struct addrinfo* info;
static int server = -1;
static int client = -1;

static void cleanup(void)
{
  if (info) {
    freeaddrinfo(info);
  }
  if (server != -1) {
    close(server);
  }
  if (client != -1) {
    close(client);
  }
}

static void parse_args(int argc, char** argv)
{
  if (argc < 3) {
    FAIL("Usage: %s PORT FILENAME\n", argv[0]);
  }

  port_number = argv[1];
  filename = argv[2];
}

static void parse_address_info(void)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

  int err = getaddrinfo("localhost", port_number, &hints, &info);
  if (err) {
    FAIL("Getaddrinfo: %s\n", gai_strerror(err));
  }
}

static void create_server(void)
{
  int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  LOG("INFO: %d == %d\n", info->ai_family, AF_INET6);
  if (sock < 0) {
    FAIL("Socket: %s\n", strerror(errno));
  }
  server = sock;
  if (bind(sock, info->ai_addr, info->ai_addrlen) < 0) {
    FAIL("Bind: %s\n", strerror(errno));
  }

  if (listen(server, 128) < 0) {
    FAIL("Listen: %s\n", strerror(errno));
  }
}


#define ASSERT(c) if (!(c)) do { error = strerror(errno); goto EXIT; } while (0)
static void write_file_to_fd(char* name, int fd)
{
  char* error = NULL;
  int file = open(name, O_RDONLY);
  if (file < 0) {
    FAIL("Open: %s\n", strerror(errno));
  }

  struct buf_t* buf = buf_new(4096);

  ssize_t size;
  while ((size = buf_fill(file, buf, 128))) {
    ASSERT(size >= 0);
    ASSERT(buf_flush(fd, buf, buf_size(buf)) >= 0);
  }

 EXIT:
  buf_free(buf);
  close(file);
  if (!error) {
    return;
  }
  FAIL("Writing file: %s\n", error);
}
#undef ASSERT

static void process_client(int client_)
{
  int err;
  if ((err = fork())) {
    if (err < 0) {
      FAIL("Fork: %s\n", strerror(errno));
    }

    close(client_);
  } else {
    close(server);
    client = client_;
    write_file_to_fd(filename, client);
    close(client);
    LOG("Ended client %d\n", client);
    exit(0);
  }
}

static void main_loop(void)
{
  int client;
  LOG("Start listening clients\n");
  while ((client = accept(server, NULL, NULL)) > 0) {
    LOG("Accepted %d\n", client);
    process_client(client);
  }
  FAIL("Accept: %s\n", strerror(errno));
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);
  parse_address_info();
  create_server();
  main_loop();
  return 0;
}
