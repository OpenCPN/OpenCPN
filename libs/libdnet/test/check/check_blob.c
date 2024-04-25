
#include <sys/types.h>

#include <dnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

START_TEST(test_blob_newfree)
{
	blob_t *b;
	
	fail_unless((b = blob_new()) != NULL, "new failed");
	fail_unless((b = blob_free(b)) == NULL, "free failed");
}
END_TEST

START_TEST(test_blob_readwrite)
{
	blob_t *b;
	char tmp[32];
	
	b = blob_new();
	blob_write(b, "foobar", 7);
	blob_write(b, "spazzo", 7);
	blob_write(b, "doofus", 7);
	blob_rewind(b);

	blob_read(b, tmp, 7);
	fail_unless(strcmp(tmp, "foobar") == 0, "read1 failed");
	blob_read(b, tmp, 7);
	fail_unless(strcmp(tmp, "spazzo") == 0, "read2 failed");
	blob_read(b, tmp, 7);
	fail_unless(strcmp(tmp, "doofus") == 0, "read3 failed");
	blob_free(b);
}
END_TEST

START_TEST(test_blob_insertdelete)
{
	blob_t *b, bf;
	char tmp[32];
	
	b = blob_new();
	fail_unless(blob_insert(b, "foo", 3) == 3, "insert1 failed");
	blob_rewind(b);
	fail_unless(blob_insert(b, "bar", 3) == 3, "insert2 failed");
	blob_rewind(b);
	blob_read(b, tmp, 6);
	tmp[6] = 0;
	fail_unless(strcmp(tmp,"barfoo") == 0, "read failed");
	blob_rewind(b);
	fail_unless(blob_delete(b, NULL, 3) == 3, "delete failed");
	blob_rewind(b);
	blob_read(b, tmp, 3);
	tmp[3] = 0;
	fail_unless(strcmp(tmp,"foo") == 0, "read failed");
	
	fail_unless(blob_delete(b, NULL, 4) < 0, "deleted more than size");
	b = blob_free(b);

	bf.base = "foobar";
	bf.end = 6;
	bf.off = bf.size = 0;

	fail_unless(blob_insert(&bf, "foobar", 6) < 0, "inserted into fixed");
	fail_unless(blob_delete(&bf, NULL, 3) < 0, "deleted from fixed"); 
}
END_TEST

START_TEST(test_blob_packunpack)
{
	blob_t *b;
	uint32_t D, d;
	uint16_t H, h;
	u_char c;
	char s[128];
	u_char buf[6];

	b = blob_new();

	D = 0xdeadbeef;
	H = 0xbabe;
	memcpy(buf, "f\x00\x00bar", 6);
	c = 'c';
	d = 555;
	h = 666;
	strcpy(s, "donkey");
#if 0	
	printf("D: 0x%x H: 0x%x c: %c d: %d h: %d s: %s\n",
	    D, H, c, d, h, s);
#endif
	fail_unless(blob_pack(b, "whee:%D%H%*b%c%d%h%s\r\n",
	    D, H, sizeof(buf), buf, c, d, h, s) == 0,
	    "pack failed");
	
	blob_rewind(b);
#if 0
	blob_print(b, "hexl", blob_left(b));
#endif
	fail_unless(blob_unpack(b, "whee:%D%H%*b%c%d%h%*s\r\n",
	    &D, &H, sizeof(buf), buf, &c, &d, &h, sizeof(s), s) == 0,
	    "unpack failed");
#if 0
	printf("D: 0x%x H: 0x%x c: %c d: %d h: %d s: %s\n",
	    D, H, c, d, h, s);
#endif
	if (D != 0xdeadbeef || H != 0xbabe ||
	    memcmp(buf, "f\x00\x00bar", 6) != 0 || c != 'c' || d != 555 ||
	    h != 666 || strcmp(s, "donkey") != 0)
		fail("unpacked weird crap");
	
	blob_free(b);
}
END_TEST

START_TEST(test_blob_seek)
{
	blob_t *b;

	b = blob_new();
	blob_insert(b, "foobar", 6);
	blob_rewind(b);
	fail_unless(blob_skip(b, 3) == 3, "skip failed");
	fail_unless(blob_skip(b, 3) == 6, "skip to end failed");
	fail_unless(blob_skip(b, 1) < 0, "skipped past end");
	fail_unless(blob_seek(b, -1, SEEK_END) == 5, "end seek failed");
	fail_unless(blob_seek(b, 1, SEEK_SET) == 1, "set seek failed");
	blob_rewind(b);
	fail_unless(blob_seek(b, -1, SEEK_CUR) < 0, "seeked past 0");
	fail_unless(blob_seek(b, 3, SEEK_CUR) == 3, "cur seek failed");
	blob_free(b);
}
END_TEST

START_TEST(test_blob_index)
{
	blob_t *b;

	b = blob_new();
	blob_write(b, "this is only a test!", 20);
	blob_rewind(b);
	fail_unless(blob_index(b, "this", 4) == 0, "index start failed");
	fail_unless(blob_index(b, "!", 1) == 19, "index end failed");
	fail_unless(blob_index(b, "only ", 5) == 8, "index middle failed");
	fail_unless(blob_rindex(b, "!", 1) == 19, "rindex end failed");
	fail_unless(blob_rindex(b, "this", 4) == 0, "rindex start failed");
	fail_unless(blob_rindex(b, "only ", 5) == 8, "rindex middle failed");
	blob_free(b);
}
END_TEST

Suite *
blob_suite(void)
{
	Suite *s = suite_create("blob");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_blob_newfree);
	tcase_add_test(tc_core, test_blob_readwrite);
	tcase_add_test(tc_core, test_blob_insertdelete);
	tcase_add_test(tc_core, test_blob_packunpack);
	tcase_add_test(tc_core, test_blob_seek);
	tcase_add_test(tc_core, test_blob_index);
	
	return (s);
}

int
main(void)
{
	Suite *s = blob_suite();
	SRunner *sr = srunner_create(s);
	int nf;
#if 0
	srunner_set_fork_status(sr, CK_NOFORK);
#endif
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
