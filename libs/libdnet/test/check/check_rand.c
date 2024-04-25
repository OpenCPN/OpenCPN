
#include <sys/types.h>

#include <dnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

START_TEST(test_rand_openclose)
{
	rand_t *r;

	fail_unless((r = rand_open()) != NULL, "open failed");
	fail_unless((r = rand_close(r)) == NULL, "close failed");
}
END_TEST

START_TEST(test_rand_get)
{
	rand_t *r;
	uint32_t i;

	r = rand_open();
	fail_unless(rand_get(r, &i, sizeof(i)) == 0, "get failed");
	/* XXX - add distribution tests */
	r = rand_close(r);
}
END_TEST

START_TEST(test_rand_set)
{
	rand_t *r;
	u_char buf1[BUFSIZ], buf2[BUFSIZ];
	uint32_t i;

	i = 666;
	r = rand_open();
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	fail_unless(rand_get(r, buf1, sizeof(buf1)) == 0, "get failed");
	fail_unless(memcmp(buf1, buf2, sizeof(buf1)) != 0, "get b0rked");
	fail_unless(rand_set(r, &i, sizeof(i)) == 0, "set failed");
	fail_unless(rand_get(r, buf2, sizeof(buf2)) == 0, "get2 failed");
	fail_unless(memcmp(buf1, buf2, sizeof(buf1)) != 0, "set/get b0rked");
	fail_unless(rand_set(r, &i, sizeof(i)) == 0, "set2 failed");
	fail_unless(rand_get(r, buf1, sizeof(buf1)) == 0, "get3 failed");
	fail_unless(memcmp(buf1, buf2, sizeof(buf1)) == 0, "set b0rked");
	r = rand_close(r);
}
END_TEST

START_TEST(test_rand_shuffle)
{
	rand_t *r;
	char buf[BUFSIZ];

#define SHUFFLE_TEXT	"hello how are you doing today?"
	r = rand_open();
	strcpy(buf, SHUFFLE_TEXT);
	rand_shuffle(r, buf, strlen(buf), 1);
	fail_unless(strcmp(buf, SHUFFLE_TEXT) != 0, "shuffle failed");
	r = rand_close(r);
}
END_TEST
	
Suite *
rand_suite(void)
{
	Suite *s = suite_create("rand");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_rand_openclose);
	tcase_add_test(tc_core, test_rand_get);
	tcase_add_test(tc_core, test_rand_set);
	tcase_add_test(tc_core, test_rand_shuffle);
	
	return (s);
}

int
main(void)
{
	Suite *s = rand_suite();
	SRunner *sr = srunner_create(s);
	int nf;
	
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
