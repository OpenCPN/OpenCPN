
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <dnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#define ADDR_PACK(a, ip)		\
	(a)->addr_type = ADDR_TYPE_IP;	\
	(a)->addr_bits = IP_ADDR_BITS;	\
	(a)->addr_ip = (ip)

#ifdef HAVE_SOCKADDR_SA_LEN
#define SIN_PACK(s, ip, port)				\
	(s)->sin_len = sizeof(struct sockaddr_in);	\
	(s)->sin_family = AF_INET;			\
	(s)->sin_port = htons(port);			\
	(s)->sin_addr.s_addr = (ip)
#else
#define SIN_PACK(s, ip, port)				\
	(s)->sin_family = AF_INET;			\
	(s)->sin_port = htons(port);			\
	(s)->sin_addr.s_addr = (ip)
#endif

typedef struct sockaddr SA;

START_TEST(test_addr_pack)
{
	struct addr a, b;

	memset(&a, 0, sizeof(a)); memset(&b, 0, sizeof(b));

	ADDR_PACK(&a, 666);
	addr_pack(&b, ADDR_TYPE_IP, IP_ADDR_BITS, &a.addr_ip, IP_ADDR_LEN);
	fail_unless(memcmp(&a, &b, sizeof(a)) == 0, "got different address");
}
END_TEST
	
START_TEST(test_addr_cmp)
{
	struct addr a, b;

	ADDR_PACK(&a, 666);
	memcpy(&b, &a, sizeof(a));
	fail_unless(addr_cmp(&a, &b) == 0, "failed on equal addresses");
	b.addr_type = ADDR_TYPE_ETH;
	fail_unless(addr_cmp(&a, &b) != 0, "failed on different addr_type");
	memcpy(&b, &a, sizeof(a)); b.addr_bits--;
	fail_unless(addr_cmp(&a, &b) > 0, "failed on lesser addr_bits");
	memcpy(&b, &a, sizeof(a)); b.addr_ip--;
	fail_unless(addr_cmp(&a, &b) != 0, "failed on different addr_ip");

	addr_aton("10.0.0.1", &a);
	addr_aton("10.0.0.2", &b);
	fail_unless(addr_cmp(&a, &b) < 0, "failed on lesser addr compare");
	fail_unless(addr_cmp(&b, &a) > 0, "failed on greater addr compare");
}
END_TEST

START_TEST(test_addr_bcast)
{
	struct addr a, b;

	ADDR_PACK(&a, htonl(0x01020304));
	a.addr_bits = 29; addr_bcast(&a, &b);
	fail_unless(b.addr_ip == htonl(0x01020307), "wrong for /29");
	a.addr_bits = 16; addr_bcast(&a, &b);
	fail_unless(b.addr_ip == htonl(0x0102ffff), "wrong for /16");
	a.addr_bits = 5; addr_bcast(&a, &b);
	fail_unless(b.addr_ip == htonl(0x7ffffff), "wrong for /5");
}
END_TEST

START_TEST(test_addr_net)
{
	struct addr a, b;

	ADDR_PACK(&a, htonl(0x01020304));
	a.addr_bits = 24; addr_net(&a, &b);
	fail_unless(b.addr_ip == htonl(0x01020300), "wrong for /24");
	addr_aton("cafe:babe::dead:beef", &a);
	a.addr_bits = 20; addr_net(&a, &b);
	addr_aton("cafe:b000::", &a);
	a.addr_bits = IP6_ADDR_BITS;
	fail_unless(addr_cmp(&a, &b) == 0, "IPv6 net failed");
}
END_TEST

