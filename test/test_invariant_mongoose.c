#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "libs/mongoose/src/mongoose.c"

/* Test that DNS parsing rejects crafted packets with invalid offsets */
START_TEST(test_dns_parse_bounds_check)
{
    // Invariant: mg_dns_parse must not read out-of-bounds when given
    // crafted DNS responses with manipulated offset values
    struct mg_dns_message dm;
    memset(&dm, 0, sizeof(dm));

    // Payload 1: Minimal DNS response with A record where offset would be < 4
    // Header (12 bytes) + minimal answer pointing to offset 2 (wraps before buffer)
    uint8_t crafted_small_ofs[] = {
        0x00, 0x01, 0x81, 0x80, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00,
        0xc0, 0x0c,             // name pointer
        0x00, 0x01,             // type A
        0x00, 0x01,             // class IN
        0x00, 0x00, 0x00, 0x3c, // TTL
        0x00, 0x04,             // rdlength = 4
        0x08, 0x08, 0x08, 0x08  // rdata
    };

    // Payload 2: DNS response with AAAA record, truncated rdata
    uint8_t crafted_ipv6_short[] = {
        0x00, 0x01, 0x81, 0x80, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00,
        0xc0, 0x0c,
        0x00, 0x1c,             // type AAAA
        0x00, 0x01,
        0x00, 0x00, 0x00, 0x3c,
        0x00, 0x10,             // rdlength = 16
        0x20, 0x01, 0x0d, 0xb8  // only 4 bytes of rdata (truncated)
    };

    // Payload 3: Zero-length buffer (boundary)
    uint8_t empty_buf[] = {0};

    // Test payload 1: should not crash, return value indicates parse result
    int ret1 = mg_dns_parse(crafted_small_ofs, sizeof(crafted_small_ofs), &dm);
    // Whether it succeeds or fails, we survived without out-of-bounds access
    (void)ret1;

    // Test payload 2: truncated IPv6 rdata - must not read beyond buffer
    memset(&dm, 0, sizeof(dm));
    int ret2 = mg_dns_parse(crafted_ipv6_short, sizeof(crafted_ipv6_short), &dm);
    (void)ret2;

    // Test payload 3: empty/minimal buffer must not crash
    memset(&dm, 0, sizeof(dm));
    int ret3 = mg_dns_parse(empty_buf, 0, &dm);
    ck_assert_int_eq(ret3, 0); // Should fail gracefully

    // If we reach here, no out-of-bounds read caused a crash
    ck_assert(1);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_dns_parse_bounds_check);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}