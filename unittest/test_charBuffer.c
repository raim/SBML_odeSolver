/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/charBuffer.h>

/* test cases */
START_TEST(test_CharBuffer_create)
{
  charBuffer_t *cb;
  const char *b;

  cb = CharBuffer_create();
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "");
  CharBuffer_free(cb);
}
END_TEST

START_TEST(test_CharBuffer_free)
{
  CharBuffer_free(NULL); /* deleting NULL is safe */
}
END_TEST

START_TEST(test_CharBuffer_append)
{
  charBuffer_t *cb;
  const char *b;

  cb = CharBuffer_create();
  CharBuffer_append(cb, "abc");
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "abc");
  CharBuffer_append(cb, "def");
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "abcdef");
  CharBuffer_free(cb);
}
END_TEST

START_TEST(test_CharBuffer_appendInt)
{
  charBuffer_t *cb;
  const char *b;

  cb = CharBuffer_create();
  CharBuffer_appendInt(cb, 0);
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "0");
  CharBuffer_appendInt(cb, -1);
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "0-1");
  CharBuffer_appendInt(cb, 9999);
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "0-19999");
  CharBuffer_free(cb);
}
END_TEST

START_TEST(test_CharBuffer_appendDouble)
{
  charBuffer_t *cb;
  const char *b;

  cb = CharBuffer_create();
  CharBuffer_appendDouble(cb, 0.0);
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "0");
  CharBuffer_appendDouble(cb, -1.0);
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "0-1");
  CharBuffer_appendDouble(cb, 2.5e-1);
  b = CharBuffer_getBuffer(cb);
  ck_assert(b != NULL);
  ck_assert_str_eq(b, "0-10.25");
  CharBuffer_free(cb);
}
END_TEST

/* public */
Suite *create_suite_charBuffer(void)
{
  Suite *s;
  TCase *tc_CharBuffer_create;
  TCase *tc_CharBuffer_free;
  TCase *tc_CharBuffer_append;
  TCase *tc_CharBuffer_appendInt;
  TCase *tc_CharBuffer_appendDouble;

  s = suite_create("charBuffer");

  tc_CharBuffer_create = tcase_create("CharBuffer_create");
  tcase_add_test(tc_CharBuffer_create, test_CharBuffer_create);
  suite_add_tcase(s, tc_CharBuffer_create);

  tc_CharBuffer_free = tcase_create("CharBuffer_free");
  tcase_add_test(tc_CharBuffer_free, test_CharBuffer_free);
  suite_add_tcase(s, tc_CharBuffer_free);

  tc_CharBuffer_append = tcase_create("CharBuffer_append");
  tcase_add_test(tc_CharBuffer_append, test_CharBuffer_append);
  suite_add_tcase(s, tc_CharBuffer_append);

  tc_CharBuffer_appendInt = tcase_create("CharBuffer_appendInt");
  tcase_add_test(tc_CharBuffer_appendInt, test_CharBuffer_appendInt);
  suite_add_tcase(s, tc_CharBuffer_appendInt);

  tc_CharBuffer_appendDouble = tcase_create("CharBuffer_appendDouble");
  tcase_add_test(tc_CharBuffer_appendDouble, test_CharBuffer_appendDouble);
  suite_add_tcase(s, tc_CharBuffer_appendDouble);

  return s;
}