START_TEST(test_addr_ntop)
{
	struct ntop {
		u_char *n;
		char *p;
	} *ntop, ntop_ip6[] = {
		{ IP6_ADDR_UNSPEC, "::" },
		{ IP6_ADDR_LOOPBACK, "::1" },
		{ "\xfe\x08\x00\x00\x00\x00\x00\x00"
		  "\x00\x00\x00\x00\x00\x00\x00\x01", "fe08::1" },
		{ "\xff\xff\xff\xff\xff\xff\xff\xff"
		  "\xff\xff\xff\xff\xff\xff\xff\xff",
		  "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff" },
		{ "\xca\xfe\xba\xbe\x00\x00\x00\x00\x00\x00\x00\x00"
		  "\xde\xad\xbe\xef", "cafe:babe::dead:beef" },
		{ "\xfe\xed\xfa\xce\x00\x00\x00\x00\x00\x00\x00\x00"
		  "\x00\x00\x00\x00", "feed:face::" },
		{ "\x00\x00\x00\x0a\x00\x0b\x00\x0c\x00"
		  "\x0d\x00\x0e\x00\x0f\x00\x00", "0:a:b:c:d:e:f:0" },
		{ "\x00\x00\x00\x00\x00\x00\x00\x00"
		  "\x00\x00\xff\xff\x01\x02\x03\x04", "::ffff:1.2.3.4" },
		{ NULL }
	};
	struct addr a;
	char buf[64];

	ADDR_PACK(&a, htonl(0x010203ff));
	a.addr_bits = 23; addr_ntop(&a, buf, sizeof(buf));
	fail_unless(strcmp(buf, "1.2.3.255/23") == 0, "bad /23 handling");
	a.addr_bits = 0; addr_ntop(&a, buf, sizeof(buf));
	fail_unless(strcmp(buf, "1.2.3.255/0") == 0, "bad /0 handling");
	a.addr_bits = 32; addr_ntop(&a, buf, sizeof(buf));
	fail_unless(strcmp(buf, "1.2.3.255") == 0, "bad /32 handling");
	fail_unless(addr_ntop(&a, buf, 9) == NULL, "buffer overflow?");

	addr_pack(&a, ADDR_TYPE_ETH, ETH_ADDR_BITS,
	    "\x00\x00\x00\x00\x00\x00", ETH_ADDR_LEN);
	fail_unless(strcmp(addr_ntop(&a, buf, sizeof(buf)),
	    "00:00:00:00:00:00") == 0, "bad empty MAC handling");
	memcpy(&a.addr_eth, "\x00\x0d\x0e\x0a\x0d\x00", ETH_ADDR_LEN);
	fail_unless(strcmp(addr_ntop(&a, buf, sizeof(buf)),
	    "00:0d:0e:0a:0d:00") == 0, "b0rked");
	a.addr_bits = 16;
	fail_unless(addr_ntop(&a, buf, sizeof(buf)) == NULL, "took /16 mask");
	
	for (ntop = ntop_ip6; ntop->n != NULL; ntop++) {
		addr_pack(&a, ADDR_TYPE_IP6, IP6_ADDR_BITS, ntop->n,
		    IP6_ADDR_LEN);
		fail_unless(strcmp(addr_ntop(&a, buf, sizeof(buf)),
		    ntop->p) == 0, ntop->p);
	}
}
END_TEST

