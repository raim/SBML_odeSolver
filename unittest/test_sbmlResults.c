/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/sbml.h>
#include <sbmlsolver/sbmlResults.h>

/* fixtures */
static SBMLDocument_t *doc;
static Model_t *model;

static void setup_model(void)
{
  doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
  model = SBMLDocument_getModel(doc);
  ck_assert(Model_getNumCompartments(model) == 1);
  ck_assert(Model_getNumSpecies(model) == 2);
  ck_assert(Model_getNumParameters(model) == 1);
  ck_assert(Model_getNumReactions(model) == 2);
}

static void teardown_model(void)
{
  SBMLDocument_free(doc);
}

/* test cases */
START_TEST(test_SBMLResults_getTime)
{
  SBMLResults_t *r;
  timeCourse_t *tc;

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  tc = SBMLResults_getTime(r);
  ck_assert(tc != NULL);
  ck_assert(r->time == tc);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_SBMLResults_getTimeCourse_no_such_name)
{
  SBMLResults_t *r;
  timeCourse_t *tc;

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  tc = SBMLResults_getTimeCourse(r, "no such name");
  ck_assert(tc == NULL);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_SBMLResults_getNout)
{
  SBMLResults_t *r;
  int n;

  r = SBMLResults_create(model, 42);
  ck_assert(r != NULL);
  n = SBMLResults_getNout(r);
  ck_assert(n == 42);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_SBMLResults_getNumSens)
{
  SBMLResults_t *r;
  int n;

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  n = SBMLResults_getNumSens(r);
  ck_assert(n == 0);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_SBMLResults_getSensParam)
{
  /* TODO */
}
END_TEST

START_TEST(test_Compartment_getTimeCourse)
{
  SBMLResults_t *r;
  Compartment_t *c;
  timeCourse_t *tc;

  c = Model_getCompartment(model, 0);
  ck_assert(c != NULL);
  Compartment_setConstant(c, 0); /* make it variable */

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  tc = Compartment_getTimeCourse(c, r);
  ck_assert(tc != NULL);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_Species_getTimeCourse)
{
  SBMLResults_t *r;
  Species_t *s;
  timeCourse_t *tc;

  s = Model_getSpecies(model, 0);
  ck_assert(s != NULL);

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  tc = Species_getTimeCourse(s, r);
  ck_assert(tc != NULL);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_Parameter_getTimeCourse)
{
  SBMLResults_t *r;
  Parameter_t *p;
  timeCourse_t *tc;

  p = Model_getParameter(model, 0);
  ck_assert(p != NULL);
  Parameter_setConstant(p, 0); /* make it variable */

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  tc = Parameter_getTimeCourse(p, r);
  ck_assert(tc != NULL);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_TimeCourse_getName)
{
  SBMLResults_t *r;
  timeCourse_t *tc;
  const char *name;

  r = SBMLResults_create(model, 1);
  ck_assert(r != NULL);
  tc = SBMLResults_getTime(r);
  ck_assert(tc != NULL);
  name = TimeCourse_getName(tc);
  ck_assert(name != NULL);
  ck_assert_str_eq(name, "time");
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_TimeCourse_getNumValues)
{
  SBMLResults_t *r;
  timeCourse_t *tc;
  int n;

  r = SBMLResults_create(model, 42);
  ck_assert(r != NULL);
  tc = SBMLResults_getTime(r);
  ck_assert(tc != NULL);
  n = TimeCourse_getNumValues(tc);
  ck_assert(n == 42);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_TimeCourse_getValue)
{
  SBMLResults_t *r;
  timeCourse_t *tc;
  double v;

  r = SBMLResults_create(model, 42);
  ck_assert(r != NULL);
  tc = SBMLResults_getTime(r);
  ck_assert(tc != NULL);
  tc->values[0] = 7.77;
  tc->values[41] = -8.5;
  v = TimeCourse_getValue(tc, 0);
  ck_assert(v == 7.77);
  v = TimeCourse_getValue(tc, 41);
  ck_assert(v == -8.5);
  SBMLResults_free(r);
}
END_TEST

START_TEST(test_TimeCourse_getSensitivity)
{
  /* TODO */
}
END_TEST

START_TEST(test_SBMLResults_dump)
{
  /* TODO */
}
END_TEST

START_TEST(test_SBMLResults_dumpSpecies)
{
  /* TODO */
}
END_TEST

START_TEST(test_SBMLResults_dumpCompartments)
{
  /* TODO */
}
END_TEST

START_TEST(test_SBMLResults_dumpParameters)
{
  /* TODO */
}
END_TEST

