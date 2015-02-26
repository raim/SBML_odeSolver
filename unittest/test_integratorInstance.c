/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/integratorInstance.h>

/* fixtures */
static odeModel_t *model;

static void teardown_model(void)
{
	ODEModel_free(model);
}

/* test cases */
START_TEST(test_IntegratorInstance_create)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	ck_assert(ii != NULL);
	ck_assert(IntegratorInstance_getSettings(ii) == cs);
	CHECK_DOUBLE_WITH_TOLERANCE(IntegratorInstance_getTime(ii), 0.0);
	ck_assert_int_eq(IntegratorInstance_timeCourseCompleted(ii), 0);
	ck_assert(IntegratorInstance_getIntegrationTime(ii) == 0.0);
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_integrate)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	int r;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	r = IntegratorInstance_integrate(ii);
	ck_assert_int_eq(r, 1);
	CHECK_DOUBLE_WITH_TOLERANCE(IntegratorInstance_getTime(ii), 1.0);
	ck_assert_int_eq(IntegratorInstance_timeCourseCompleted(ii), 1);
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_getResults)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	int r;
	const cvodeResults_t *cr;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	r = IntegratorInstance_integrate(ii);
	ck_assert_int_eq(r, 1);
	cr = IntegratorInstance_getResults(ii);
	ck_assert(ii->results == cr);
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_createResults)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	int r;
	cvodeResults_t *cr;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	r = IntegratorInstance_integrate(ii);
	ck_assert_int_eq(r, 1);
	cr = IntegratorInstance_createResults(ii);
	ck_assert(ii->results != cr);
	CvodeResults_free(cr);
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_printResults)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	int r;
	FILE *fp;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	r = IntegratorInstance_integrate(ii);
	ck_assert_int_eq(r, 1);
	OPEN_TMPFILE_OR_ABORT(fp);
	IntegratorInstance_printResults(ii, fp);
	/* TODO: check the printed text */
	fclose(fp);
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_updateModel)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	int r;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	r = IntegratorInstance_integrate(ii);
	ck_assert_int_eq(r, 1);
	r = IntegratorInstance_updateModel(ii);
	ck_assert_int_eq(r, 0); /* TODO: expected? */
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_printStatistics)
{
	integratorInstance_t *ii;
	cvodeSettings_t *cs;
	int r;
	FILE *fp;
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
	r = IntegratorInstance_integrate(ii);
	ck_assert_int_eq(r, 1);
	OPEN_TMPFILE_OR_ABORT(fp);
	IntegratorInstance_printStatistics(ii, fp);
	/* TODO: check the printed text */
	fclose(fp);
	CvodeSettings_free(cs);
	IntegratorInstance_free(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_free)
{
	IntegratorInstance_free(NULL); /* freeing NULL is safe */
}
END_TEST

/* public */
Suite *create_suite_integratorInstance(void)
{
	Suite *s;
	TCase *tc_IntegratorInstance_create;
	TCase *tc_IntegratorInstance_integrate;
	TCase *tc_IntegratorInstance_getResults;
	TCase *tc_IntegratorInstance_createResults;
	TCase *tc_IntegratorInstance_printResults;
	TCase *tc_IntegratorInstance_updateModel;
	TCase *tc_IntegratorInstance_printStatistics;
	TCase *tc_IntegratorInstance_free;

	s = suite_create("integratorInstance");

	tc_IntegratorInstance_create = tcase_create("IntegratorInstance_create");
	tcase_add_checked_fixture(tc_IntegratorInstance_create,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_create, test_IntegratorInstance_create);
	suite_add_tcase(s, tc_IntegratorInstance_create);

	tc_IntegratorInstance_integrate = tcase_create("IntegratorInstance_integrate");
	tcase_add_checked_fixture(tc_IntegratorInstance_integrate,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_integrate, test_IntegratorInstance_integrate);
	suite_add_tcase(s, tc_IntegratorInstance_integrate);

	tc_IntegratorInstance_getResults = tcase_create("IntegratorInstance_getResults");
	tcase_add_checked_fixture(tc_IntegratorInstance_getResults,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_getResults, test_IntegratorInstance_getResults);
	suite_add_tcase(s, tc_IntegratorInstance_getResults);

	tc_IntegratorInstance_createResults = tcase_create("IntegratorInstance_createResults");
	tcase_add_checked_fixture(tc_IntegratorInstance_createResults,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_createResults, test_IntegratorInstance_createResults);
	suite_add_tcase(s, tc_IntegratorInstance_createResults);

	tc_IntegratorInstance_printResults = tcase_create("IntegratorInstance_printResults");
	tcase_add_checked_fixture(tc_IntegratorInstance_printResults,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_printResults, test_IntegratorInstance_printResults);
	suite_add_tcase(s, tc_IntegratorInstance_printResults);

	tc_IntegratorInstance_updateModel = tcase_create("IntegratorInstance_updateModel");
	tcase_add_checked_fixture(tc_IntegratorInstance_updateModel,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_updateModel, test_IntegratorInstance_updateModel);
	suite_add_tcase(s, tc_IntegratorInstance_updateModel);

	tc_IntegratorInstance_printStatistics = tcase_create("IntegratorInstance_printStatistics");
	tcase_add_checked_fixture(tc_IntegratorInstance_printStatistics,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_IntegratorInstance_printStatistics, test_IntegratorInstance_printStatistics);
	suite_add_tcase(s, tc_IntegratorInstance_printStatistics);

	tc_IntegratorInstance_free = tcase_create("IntegratorInstance_free");
	tcase_add_test(tc_IntegratorInstance_free, test_IntegratorInstance_free);
	suite_add_tcase(s, tc_IntegratorInstance_free);

	return s;
}
