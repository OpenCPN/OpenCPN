
#include <sys/types.h>

#include <dnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

START_TEST(test_ip_fill)
{
}
END_TEST

START_TEST(test_ip_openclose)
{
	ip_t *i;

	fail_unless((i = ip_open()) != NULL, "open failed");
	fail_unless((i = ip_close(i)) == NULL, "close failed");
}
END_TEST

START_TEST(test_ip_send)
{
}
END_TEST

START_TEST(test_ip_ntoa)
{
}
END_TEST

START_TEST(test_ip_aton)
{
}
END_TEST

START_TEST(test_ip_add_option)
{
}
END_TEST

START_TEST(test_ip_checksum)
{
}
END_TEST

Suite *
ip_suite(void)
{
	Suite *s = suite_create("ip");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_ip_fill);
	tcase_add_test(tc_core, test_ip_openclose);
	tcase_add_test(tc_core, test_ip_send);
	tcase_add_test(tc_core, test_ip_ntoa);
	tcase_add_test(tc_core, test_ip_aton);
	tcase_add_test(tc_core, test_ip_add_option);
	tcase_add_test(tc_core, test_ip_checksum);
	
	return (s);
}

int
main(void)
{
	Suite *s = ip_suite();
	SRunner *sr = srunner_create(s);
	int nf;
	
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
