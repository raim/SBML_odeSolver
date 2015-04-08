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

START_TEST(test_CvodeData_free)
{
  CvodeData_free(NULL); /* freeing NULL is safe */
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

  cr->nvalues = 3;
  cr->value = calloc(cr->nvalues, sizeof(double *));
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

START_TEST(test_CvodeResults_getSensitivityByNum)
{
  cr->nout = 3;
  CvodeResults_allocateSens(cr, 2, 3, cr->nout);
  cr->sensitivity[1][2][3] = .7;
  ck_assert(CvodeResults_getSensitivityByNum(cr, 1, 2, 3) == .7);
  ck_assert(CvodeResults_getSensitivityByNum(cr, 2, 2, 3) == 0);
  ck_assert(CvodeResults_getSensitivityByNum(cr, 1, 3, 3) == 0);
  ck_assert(CvodeResults_getSensitivityByNum(cr, 1, 2, 4) == 0);
}
END_TEST

START_TEST(test_CvodeResults_getSensitivity)
{
  variableIndex_t *vi_y;
  variableIndex_t *vi_s;

  vi_y = calloc(1, sizeof(*vi_y));
  vi_s = calloc(1, sizeof(*vi_s));

  cr->nout = 3;
  CvodeResults_allocateSens(cr, 2, 3, cr->nout);
  cr->index_sens[0] = 0;
  cr->index_sens[1] = 2;
  cr->index_sens[2] = 17;
  cr->sensitivity[0][1][3] = .7;
  cr->sensitivity[1][2][3] = .8;

  vi_y->index = 0;
  vi_s->index = 16;
  ck_assert(CvodeResults_getSensitivity(cr, vi_y, vi_s, 3) == 0);

  vi_y->index = 1;
  vi_s->index = 17;
  ck_assert(CvodeResults_getSensitivity(cr, vi_y, vi_s, 3) == .8);

  free(vi_y);
  free(vi_s);
}
END_TEST

START_TEST(test_CvodeResults_computeDirectional)
{
  static const double dp[] = {
    1.0,
    0.5,
    2.0
  };

  cr->nout = 3;
  CvodeResults_allocateSens(cr, 2, 3, cr->nout);
  cr->sensitivity[0][0][0] = 1;
  cr->sensitivity[0][0][1] = 2;
  cr->sensitivity[0][0][2] = 3;
  cr->sensitivity[0][0][3] = 4;
  cr->sensitivity[0][1][0] = 5;
  cr->sensitivity[0][1][1] = 6;
  cr->sensitivity[0][1][2] = 7;
  cr->sensitivity[0][1][3] = 8;
  cr->sensitivity[0][2][0] = 9;
  cr->sensitivity[0][2][1] = 10;
  cr->sensitivity[0][2][2] = 11;
  cr->sensitivity[0][2][3] = 12;
  cr->sensitivity[1][0][0] = 13;
  cr->sensitivity[1][0][1] = 14;
  cr->sensitivity[1][0][2] = 15;
  cr->sensitivity[1][0][3] = 16;
  cr->sensitivity[1][1][0] = 17;
  cr->sensitivity[1][1][1] = 18;
  cr->sensitivity[1][1][2] = 19;
  cr->sensitivity[1][1][3] = 20;
  cr->sensitivity[1][2][0] = 21;
  cr->sensitivity[1][2][1] = 22;
  cr->sensitivity[1][2][2] = 23;
  cr->sensitivity[1][2][3] = 24;

  CvodeResults_computeDirectional(cr, dp);
  ck_assert(cr->directional[0][0] == 21.5);
  ck_assert(cr->directional[0][1] == 25);
  ck_assert(cr->directional[0][2] == 28.5);
  ck_assert(cr->directional[0][3] == 32);
  ck_assert(cr->directional[1][0] == 63.5);
  ck_assert(cr->directional[1][1] == 67);
  ck_assert(cr->directional[1][2] == 70.5);
  ck_assert(cr->directional[1][3] == 74);
}
END_TEST

START_TEST(test_CvodeResults_free)
{
  CvodeResults_free(NULL); /* freeing NULL is safe */
}
END_TEST

/* public */
Suite *create_suite_cvodeData(void)
{
  Suite *s;
  TCase *tc_CvodeData_create;
  TCase *tc_CvodeData_free;
  TCase *tc_CvodeData_initializeValues;
  TCase *tc_CvodeData_initialize;
  TCase *tc_CvodeResults_getTime;
  TCase *tc_CvodeResults_getValue;
  TCase *tc_CvodeResults_getNout;
  TCase *tc_CvodeResults_getSensitivityByNum;
  TCase *tc_CvodeResults_getSensitivity;
  TCase *tc_CvodeResults_computeDirectional;
  TCase *tc_CvodeResults_free;

  s = suite_create("cvodeData");

  tc_CvodeData_create = tcase_create("CvodeData_create");
  tcase_add_checked_fixture(tc_CvodeData_create,
                NULL,
                teardown_model);
  tcase_add_test(tc_CvodeData_create, test_CvodeData_create);
  suite_add_tcase(s, tc_CvodeData_create);

  tc_CvodeData_free = tcase_create("CvodeData_free");
  tcase_add_test(tc_CvodeData_free, test_CvodeData_free);
  suite_add_tcase(s, tc_CvodeData_free);

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

  tc_CvodeResults_getSensitivityByNum = tcase_create("CvodeResults_getSensitivityByNum");
  tcase_add_checked_fixture(tc_CvodeResults_getSensitivityByNum,
                            setup_cr,
                            teardown_cr);
  tcase_add_test(tc_CvodeResults_getSensitivityByNum, test_CvodeResults_getSensitivityByNum);
  suite_add_tcase(s, tc_CvodeResults_getSensitivityByNum);

  tc_CvodeResults_getSensitivity = tcase_create("CvodeResults_getSensitivity");
  tcase_add_checked_fixture(tc_CvodeResults_getSensitivity,
                            setup_cr,
                            teardown_cr);
  tcase_add_test(tc_CvodeResults_getSensitivity, test_CvodeResults_getSensitivity);
  suite_add_tcase(s, tc_CvodeResults_getSensitivity);

  tc_CvodeResults_computeDirectional = tcase_create("CvodeResults_computeDirectional");
  tcase_add_checked_fixture(tc_CvodeResults_computeDirectional,
                            setup_cr,
                            teardown_cr);
  tcase_add_test(tc_CvodeResults_computeDirectional, test_CvodeResults_computeDirectional);
  suite_add_tcase(s, tc_CvodeResults_computeDirectional);

  tc_CvodeResults_free = tcase_create("CvodeResults_free");
  tcase_add_test(tc_CvodeResults_free, test_CvodeResults_free);
  suite_add_tcase(s, tc_CvodeResults_free);

  return s;
}
