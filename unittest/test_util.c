/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/util.h>

/* helpers */
static int *generate_random_line(int size, int newline)
{
  int *line;
  int i, c;

  line = calloc(size + newline, sizeof(int));
  if (!line) ck_abort_msg("failed to malloc");
  srand((unsigned int)time(NULL));
  for (i=0;i<size;i++) {
    do {
      c = rand() % 128;
    } while (c == '\0' || c == '\n');  /* excluding NUL and LF */
    line[i] = c;
  }
  if (newline) {
    line[size] = '\n';
  }
  return line;
}

static void check_get_line(int size, int newline)
{
  FILE *fp, *rfp;
  int *input;
  char *line;
  int i;

  input = generate_random_line(size, newline);
  OPEN_TMPFILE_OR_ABORT(fp);
  for (i=0;i<size + newline;i++) {
    fputc(input[i], fp);
  }
  fflush(fp);
  rfp = freopen(NULL, "r+b", fp);
  if (!rfp) {
    fclose(fp);
    ck_abort_msg("failed to reopen the file");
  }
  line = get_line(rfp);
  fclose(rfp);
  ck_assert(line != NULL);
  ck_assert_int_eq((int)strlen(line), size);
  for (i=0;i<size;i++) {
    ck_assert_int_eq(line[i], input[i]);
  }
  xfree(line);
  free(input);
}

#define CHECK_GET_LINE__EOF(size) check_get_line(size, 0);
#define CHECK_GET_LINE__NEWLINE(size) check_get_line(size, 1)

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

START_TEST(test_get_line__eof)
{
  CHECK_GET_LINE__EOF(1);
  CHECK_GET_LINE__EOF(2);
  CHECK_GET_LINE__EOF(511);
  CHECK_GET_LINE__EOF(512);
  CHECK_GET_LINE__EOF(513);
  CHECK_GET_LINE__EOF(1022);
  CHECK_GET_LINE__EOF(1023);
  CHECK_GET_LINE__EOF(1024);
  CHECK_GET_LINE__EOF(1025);
}
END_TEST

START_TEST(test_get_line__newline)
{
  CHECK_GET_LINE__NEWLINE(1);
  CHECK_GET_LINE__NEWLINE(2);
  CHECK_GET_LINE__NEWLINE(511);
  CHECK_GET_LINE__NEWLINE(512);
  CHECK_GET_LINE__NEWLINE(513);
  CHECK_GET_LINE__NEWLINE(1022);
  CHECK_GET_LINE__NEWLINE(1023);
  CHECK_GET_LINE__NEWLINE(1024);
  CHECK_GET_LINE__NEWLINE(1025);
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
  tcase_add_test(tc_get_line, test_get_line__eof);
  tcase_add_test(tc_get_line, test_get_line__newline);
  suite_add_tcase(s, tc_get_line);

  tc_concat = tcase_create("concat");
  tcase_add_test(tc_concat, test_concat);
  suite_add_tcase(s, tc_concat);

  return s;
}
