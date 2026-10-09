#ifndef LAB_H
#define LAB_H

/*
 * CS 425 P2 - Go-Back-N over UDP
 *
 * Three layers (Task 4):
 *   1. Packets         - pure functions: checksum, encode, decode/validate
 *   2. State machines  - Go-Back-N sender and receiver. NO sockets, clocks,
 *                        files, or printing. Time is passed in as a number.
 *   3. I/O             - lives in main.c only (socket, hello, poll, clock, file)
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Layer 1: Packets                                                    */
/* ------------------------------------------------------------------ */

#define PKT_HDR_LEN 10
#define PKT_MAX_PAYLOAD 1024
#define PKT_MAX_LEN (PKT_HDR_LEN + PKT_MAX_PAYLOAD) /* 1034 */

typedef enum
{
  PKT_DATA = 0,
  PKT_ACK = 1,
  PKT_FIN = 2
} pkt_type_t;

/* In-memory form of a packet. This struct is NEVER memcpy'd onto the wire. */
typedef struct
{
  uint8_t type;     /* pkt_type_t */
  uint32_t seq;     /* packet index (DATA/FIN) or next expected (ACK) */
  uint16_t length;  /* payload bytes, 0..PKT_MAX_PAYLOAD */
  uint8_t payload[PKT_MAX_PAYLOAD];
} packet_t;

/**
 * RFC 1071 Internet checksum over buf[0..len).
 * Odd length: pad with one zero byte for the calculation only.
 * Returns the one's complement of the one's complement sum.
 */
uint16_t inet_checksum(const uint8_t *buf, size_t len);

/**
 * Serialize p into out (network byte order), filling in the checksum.
 * Returns the number of bytes written (10 + p->length), or 0 if p is
 * invalid or out is too small.
 */
size_t pkt_encode(const packet_t *p, uint8_t *out, size_t outlen);

/**
 * Parse and validate a received datagram of exactly len bytes.
 * Returns true and fills *out only if every validation rule passes:
 *   len >= 10; 10 + length == len; length <= 1024;
 *   type in {0,1,2}; reserved == 0; checksum verifies.
 * Returns false otherwise (caller treats it as lost).
 */
bool pkt_decode(const uint8_t *buf, size_t len, packet_t *out);

/* Convenience constructor for an ACK (length 0). */
void pkt_make_ack(packet_t *p, uint32_t seq);

/* ------------------------------------------------------------------ */
/* Layer 2a: Go-Back-N receiver                                        */
/* ------------------------------------------------------------------ */

typedef struct
{
  uint32_t expected; /* index of the next packet wanted */
  bool fin_seen;     /* true once the in-order FIN has been accepted */
} gbn_receiver_t;

/* What the I/O layer must do after the receiver handles one packet. */
typedef struct
{
  bool send_ack;           /* send ACK ack_seq? */
  uint32_t ack_seq;
  const uint8_t *deliver;  /* bytes to append to the file, or NULL */
  uint16_t deliver_len;
  bool fin;                /* in-order FIN accepted: close file, start linger */
} recv_action_t;

void gbn_recv_init(gbn_receiver_t *r);

/**
 * Feed one VALID packet (already passed pkt_decode) to the receiver.
 * Implements Task 2's three rules. ACK packets arriving here are ignored.
 * deliver points into p->payload, so use it before p is reused.
 */
recv_action_t gbn_recv_on_packet(gbn_receiver_t *r, const packet_t *p);

/* ------------------------------------------------------------------ */
/* Layer 2b: Go-Back-N sender                                          */
/* ------------------------------------------------------------------ */

#define GBN_MAX_WINDOW 64
#define GBN_MAX_TIMEOUTS 10

typedef struct
{
  const uint8_t *data; /* whole file in memory (<= 16 MiB); not owned */
  size_t data_len;
  uint32_t num_data;   /* number of DATA packets; FIN seq == num_data */

  uint32_t window;     /* N, 1..64 */
  uint64_t timeout_ms;

  uint32_t base;       /* oldest unacked packet */
  uint32_t next;       /* next packet never sent */

  bool timer_running;
  uint64_t timer_deadline_ms;

  int timeouts_in_a_row;
  bool done;           /* FIN acked: exit 0 */
  bool gave_up;        /* 10 timeouts with no progress: exit 2 */
} gbn_sender_t;

/* Packet indices the I/O layer must transmit now, in order. */
typedef struct
{
  uint32_t seqs[GBN_MAX_WINDOW + 1]; /* +1 leaves room for the FIN */
  size_t count;
} send_action_t;

/**
 * Set up a sender over data[0..len). Does not send anything.
 * Returns false if window or arguments are out of range.
 */
bool gbn_send_init(gbn_sender_t *s, const uint8_t *data, size_t len,
                   uint32_t window, uint64_t timeout_ms);

/**
 * Rule 1 (and the FIN rule): send while the window has room.
 * Appends seqs to *out and starts the timer if it was not running.
 */
void gbn_send_fill(gbn_sender_t *s, uint64_t now_ms, send_action_t *out);

/**
 * Rule 2: an ACK arrived. Slide base if ack_seq > base, otherwise ignore.
 * Restart/stop the timer; reset timeouts_in_a_row on progress; set done
 * when the FIN is acknowledged. Then refills the window into *out.
 */
void gbn_send_on_ack(gbn_sender_t *s, uint32_t ack_seq, uint64_t now_ms,
                     send_action_t *out);

/**
 * Rule 3: the timer expired. Resend base..next-1 into *out, restart the
 * timer, count the timeout, and set gave_up after GBN_MAX_TIMEOUTS.
 */
void gbn_send_on_timeout(gbn_sender_t *s, uint64_t now_ms, send_action_t *out);

/**
 * Milliseconds from now_ms until the timer fires: 0 if already due,
 * -1 if the timer is not running. The I/O layer passes this to poll().
 */
int64_t gbn_send_time_left(const gbn_sender_t *s, uint64_t now_ms);

/**
 * Build packet number seq (DATA, or FIN when seq == num_data) into *p.
 * Because the whole file is in memory, retransmissions are rebuilt
 * from data instead of keeping separate copies.
 */
void gbn_send_build(const gbn_sender_t *s, uint32_t seq, packet_t *p);

#ifdef __cplusplus
}
#endif

#endif /* LAB_H */