START_TEST(test_addr_pton)
{
	struct pton {
		char	*p;
		u_char	*n;
	} *pton, pton_ip6[] = {
		{ "::", IP6_ADDR_UNSPEC },
		{ "::1", IP6_ADDR_LOOPBACK },
		{ "fe08::", "\xfe\x08\x00\x00\x00\x00\x00\x00"
		  "\x00\x00\x00\x00\x00\x00\x00\x00" },
		{ "fe08::1", "\xfe\x08\x00\x00\x00\x00\x00\x00"
		  "\x00\x00\x00\x00\x00\x00\x00\x01" },
		{ "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", "\xff\xff\xff\xff"
		  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" },
		{ "cafe::babe:dead:beef:0:ffff", "\xca\xfe\x00\x00\x00\x00"
		  "\xba\xbe\xde\xad\xbe\xef\x00\x00\xff\xff" },
		{ "::1.2.3.4", "\x00\x00\x00\x00\x00\x00\x00\x00"
		  "\x00\x00\x00\x00\x01\x02\x03\x04" },
		{ ":cafe", NULL }, { ":::", NULL }, { "::fffff", NULL },
		{ NULL }
	}, pton_eth[] = {
		{ "0:d:e:a:d:0", "\x00\x0d\x0e\x0a\x0d\x00" },
		{ "ff:ff:ff:ff:ff:ff", ETH_ADDR_BROADCAST },
		{ "00:d:0e:a:0d:0", "\x00\x0d\x0e\x0a\x0d\x00" },
		{ ":d:e:a:d:0", NULL }, { "0:d:e:a:d:", NULL },
		{ "0:d:e:a:def:0", NULL }, { "0:d:e:a:d:0:0", NULL },
		{ "0:0:0:0:0:0", "\x00\x00\x00\x00\x00\x00" },
		{ NULL }
	};
	struct addr a, b;
	int res;

	ADDR_PACK(&a, htonl(0x010203ff));
	a.addr_bits = 17; addr_pton("1.2.3.255/17", &b);
	fail_unless(addr_cmp(&a, &b) == 0, "bad /17 handling");
	a.addr_bits = 32; addr_pton("1.2.3.255", &b);
	fail_unless(addr_cmp(&a, &b) == 0, "bad handling of missing /32");
	fail_unless(addr_pton("1.2.3.4/33", &b) < 0, "accepted /33");
	fail_unless(addr_pton("1.2.3.256", &b) < 0, "accepted .256");
	fail_unless(addr_pton("1.2.3.4.5", &b) < 0, "accepted quint octet");
	fail_unless(addr_pton("1.2.3", &b) < 0, "accepted triple octet");
	fail_unless(addr_pton("localhost", &b) < 0, "barfed on localhost");
	fail_unless(addr_pton("localhost/24", &b) < 0,
	    "barfed on localhost/24");
	addr_pton("1.2.3.4/24", &a);
	addr_pton("1.2.3.4/255.255.255.0", &b);
	fail_unless(addr_cmp(&a, &b) == 0, "bad /255.255.255.0 handling");

	for (pton = pton_eth; pton->n != NULL; pton++) {
		res = addr_pton(pton->p, &a);
		if (pton->n != NULL) {
			fail_unless(res == 0 &&
			    a.addr_type == ADDR_TYPE_ETH &&
			    a.addr_bits == ETH_ADDR_BITS &&
			    memcmp(&a.addr_eth, pton->n, ETH_ADDR_LEN) == 0,
			    pton->p);
		} else {
			fail_unless(res < 0, pton->p);
		}
	}
	for (pton = pton_ip6; pton->n != NULL; pton++) {
		res = addr_pton(pton->p, &a);
		if (pton->n != NULL) {
			fail_unless(res == 0 &&
			    a.addr_type == ADDR_TYPE_IP6 &&
			    a.addr_bits == IP6_ADDR_BITS &&
			    memcmp(&a.addr_ip6, pton->n, IP6_ADDR_LEN) == 0,
			    pton->p);
		} else {
			fail_unless(res < 0, pton->p);
		}
	}
}
END_TEST

START_TEST(test_addr_ntoa)
{
	struct addr a;
	int i;

	ADDR_PACK(&a, htonl(0x01020304));
	for (i = 0; i < 1000; i++) {
		fail_unless(strcmp(addr_ntoa(&a), "1.2.3.4") == 0,
		    "barfed on 1.2.3.4 loop");
	}
}
END_TEST

START_TEST(test_addr_ntos)
{
	struct sockaddr_in s1, s2;
	struct addr a;

	memset(&s1, 0, sizeof(s1));
	memset(&s2, 0, sizeof(s2));
	SIN_PACK(&s1, htonl(0x01020304), 0);
	ADDR_PACK(&a, htonl(0x01020304));
	addr_ntos(&a, (SA *)&s2);
	fail_unless(memcmp(&s1, &s2, sizeof(s1)) == 0, "bad sockaddr_in");
}
END_TEST

START_TEST(test_addr_ston)
{
	struct sockaddr_in s, t;
	struct addr a, b;

	memset(&a, 0, sizeof(a));
	ADDR_PACK(&a, htonl(0x01020304));
	memcpy(&b, &a, sizeof(&b));
	SIN_PACK(&s, htonl(0x01020304), 0);
	memcpy(&t, &s, sizeof(&t));
	
	addr_ston((SA *)&s, &b);
	fail_unless(memcmp(&a, &b, sizeof(a)) == 0, "bad addr");
#ifdef HAVE_SOCKADDR_SA_LEN
	s.sin_len = 0;
	fail_unless(addr_ston((SA *)&s, &b) == 0 && addr_cmp(&a, &b) == 0,
	    "sin_len == 0");
#endif
	s.sin_family = 123;
	fail_unless(addr_ston((SA *)&s, &b) < 0, "sin_family == 123");
}
END_TEST

START_TEST(test_addr_btos)
{
	struct sockaddr s;
	struct addr a;
	
	ADDR_PACK(&a, htonl(0xffffff00));
	a.addr_bits = 24;
	fail_unless(addr_btos(a.addr_bits, &s) == 0, "b0rked");
}
END_TEST

START_TEST(test_addr_stob)
{
	struct sockaddr_in s;
	struct addr a;

	SIN_PACK(&s, htonl(0xffffff00), 0);
	addr_stob((SA *)&s, &a.addr_bits);
	fail_unless(a.addr_bits == 24, "b0rked");
	/* XXX - BSD routing sockets or SIOCGIFNETMASK */
	s.sin_family = 0;
	fail_unless(addr_stob((SA *)&s, &a.addr_bits) == 0 &&
	    a.addr_bits == 24, "sin_family = 0");
}
END_TEST

START_TEST(test_addr_btom)
{
	struct addr a;
	uint32_t mask;

	ADDR_PACK(&a, htonl(0xffffff00));
	a.addr_bits = 24;
	addr_btom(a.addr_bits, &mask, sizeof(mask));
	fail_unless(mask == htonl(0xffffff00), "b0rked");
}
END_TEST

START_TEST(test_addr_mtob)
{
	struct addr a;
	uint32_t mask;

	mask = htonl(0xffffff00);
	addr_mtob(&mask, sizeof(mask), &a.addr_bits);
	fail_unless(a.addr_bits == 24, "b0rked");
}
END_TEST

Suite *
addr_suite(void)
{
	Suite *s = suite_create("addr");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_addr_pack);
	tcase_add_test(tc_core, test_addr_cmp);
	tcase_add_test(tc_core, test_addr_bcast);
	tcase_add_test(tc_core, test_addr_net);
	tcase_add_test(tc_core, test_addr_ntop);
	tcase_add_test(tc_core, test_addr_pton);
	tcase_add_test(tc_core, test_addr_ntoa);
	tcase_add_test(tc_core, test_addr_ntos);
	tcase_add_test(tc_core, test_addr_ston);
	tcase_add_test(tc_core, test_addr_btos);
	tcase_add_test(tc_core, test_addr_stob);
	tcase_add_test(tc_core, test_addr_btom);
	tcase_add_test(tc_core, test_addr_mtob);
	
	return (s);
}

int
main(void)
{
	Suite *s = addr_suite();
	SRunner *sr = srunner_create(s);
	int nf;
	
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
