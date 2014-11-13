/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/util.h>

/* test cases */
START_TEST(test_get_line)
{
	FILE *fp, *rfp;
	char *line;

	fp = tmpfile();
	if (!fp) {
		ck_abort_msg("could not open a temporary file");
	}
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
	line = get_line(rfp);
	ck_assert(line != NULL);
	ck_assert_str_eq(line, "xyz\r");
	line = get_line(rfp);
	ck_assert(line != NULL);
	ck_assert_str_eq(line, "");
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
	free(p);

	p = concat("", "xyz");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "/xyz");
	free(p);

	p = concat("abc", "");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "abc/");
	free(p);

	p = concat("abc", "xyz");
	ck_assert(p != NULL);
	ck_assert_str_eq(p, "abc/xyz");
	free(p);
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
