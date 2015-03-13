/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/cvodeData.h>

/* fixtures */
static odeModel_t *model;

static void teardown_model(void)
{
	ODEModel_free(model);
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

/* public */
Suite *create_suite_cvodeData(void)
{
	Suite *s;
	TCase *tc_CvodeData_create;
	TCase *tc_CvodeData_initializeValues;
	TCase *tc_CvodeData_initialize;

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

	return s;
}
