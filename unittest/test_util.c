/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/util.h>

/* test cases */
START_TEST(test_get_line)
{
	FILE *fp, *rfp;
	char *line;

	OPEN_TMPFILE_OR_ABORT(fp);
	fprintf(fp, "abc\n");
	fprintf(fp, "xyz\r\n");
	fprintf(fp, "\n");
	fflush(fp);
	rfp = freopen(NULL, "r+b", fp);
	if (!rfp) {
		fclose(fp);
		ck_abort_msg("failed to reopen the file");
	}
	line = get_line(rfp);
	ck_assert(line != NULL);
	ck_assert_str_eq(line, "abc");
	xfree(line);
	line = get_line(rfp);
	ck_assert(line != NULL);
	ck_assert_str_eq(line, "xyz\r");
	xfree(line);
	line = get_line(rfp);
	ck_assert(line != NULL);
	ck_assert_str_eq(line, "");
	xfree(line);
	line = get_line(rfp);
	ck_assert(line == NULL);
	line = get_line(rfp);
	ck_assert(line == NULL);
	fclose(rfp);
}
END_TEST

START_TEST(test_concat)
{
	char *p;

	p = concat("", "");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "/");
	xfree(p);

	p = concat("", "xyz");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "/xyz");
	xfree(p);

	p = concat("abc", "");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "abc/");
	xfree(p);

	p = concat("abc", "xyz");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "abc/xyz");
	xfree(p);
}
END_TEST

/* public */
Suite *create_suite_util(void)
{
	Suite *s;
	TCase *tc_get_line;
	TCase *tc_concat;

	s = suite_create("util");

	tc_get_line = tcase_create("get_line");
	tcase_add_test(tc_get_line, test_get_line);
	suite_add_tcase(s, tc_get_line);

	tc_concat = tcase_create("concat");
	tcase_add_test(tc_concat, test_concat);
	suite_add_tcase(s, tc_concat);

	return s;
}
