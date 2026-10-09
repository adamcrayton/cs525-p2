#include "harness/unity.h" /* adjust to match the starter repo */
#include "../src/lab.h"

#include <stdlib.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/* ---------------- Layer 1: packets ---------------- */

void test_checksum_rfc1071_example(void)
{
  const uint8_t b[] = {0x00, 0x01, 0xf2, 0x03, 0xf4, 0xf5, 0xf6, 0xf7};
  TEST_ASSERT_EQUAL_HEX16(0x220d, inet_checksum(b, sizeof b));
}

void test_checksum_odd_length(void) { TEST_IGNORE(); }
void test_checksum_catches_single_bit_flip(void) { TEST_IGNORE(); }

void test_encode_hi_matches_worked_example(void)
{
  /* Expected bytes from worked example 2 */
  const uint8_t want[] = {0x00, 0x00, 0x96, 0x91, 0x00, 0x00, 0x00, 0x02,
                          0x00, 0x03, 0x48, 0x69, 0x21};
  (void)want;
  TEST_IGNORE(); /* TODO: build packet, pkt_encode, TEST_ASSERT_EQUAL_HEX8_ARRAY */
}

void test_encode_ack3_matches_worked_example(void) { TEST_IGNORE(); }
void test_decode_roundtrip(void) { TEST_IGNORE(); }
void test_decode_too_short(void) { TEST_IGNORE(); }
void test_decode_length_mismatch(void) { TEST_IGNORE(); }
void test_decode_length_over_1024(void) { TEST_IGNORE(); }
void test_decode_unknown_type(void) { TEST_IGNORE(); }
void test_decode_reserved_nonzero(void) { TEST_IGNORE(); }
void test_decode_bad_checksum(void) { TEST_IGNORE(); }

/* ---------------- Layer 2a: receiver ---------------- */

void test_recv_in_order_data(void) { TEST_IGNORE(); }
void test_recv_duplicate(void) { TEST_IGNORE(); }
void test_recv_beyond_gap(void) { TEST_IGNORE(); }
void test_recv_fin_in_order(void) { TEST_IGNORE(); }
void test_recv_repeated_fin(void) { TEST_IGNORE(); }

/* ---------------- Layer 2b: sender ---------------- */

void test_send_init_bad_window(void) { TEST_IGNORE(); }
void test_send_window_full(void) { TEST_IGNORE(); }
void test_send_cumulative_ack_slides_several(void) { TEST_IGNORE(); }
void test_send_duplicate_ack_ignored(void) { TEST_IGNORE(); }
void test_send_timeout_resends_window(void) { TEST_IGNORE(); }
void test_send_gives_up_after_10(void) { TEST_IGNORE(); }
void test_send_empty_file_is_single_fin(void) { TEST_IGNORE(); }
void test_send_exact_multiple_of_1024(void) { TEST_IGNORE(); }
void test_send_build_last_packet_short(void) { TEST_IGNORE(); }

/* ---------------- The big one ---------------- */

/*
 * Lossy in-memory channel: sender and receiver state machines exchange
 * ENCODED bytes. Each direction, per datagram, with a seeded RNG:
 *   drop 20% / flip one bit 20% / duplicate 20%.
 * Advance a fake clock; when nothing is in flight, jump "now" to the
 * sender's deadline and call gbn_send_on_timeout.
 * Assert: sender done, receiver saw FIN, delivered bytes == sent bytes.
 * Run it for several seeds and window sizes 1 and 16.
 */
void test_full_transfer_lossy_channel(void) { TEST_IGNORE(); }

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_checksum_rfc1071_example);
  RUN_TEST(test_checksum_odd_length);
  RUN_TEST(test_checksum_catches_single_bit_flip);
  RUN_TEST(test_encode_hi_matches_worked_example);
  RUN_TEST(test_encode_ack3_matches_worked_example);
  RUN_TEST(test_decode_roundtrip);
  RUN_TEST(test_decode_too_short);
  RUN_TEST(test_decode_length_mismatch);
  RUN_TEST(test_decode_length_over_1024);
  RUN_TEST(test_decode_unknown_type);
  RUN_TEST(test_decode_reserved_nonzero);
  RUN_TEST(test_decode_bad_checksum);
  RUN_TEST(test_recv_in_order_data);
  RUN_TEST(test_recv_duplicate);
  RUN_TEST(test_recv_beyond_gap);
  RUN_TEST(test_recv_fin_in_order);
  RUN_TEST(test_recv_repeated_fin);
  RUN_TEST(test_send_init_bad_window);
  RUN_TEST(test_send_window_full);
  RUN_TEST(test_send_cumulative_ack_slides_several);
  RUN_TEST(test_send_duplicate_ack_ignored);
  RUN_TEST(test_send_timeout_resends_window);
  RUN_TEST(test_send_gives_up_after_10);
  RUN_TEST(test_send_empty_file_is_single_fin);
  RUN_TEST(test_send_exact_multiple_of_1024);
  RUN_TEST(test_send_build_last_packet_short);
  RUN_TEST(test_full_transfer_lossy_channel);
  return UNITY_END();
}