
#include <sys/types.h>

#include <dnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

START_TEST(test_arp_pack)
{
}
END_TEST

START_TEST(test_arp_openclose)
{
	arp_t	*a;

	fail_unless((a = arp_open()) != NULL, "open failed");
	fail_unless((a = arp_close(a)) == NULL, "close failed");
}
END_TEST

START_TEST(test_arp_add)
{
	
}
END_TEST

START_TEST(test_arp_delete)
{
}
END_TEST

START_TEST(test_arp_get)
{
}
END_TEST

START_TEST(test_arp_loop)
{
}
END_TEST

Suite *
arp_suite(void)
{
	Suite *s = suite_create("arp");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_arp_pack);
	tcase_add_test(tc_core, test_arp_openclose);
	tcase_add_test(tc_core, test_arp_add);
	tcase_add_test(tc_core, test_arp_delete);
	tcase_add_test(tc_core, test_arp_get);
	tcase_add_test(tc_core, test_arp_loop);
	
	return (s);
}

int
main(void)
{
	Suite *s = arp_suite();
	SRunner *sr = srunner_create(s);
	int nf;
	
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
