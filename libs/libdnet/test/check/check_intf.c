
#include <sys/types.h>

#include <dnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

START_TEST(test_intf_openclose)
{
	intf_t *i;

	fail_unless((i = intf_open()) != NULL, "open failed");
	fail_unless((i = intf_close(i)) == NULL, "closed failed");
}
END_TEST

START_TEST(test_intf_get)
{
	struct intf_entry ifent;
	intf_t *i;

	i = intf_open();
	memset(&ifent, 0, sizeof(ifent));
	fail_unless(intf_get(i, &ifent) < 0, "didn't fail on empty request");
	ifent.intf_len = sizeof(ifent);
	fail_unless(intf_get(i, &ifent) < 0, "didn't fail on empty name");
	intf_close(i);
}
END_TEST

START_TEST(test_intf_get_src)
{
}
END_TEST

START_TEST(test_intf_get_dst)
{
}
END_TEST

START_TEST(test_intf_set)
{
}
END_TEST

START_TEST(test_intf_loop)
{
}
END_TEST

Suite *
intf_suite(void)
{
	Suite *s = suite_create("intf");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_intf_openclose);
	tcase_add_test(tc_core, test_intf_get);
	tcase_add_test(tc_core, test_intf_get_src);
	tcase_add_test(tc_core, test_intf_get_dst);
	tcase_add_test(tc_core, test_intf_set);
	tcase_add_test(tc_core, test_intf_loop);
	
	return (s);
}

int
main(void)
{
	Suite *s = intf_suite();
	SRunner *sr = srunner_create(s);
	int nf;
	
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
