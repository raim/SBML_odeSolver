/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/cvodeData.h>

/* fixtures */
static odeModel_t *model;

static cvodeResults_t *cr;

static void teardown_model(void)
{
	ODEModel_free(model);
}

static void setup_cr(void)
{
  cr = calloc(1, sizeof(*cr));
}

static void teardown_cr(void)
{
  CvodeResults_free(cr);
}

/* test cases */
START_TEST(test_CvodeData_create)
{
	cvodeData_t *cd;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cd = CvodeData_create(model);
	ck_assert(cd != NULL);
	ck_assert_int_eq(cd->nvalues, model->neq + model->nconst + model->nass);
	ck_assert_int_eq(cd->neq, model->neq);
	ck_assert_int_eq(cd->nevents, model->nevents);
	ck_assert(cd->model == model);
	ck_assert_int_eq(cd->allRulesUpdated, 0);
	CvodeData_free(cd);
}
END_TEST

START_TEST(test_CvodeData_initializeValues)
{
	cvodeData_t *cd;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cd = CvodeData_create(model);
	CvodeData_initializeValues(cd);
	ck_assert_int_eq(cd->allRulesUpdated, 1);
	CvodeData_free(cd);
}
END_TEST

START_TEST(test_CvodeData_initialize)
{
	cvodeData_t *cd;
	cvodeSettings_t *cs;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cd = CvodeData_create(model);
	cs = CvodeSettings_create();
	CvodeData_initialize(cd, cs, model, 0);
	ck_assert_int_eq(cd->allRulesUpdated, 1);
	CvodeSettings_free(cs);
	CvodeData_free(cd);
}
END_TEST

START_TEST(test_CvodeResults_getTime)
{
  cr->time = calloc(4, sizeof(double));
  cr->time[0] = 0.1;
  cr->time[1] = 0.2;
  cr->time[2] = 1.35;
  cr->time[3] = 1.5;
  ck_assert_int_eq(CvodeResults_getTime(cr, 0), 0.1);
  ck_assert_int_eq(CvodeResults_getTime(cr, 1), 0.2);
  ck_assert_int_eq(CvodeResults_getTime(cr, 2), 1.35);
  ck_assert_int_eq(CvodeResults_getTime(cr, 3), 1.5);
}
END_TEST

START_TEST(test_CvodeResults_getValue)
{
  variableIndex_t *vi;

  vi = calloc(1, sizeof(*vi));
  vi->type = ODE_VARIABLE;
  vi->type_index = 0;
  vi->index = 1;

  cr->value = calloc(3, sizeof(double *));
  cr->value[0] = calloc(4, sizeof(double));
  cr->value[1] = calloc(4, sizeof(double));
  cr->value[2] = calloc(4, sizeof(double));
  cr->value[0][0] = 1;
  cr->value[0][1] = 2;
  cr->value[0][2] = 3;
  cr->value[0][3] = 4;
  cr->value[1][0] = 5;
  cr->value[1][1] = 6;
  cr->value[1][2] = 7;
  cr->value[1][3] = 8;
  cr->value[2][0] = 9;
  cr->value[2][1] = 10;
  cr->value[2][2] = 11;
  cr->value[2][3] = 12;

  ck_assert(CvodeResults_getValue(cr, vi, 0) == 5);
  ck_assert(CvodeResults_getValue(cr, vi, 1) == 6);
  ck_assert(CvodeResults_getValue(cr, vi, 2) == 7);
  ck_assert(CvodeResults_getValue(cr, vi, 3) == 8);

  free(vi);
}
END_TEST

START_TEST(test_CvodeResults_getNout)
{
  ck_assert_int_eq(CvodeResults_getNout(cr), 1);
  cr->nout = 3;
  ck_assert_int_eq(CvodeResults_getNout(cr), 4);
}
END_TEST

/* public */
Suite *create_suite_cvodeData(void)
{
	Suite *s;
	TCase *tc_CvodeData_create;
	TCase *tc_CvodeData_initializeValues;
	TCase *tc_CvodeData_initialize;
  TCase *tc_CvodeResults_getTime;
  TCase *tc_CvodeResults_getValue;
  TCase *tc_CvodeResults_getNout;

	s = suite_create("cvodeData");

	tc_CvodeData_create = tcase_create("CvodeData_create");
	tcase_add_checked_fixture(tc_CvodeData_create,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_CvodeData_create, test_CvodeData_create);
	suite_add_tcase(s, tc_CvodeData_create);

	tc_CvodeData_initializeValues = tcase_create("CvodeData_initializeValues");
	tcase_add_checked_fixture(tc_CvodeData_initializeValues,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_CvodeData_initializeValues, test_CvodeData_initializeValues);
	suite_add_tcase(s, tc_CvodeData_initializeValues);

	tc_CvodeData_initialize = tcase_create("CvodeData_initialize");
	tcase_add_checked_fixture(tc_CvodeData_initialize,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_CvodeData_initialize, test_CvodeData_initialize);
	suite_add_tcase(s, tc_CvodeData_initialize);

  tc_CvodeResults_getTime = tcase_create("CvodeResults_getTime");
  tcase_add_checked_fixture(tc_CvodeResults_getTime,
                            setup_cr,
                            teardown_cr);
  tcase_add_test(tc_CvodeResults_getTime, test_CvodeResults_getTime);
  suite_add_tcase(s, tc_CvodeResults_getTime);

  tc_CvodeResults_getValue = tcase_create("CvodeResults_getValue");
  tcase_add_checked_fixture(tc_CvodeResults_getValue,
                            setup_cr,
                            teardown_cr);
  tcase_add_test(tc_CvodeResults_getValue, test_CvodeResults_getValue);
  suite_add_tcase(s, tc_CvodeResults_getValue);

  tc_CvodeResults_getNout = tcase_create("CvodeResults_getNout");
  tcase_add_checked_fixture(tc_CvodeResults_getNout,
                            setup_cr,
                            teardown_cr);
  tcase_add_test(tc_CvodeResults_getNout, test_CvodeResults_getNout);
  suite_add_tcase(s, tc_CvodeResults_getNout);

	return s;
}