START_TEST(test_SBMLResults_dumpFluxes)
{
  /* TODO */
}
END_TEST

START_TEST(test_SBMLResults_free)
{
  SBMLResults_free(NULL); /* freeing NULL is safe */
}
END_TEST

START_TEST(test_SBMLResultsArray_free)
{
  SBMLResultsArray_free(NULL); /* freeing NULL is safe */
}
END_TEST

START_TEST(test_SBMLResultsArray_getNumResults)
{
  SBMLResultsArray_t *ra;
  int n;

  ra = SBMLResultsArray_allocate(42);
  ck_assert(ra != NULL);
  n = SBMLResultsArray_getNumResults(ra);
  ck_assert(n == 42);
  SBMLResultsArray_free(ra);
}
END_TEST

START_TEST(test_SBMLResultsArray_getResults)
{
  SBMLResultsArray_t *ra;
  SBMLResults_t *r;

  ra = SBMLResultsArray_allocate(3);
  ck_assert(ra != NULL);
  r = SBMLResultsArray_getResults(ra, 2);
  ck_assert(r == ra->results[2]);
  SBMLResultsArray_free(ra);
}
END_TEST

/* public */
Suite *create_suite_sbmlResults(void)
{
  Suite *s;
  TCase *tc_SBMLResults_getTime;
  TCase *tc_SBMLResults_getTimeCourse;
  TCase *tc_SBMLResults_getNout;
  TCase *tc_SBMLResults_getNumSens;
  TCase *tc_SBMLResults_getSensParam;
  TCase *tc_Compartment_getTimeCourse;
  TCase *tc_Species_getTimeCourse;
  TCase *tc_Parameter_getTimeCourse;
  TCase *tc_TimeCourse_getName;
  TCase *tc_TimeCourse_getNumValues;
  TCase *tc_TimeCourse_getValue;
  TCase *tc_TimeCourse_getSensitivity;
  TCase *tc_SBMLResults_dump;
  TCase *tc_SBMLResults_dumpSpecies;
  TCase *tc_SBMLResults_dumpCompartments;
  TCase *tc_SBMLResults_dumpParameters;
  TCase *tc_SBMLResults_dumpFluxes;
  TCase *tc_SBMLResults_free;
  TCase *tc_SBMLResultsArray_free;
  TCase *tc_SBMLResultsArray_getNumResults;
  TCase *tc_SBMLResultsArray_getResults;

  s = suite_create("sbmlResults");

  tc_SBMLResults_getTime = tcase_create("SBMLResults_getTime");
  tcase_add_checked_fixture(tc_SBMLResults_getTime,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_getTime, test_SBMLResults_getTime);
  suite_add_tcase(s, tc_SBMLResults_getTime);

  tc_SBMLResults_getTimeCourse = tcase_create("SBMLResults_getTimeCourse");
  tcase_add_checked_fixture(tc_SBMLResults_getTimeCourse,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_getTimeCourse, test_SBMLResults_getTimeCourse_no_such_name);
  suite_add_tcase(s, tc_SBMLResults_getTimeCourse);

  tc_SBMLResults_getNout = tcase_create("SBMLResults_getNout");
  tcase_add_checked_fixture(tc_SBMLResults_getNout,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_getNout, test_SBMLResults_getNout);
  suite_add_tcase(s, tc_SBMLResults_getNout);

  tc_SBMLResults_getNumSens = tcase_create("SBMLResults_getNumSens");
  tcase_add_checked_fixture(tc_SBMLResults_getNumSens,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_getNumSens, test_SBMLResults_getNumSens);
  suite_add_tcase(s, tc_SBMLResults_getNumSens);

  tc_SBMLResults_getSensParam = tcase_create("SBMLResults_getSensParam");
  tcase_add_checked_fixture(tc_SBMLResults_getSensParam,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_getSensParam, test_SBMLResults_getSensParam);
  suite_add_tcase(s, tc_SBMLResults_getSensParam);

  tc_Compartment_getTimeCourse = tcase_create("Compartment_getTimeCourse");
  tcase_add_checked_fixture(tc_Compartment_getTimeCourse,
                setup_model,
                teardown_model);
  tcase_add_test(tc_Compartment_getTimeCourse, test_Compartment_getTimeCourse);
  suite_add_tcase(s, tc_Compartment_getTimeCourse);

  tc_Species_getTimeCourse = tcase_create("Species_getTimeCourse");
  tcase_add_checked_fixture(tc_Species_getTimeCourse,
                setup_model,
                teardown_model);
  tcase_add_test(tc_Species_getTimeCourse, test_Species_getTimeCourse);
  suite_add_tcase(s, tc_Species_getTimeCourse);

  tc_Parameter_getTimeCourse = tcase_create("Parameter_getTimeCourse");
  tcase_add_checked_fixture(tc_Parameter_getTimeCourse,
                setup_model,
                teardown_model);
  tcase_add_test(tc_Parameter_getTimeCourse, test_Parameter_getTimeCourse);
  suite_add_tcase(s, tc_Parameter_getTimeCourse);

  tc_TimeCourse_getName = tcase_create("TimeCourse_getName");
  tcase_add_checked_fixture(tc_TimeCourse_getName,
                setup_model,
                teardown_model);
  tcase_add_test(tc_TimeCourse_getName, test_TimeCourse_getName);
  suite_add_tcase(s, tc_TimeCourse_getName);

  tc_TimeCourse_getNumValues = tcase_create("TimeCourse_getNumValues");
  tcase_add_checked_fixture(tc_TimeCourse_getNumValues,
                setup_model,
                teardown_model);
  tcase_add_test(tc_TimeCourse_getNumValues, test_TimeCourse_getNumValues);
  suite_add_tcase(s, tc_TimeCourse_getNumValues);

  tc_TimeCourse_getValue = tcase_create("TimeCourse_getValue");
  tcase_add_checked_fixture(tc_TimeCourse_getValue,
                setup_model,
                teardown_model);
  tcase_add_test(tc_TimeCourse_getValue, test_TimeCourse_getValue);
  suite_add_tcase(s, tc_TimeCourse_getValue);

  tc_TimeCourse_getSensitivity = tcase_create("TimeCourse_getSensitivity");
  tcase_add_checked_fixture(tc_TimeCourse_getSensitivity,
                setup_model,
                teardown_model);
  tcase_add_test(tc_TimeCourse_getSensitivity, test_TimeCourse_getSensitivity);
  suite_add_tcase(s, tc_TimeCourse_getSensitivity);

  tc_SBMLResults_dump = tcase_create("SBMLResults_dump");
  tcase_add_checked_fixture(tc_SBMLResults_dump,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_dump, test_SBMLResults_dump);
  suite_add_tcase(s, tc_SBMLResults_dump);

  tc_SBMLResults_dumpSpecies = tcase_create("SBMLResults_dumpSpecies");
  tcase_add_checked_fixture(tc_SBMLResults_dumpSpecies,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_dumpSpecies, test_SBMLResults_dumpSpecies);
  suite_add_tcase(s, tc_SBMLResults_dumpSpecies);

  tc_SBMLResults_dumpCompartments = tcase_create("SBMLResults_dumpCompartments");
  tcase_add_checked_fixture(tc_SBMLResults_dumpCompartments,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_dumpCompartments, test_SBMLResults_dumpCompartments);
  suite_add_tcase(s, tc_SBMLResults_dumpCompartments);

  tc_SBMLResults_dumpParameters = tcase_create("SBMLResults_dumpParameters");
  tcase_add_checked_fixture(tc_SBMLResults_dumpParameters,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_dumpParameters, test_SBMLResults_dumpParameters);
  suite_add_tcase(s, tc_SBMLResults_dumpParameters);

  tc_SBMLResults_dumpFluxes = tcase_create("SBMLResults_dumpFluxes");
  tcase_add_checked_fixture(tc_SBMLResults_dumpFluxes,
                setup_model,
                teardown_model);
  tcase_add_test(tc_SBMLResults_dumpFluxes, test_SBMLResults_dumpFluxes);
  suite_add_tcase(s, tc_SBMLResults_dumpFluxes);

  tc_SBMLResults_free = tcase_create("SBMLResults_free");
  tcase_add_test(tc_SBMLResults_free, test_SBMLResults_free);
  suite_add_tcase(s, tc_SBMLResults_free);

  tc_SBMLResultsArray_free = tcase_create("SBMLResultsArray_free");
  tcase_add_test(tc_SBMLResultsArray_free, test_SBMLResultsArray_free);
  suite_add_tcase(s, tc_SBMLResultsArray_free);

  tc_SBMLResultsArray_getNumResults = tcase_create("SBMLResultsArray_getNumResults");
  tcase_add_test(tc_SBMLResultsArray_getNumResults, test_SBMLResultsArray_getNumResults);
  suite_add_tcase(s, tc_SBMLResultsArray_getNumResults);

  tc_SBMLResultsArray_getResults = tcase_create("SBMLResultsArray_getResults");
  tcase_add_test(tc_SBMLResultsArray_getResults, test_SBMLResultsArray_getResults);
  suite_add_tcase(s, tc_SBMLResultsArray_getResults);

  return s;
}
