/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/integratorSettings.h>

/* test cases */
START_TEST(test_TimeSettings_create)
{
	timeSettings_t *ts;
	ts = TimeSettings_create(10.0, 110.0, 200);
	ck_assert(ts != NULL);
	CHECK_DOUBLE_WITH_TOLERANCE(ts->tmult, -0.5); /* TODO: expected? */
	TimeSettings_free(ts);
}
END_TEST

START_TEST(test_TimeSettings_free)
{
	TimeSettings_free(NULL); /* freeing NULL is safe */
}
END_TEST

START_TEST(test_CvodeSettings_createFromTimeSettings)
{
	timeSettings_t *ts;
	cvodeSettings_t *cs;
	ts = TimeSettings_create(0.0, 10.0, 100);
	cs = CvodeSettings_createFromTimeSettings(ts);
	ck_assert(cs != NULL);
	ck_assert(cs->Time == 10.0);
	ck_assert_int_eq(cs->PrintStep, 100);
	CvodeSettings_free(cs);
	TimeSettings_free(ts);
}
END_TEST

START_TEST(test_CvodeSettings_create)
{
	cvodeSettings_t *cs;
	cs = CvodeSettings_create();
	ck_assert(cs != NULL);
	ck_assert(cs->Time == 1.0);
	ck_assert_int_eq(cs->PrintStep, 10);
	CvodeSettings_free(cs);
}
END_TEST

START_TEST(test_CvodeSettings_createWithTime)
{
	cvodeSettings_t *cs;
	cs = CvodeSettings_createWithTime(0.7, 29);
	ck_assert(cs != NULL);
	ck_assert(cs->Time == 0.7);
	ck_assert_int_eq(cs->PrintStep, 29);
	CvodeSettings_free(cs);
}
END_TEST

START_TEST(test_CvodeSettings_free)
{
	CvodeSettings_free(NULL); /* freeing NULL is safe */
}
END_TEST

START_TEST(test_CvodeSettings_getMethod)
{
  cvodeSettings_t *cs;
  cs = CvodeSettings_create();
  ck_assert_str_eq(CvodeSettings_getMethod(cs), "BDF");
  CvodeSettings_free(cs);
}
END_TEST

START_TEST(test_CvodeSettings_getIterMethod)
{
  cvodeSettings_t *cs;
  cs = CvodeSettings_create();
  ck_assert_str_eq(CvodeSettings_getIterMethod(cs), "NEWTON");
  CvodeSettings_free(cs);
}
END_TEST

START_TEST(test_CvodeSettings_getSensMethod)
{
  cvodeSettings_t *cs;
  cs = CvodeSettings_create();
  ck_assert_str_eq(CvodeSettings_getSensMethod(cs), "simultaneous");
  CvodeSettings_free(cs);
}
END_TEST

/* public */
Suite *create_suite_integratorSettings(void)
{
	Suite *s;
	TCase *tc_TimeSettings_create;
	TCase *tc_TimeSettings_free;
	TCase *tc_CvodeSettings_createFromTimeSettings;
	TCase *tc_CvodeSettings_create;
	TCase *tc_CvodeSettings_createWithTime;
	TCase *tc_CvodeSettings_free;
  TCase *tc_CvodeSettings_getMethod;
  TCase *tc_CvodeSettings_getIterMethod;
  TCase *tc_CvodeSettings_getSensMethod;

	s = suite_create("integratorSettings");

	tc_TimeSettings_create = tcase_create("TimeSettings_create");
	tcase_add_test(tc_TimeSettings_create, test_TimeSettings_create);
	suite_add_tcase(s, tc_TimeSettings_create);

	tc_TimeSettings_free = tcase_create("TimeSettings_free");
	tcase_add_test(tc_TimeSettings_free, test_TimeSettings_free);
	suite_add_tcase(s, tc_TimeSettings_free);

	tc_CvodeSettings_createFromTimeSettings = tcase_create("CvodeSettings_createFromTimeSettings");
	tcase_add_test(tc_CvodeSettings_createFromTimeSettings, test_CvodeSettings_createFromTimeSettings);
	suite_add_tcase(s, tc_CvodeSettings_createFromTimeSettings);

	tc_CvodeSettings_create = tcase_create("CvodeSettings_create");
	tcase_add_test(tc_CvodeSettings_create, test_CvodeSettings_create);
	suite_add_tcase(s, tc_CvodeSettings_create);

	tc_CvodeSettings_createWithTime = tcase_create("CvodeSettings_createWithTime");
	tcase_add_test(tc_CvodeSettings_createWithTime, test_CvodeSettings_createWithTime);
	suite_add_tcase(s, tc_CvodeSettings_createWithTime);

	tc_CvodeSettings_free = tcase_create("CvodeSettings_free");
	tcase_add_test(tc_CvodeSettings_free, test_CvodeSettings_free);
	suite_add_tcase(s, tc_CvodeSettings_free);

  tc_CvodeSettings_getMethod = tcase_create("CvodeSettings_getMethod");
  tcase_add_test(tc_CvodeSettings_getMethod, test_CvodeSettings_getMethod);
  suite_add_tcase(s, tc_CvodeSettings_getMethod);

  tc_CvodeSettings_getIterMethod = tcase_create("CvodeSettings_getIterMethod");
  tcase_add_test(tc_CvodeSettings_getIterMethod, test_CvodeSettings_getIterMethod);
  suite_add_tcase(s, tc_CvodeSettings_getIterMethod);

  tc_CvodeSettings_getSensMethod = tcase_create("CvodeSettings_getSensMethod");
  tcase_add_test(tc_CvodeSettings_getSensMethod, test_CvodeSettings_getSensMethod);
  suite_add_tcase(s, tc_CvodeSettings_getSensMethod);

	return s;
}
