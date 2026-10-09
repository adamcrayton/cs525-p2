/*
 * Layer 3: I/O. The only file that knows sockets, clocks, and files exist.
 * Keep it thin: read an event, hand it to the state machine, do what it says.
 */
#define _POSIX_C_SOURCE 200809L

#include "lab.h"

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define EXIT_OK 0
#define EXIT_USAGE 1
#define EXIT_FAIL 2

#define HELLO_TIMEOUT_MS 1000
#define HELLO_ATTEMPTS 5
#define RECV_IDLE_MS 30000
#define RECV_LINGER_MS 2000

typedef struct
{
  bool is_sender;
  const char *session;
  uint32_t window;     /* default 8 */
  uint64_t timeout_ms; /* default 250 */
  double loss, corrupt, dup;
  const char *port;    /* default "4250" */
  const char *relay;
  const char *file;
} options_t;

static void usage(FILE *f)
{
  /* TODO: print the usage text from Task 1 exactly */
  fprintf(f, "Usage: myapp send ...\n       myapp recv ...\n");
}

/* Monotonic milliseconds. The ONLY place the clock is read. */
static uint64_t now_ms(void)
{
  /* TODO: clock_gettime(CLOCK_MONOTONIC, &ts); convert to ms */
  return 0;
}

/* Returns EXIT_OK, or EXIT_USAGE on a bad command line. */
static int parse_args(int argc, char **argv, options_t *o)
{
  /* TODO:
   *  - argv[1] is "send" or "recv"
   *  - getopt on the rest (reset optind appropriately, start at argv+1)
   *  - validate: session 1..32 chars [a-z0-9-], window 1..64,
   *    probabilities 0..0.5, recv must not accept -w/-T/-l/-c/-d
   *  - exactly two positionals: relay, file
   */
  (void)argc;
  (void)argv;
  (void)o;
  return EXIT_USAGE;
}

/* Resolve relay:port with getaddrinfo, create ONE UDP socket, connect() it
 * so plain send()/recv() go to the relay. Returns fd or -1. */
static int open_relay_socket(const char *host, const char *port)
{
  /* TODO */
  (void)host;
  (void)port;
  return -1;
}

/* Send hello, wait up to 1 s for a reply, retry up to 5 times.
 * Returns 0 on "OK"; prints the reason and returns -1 on ERR or silence. */
static int relay_hello(int fd, const char *hello)
{
  /* TODO: send(); poll(fd, HELLO_TIMEOUT_MS); recv(); compare to "OK" */
  (void)fd;
  (void)hello;
  return -1;
}

static int run_receiver(const options_t *o, int fd)
{
  /* TODO:
   *  - fopen output file
   *  - loop: poll with RECV_IDLE_MS (or linger time left once FIN seen)
   *      recv -> pkt_decode -> gbn_recv_on_packet
   *      write deliver bytes; send ACK if asked
   *      on fin: fclose, note linger deadline = now + RECV_LINGER_MS
   *  - linger expires -> EXIT_OK; idle 30 s -> EXIT_FAIL
   */
  (void)o;
  (void)fd;
  return EXIT_FAIL;
}

static int run_sender(const options_t *o, int fd)
{
  /* TODO:
   *  - read whole file into a malloc'd buffer (free on EVERY path)
   *  - gbn_send_init; gbn_send_fill; transmit the action
   *  - loop until done or gave_up:
   *      poll(fd, gbn_send_time_left(...))
   *      readable -> recv -> pkt_decode -> if ACK: gbn_send_on_ack
   *      timed out -> gbn_send_on_timeout
   *      transmit whatever seqs come back (gbn_send_build + pkt_encode + send)
   */
  (void)o;
  (void)fd;
  return EXIT_FAIL;
}

int main(int argc, char **argv)
{
  if (argc == 1)
  {
    usage(stdout);
    return EXIT_OK; /* make leak relies on this path being clean */
  }

  options_t o;
  if (parse_args(argc, argv, &o) != EXIT_OK)
  {
    usage(stderr);
    return EXIT_USAGE;
  }

  int fd = open_relay_socket(o.relay, o.port);
  if (fd < 0)
    return EXIT_FAIL;

  char hello[128];
  /* TODO: build "HELLO <session> recv" or
   *       "HELLO <session> send <loss> <corrupt> <dup>" with snprintf */
  hello[0] = '\0';

  int rc;
  if (relay_hello(fd, hello) != 0)
    rc = EXIT_FAIL;
  else
    rc = o.is_sender ? run_sender(&o, fd) : run_receiver(&o, fd);

  close(fd);
  (void)now_ms;
  return rc;
}