/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/interpol.h>

/* fixtures */
static const char *vars[] = {"MAPK", "MAPK_PP", "MKKK", "MKK_PP"};
static const int n_vars = (int)(sizeof(vars)/sizeof(vars[0]));

static const double xs[] = {0.0, 1.0, 2.0, 3.0, 5.0, 10.0};
static const double ys[] = {-2.5, 1.5, -0.5, 0.25, 1.0, 2.0};
static const int n_xs = (int)(sizeof(xs)/sizeof(xs[0]));

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

START_TEST(test_free_data)
{
  free_data(NULL); /* freeing NULL is safe */
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

START_TEST(test_hunt)
{
  static const double x[] = {-20.0, -4.5, 0.0, 1.25, 8.5};
  static const int n = (int)sizeof(x)/sizeof(x[0]);
  int i, r;
  for (i=0;i<n;i++) {
    r = i;
    hunt(n, x, -30.0, &r);
    ck_assert_int_eq(r, -1);
  }
  for (i=0;i<n;i++) {
    r = i;
    hunt(n, x, 0.1, &r);
    ck_assert_int_eq(r, 2);
  }
  for (i=0;i<n;i++) {
    r = i;
    hunt(n, x, 0.0, &r);
    ck_assert_int_eq(r, 2);
  }
  for (i=0;i<n;i++) {
    r = i;
    hunt(n, x, 10.0, &r);
    ck_assert_int_eq(r, 4);
  }
}
END_TEST

START_TEST(test_spline)
{
  assert(sizeof(xs)/sizeof(xs[0]) == sizeof(ys)/sizeof(ys[0]));
  double *y2;
  int r;
  y2 = calloc(n_xs, sizeof(*y2));
  r = spline(n_xs, xs, ys, y2);
  ck_assert_int_eq(r, 1);
  CHECK_DOUBLE_WITH_TOLERANCE(y2[0], 0.0);
  CHECK_DOUBLE_WITH_TOLERANCE(y2[1], -10.80891608391608471607);
  CHECK_DOUBLE_WITH_TOLERANCE(y2[2], 7.23566433566433531155);
  CHECK_DOUBLE_WITH_TOLERANCE(y2[3], -1.63374125874125875058);
  CHECK_DOUBLE_WITH_TOLERANCE(y2[4], 0.15839160839160837391);
  CHECK_DOUBLE_WITH_TOLERANCE(y2[5], 0.0);
  free(y2);
}
END_TEST

START_TEST(test_splint)
{
  double *y2;
  double y;
  int i;
  int j;
  y2 = calloc(n_xs, sizeof(*y2));
  (void)spline(n_xs, xs, ys, y2);
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, -0.5, &y, &j);
    ck_assert_int_eq(j, -1);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, 0.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, -2.5);
    ck_assert_int_eq(j, 0);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, 1.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, 1.5);
    ck_assert_int_eq(j, 1);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, 2.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, -0.5);
    ck_assert_int_eq(j, 2);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, 4.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, 0.99383741258741253866);
    ck_assert_int_eq(j, 3);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, 8.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, 1.37825174825174845417);
    ck_assert_int_eq(j, 4);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    splint(n_xs, xs, ys, y2, 16.0, &y, &j);
    ck_assert_int_eq(j, 5);
  }
  free(y2);
}
END_TEST

START_TEST(test_linint)
{
  double y;
  int i;
  int j;
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, -0.5, &y, &j);
    ck_assert_int_eq(j, -1);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, 0.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, -2.5);
    ck_assert_int_eq(j, 0);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, 1.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, 1.5);
    ck_assert_int_eq(j, 1);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, 2.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, -0.5);
    ck_assert_int_eq(j, 2);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, 4.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, 0.625);
    ck_assert_int_eq(j, 3);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, 8.0, &y, &j);
    CHECK_DOUBLE_WITH_TOLERANCE(y, 1.60000000000000008882);
    ck_assert_int_eq(j, 4);
  }
  for (i=0;i<n_xs;i++) {
    j = i;
    linint(n_xs, xs, ys, 16.0, &y, &j);
    ck_assert_int_eq(j, 5);
  }
}
END_TEST

/* public */
Suite *create_suite_interpol(void)
{
  Suite *s;
  TCase *tc_read_header_line;
  TCase *tc_read_columns;
  TCase *tc_free_data;
  TCase *tc_read_data;
  TCase *tc_bisection;
  TCase *tc_hunt;
  TCase *tc_spline;
  TCase *tc_splint;
  TCase *tc_linint;

  s = suite_create("interpol");

  tc_read_header_line = tcase_create("read_header_line");
  tcase_add_test(tc_read_header_line, test_read_header_line);
  suite_add_tcase(s, tc_read_header_line);

  tc_read_columns = tcase_create("read_columns");
  tcase_add_test(tc_read_columns, test_read_columns);
  suite_add_tcase(s, tc_read_columns);

  tc_free_data = tcase_create("free_data");
  tcase_add_test(tc_free_data, test_free_data);
  suite_add_tcase(s, tc_free_data);

  tc_read_data = tcase_create("read_data");
  tcase_add_test(tc_read_data, test_read_data);
  suite_add_tcase(s, tc_read_data);

  tc_bisection = tcase_create("bisection");
  tcase_add_test(tc_bisection, test_bisection);
  suite_add_tcase(s, tc_bisection);

  tc_hunt = tcase_create("hunt");
  tcase_add_test(tc_hunt, test_hunt);
  suite_add_tcase(s, tc_hunt);

  tc_spline = tcase_create("spline");
  tcase_add_test(tc_spline, test_spline);
  suite_add_tcase(s, tc_spline);

  tc_splint = tcase_create("splint");
  tcase_add_test(tc_splint, test_splint);
  suite_add_tcase(s, tc_splint);

  tc_linint = tcase_create("linint");
  tcase_add_test(tc_linint, test_linint);
  suite_add_tcase(s, tc_linint);

  return s;
}
