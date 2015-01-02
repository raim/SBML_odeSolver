/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/interpol.h>

/* fixtures */
static const char *vars[] = {"MAPK", "MAPK_PP", "MKKK", "MKK_PP"};
static const int n_vars = (int)(sizeof(vars)/sizeof(vars[0]));

/* test cases */
START_TEST(test_read_header_line)
{
	int *cols;
	int *indices;
	int r;
	cols = calloc(n_vars, sizeof(int));
	indices = calloc(n_vars, sizeof(int));
	r = read_header_line(EXAMPLES_FILENAME("MAPK_10pt.dat"), n_vars, (char **)vars, cols, indices);
	ck_assert_int_eq(r, n_vars);
	ck_assert_int_eq(cols[0], 1);
	ck_assert_int_eq(cols[1], 5);
	ck_assert_int_eq(cols[2], 6);
	ck_assert_int_eq(cols[3], 8);
	ck_assert_int_eq(indices[0], 2);
	ck_assert_int_eq(indices[1], 3);
	ck_assert_int_eq(indices[2], 0);
	ck_assert_int_eq(indices[3], 1);
	free(cols);
	free(indices);
}
END_TEST

START_TEST(test_read_columns)
{
	static int cols[] = {1, 5, 6, 8};
	static int indices[] = {2, 3, 0, 1};
	time_series_t *ts;
	int i, r;
	ts = calloc(1, sizeof(*ts));
	ts->data = calloc(4, sizeof(double *));
	for (i=0;i<4;i++) {
		ts->data[i] = calloc(11, sizeof(double));
	}
	ts->time = calloc(11, sizeof(double));
	r = read_columns(EXAMPLES_FILENAME("MAPK_10pt.dat"), 4, cols, indices, ts);
	ck_assert_int_eq(r, 11);
	ck_assert(ts->time[0] == 0);
	ck_assert(ts->data[0][0] == 280);
	ck_assert(ts->data[1][0] == 10);
	ck_assert(ts->data[2][0] == 90);
	ck_assert(ts->data[3][0] == 10);
	ck_assert(ts->time[1] == 10);
	ck_assert(ts->data[0][1] == 279.88);
	ck_assert(ts->data[1][1] == 9.01507);
	ck_assert(ts->data[2][1] == 80.718);
	ck_assert(ts->data[3][1] == 8.68109);
	ck_assert(ts->time[2] == 20);
	ck_assert(ts->data[0][2] == 280.012);
	ck_assert(ts->data[1][2] == 8.13013);
	ck_assert(ts->data[2][2] == 71.2642);
	ck_assert(ts->data[3][2] == 8.71021);
	ck_assert(ts->time[10] == 100);
	ck_assert(ts->data[0][10] == 251.69);
	ck_assert(ts->data[1][10] == 21.2823);
	ck_assert(ts->data[2][10] == 14.2049);
	ck_assert(ts->data[3][10] == 56.6475);
	free_data(ts);
}
END_TEST

START_TEST(test_read_data)
{
	time_series_t *ts;
	int i;
	ts = read_data(EXAMPLES_FILENAME("MAPK_10pt.dat"), n_vars, (char **)vars);
	ck_assert(ts != NULL);
	ck_assert_int_eq(ts->n_var, n_vars);
	for (i=0;i<n_vars;i++) {
		ck_assert_str_eq(ts->var[i], vars[i]);
	}
	ck_assert_int_eq(ts->n_data, n_vars);
	ck_assert_int_eq(ts->n_time, 11);
	ck_assert(ts->warn != NULL);
	ck_assert_int_eq(ts->warn[0], 0);
	ck_assert_int_eq(ts->warn[1], 0);
	free_data(ts);
}
END_TEST

START_TEST(test_bisection)
{
	static const double x[] = {-20.0, -4.5, 0.0, 1.25, 8.5};
	int r;
	r = bisection(5, x, -4.5);
	ck_assert_int_eq(r, 1);
	r = bisection(5, x, 2.0);
	ck_assert_int_eq(r, 3);
	r = bisection(5, x, -30.0);
	ck_assert_int_eq(r, -1);
	r = bisection(5, x, 10.0);
	ck_assert_int_eq(r, 4);
}
END_TEST

/* public */
Suite *create_suite_interpol(void)
{
	Suite *s;
	TCase *tc_read_header_line;
	TCase *tc_read_columns;
	TCase *tc_read_data;
	TCase *tc_bisection;

	s = suite_create("interpol");

	tc_read_header_line = tcase_create("read_header_line");
	tcase_add_test(tc_read_header_line, test_read_header_line);
	suite_add_tcase(s, tc_read_header_line);

	tc_read_columns = tcase_create("read_columns");
	tcase_add_test(tc_read_columns, test_read_columns);
	suite_add_tcase(s, tc_read_columns);

	tc_read_data = tcase_create("read_data");
	tcase_add_test(tc_read_data, test_read_data);
	suite_add_tcase(s, tc_read_data);

	tc_bisection = tcase_create("bisection");
	tcase_add_test(tc_bisection, test_bisection);
	suite_add_tcase(s, tc_bisection);

	return s;
}
