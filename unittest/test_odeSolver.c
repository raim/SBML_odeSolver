/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/odeSolver.h>
#include <sbmlsolver/sbml.h>

/* fixtures */
static SBMLDocument_t *doc = NULL;

Model_t *model;

static cvodeSettings_t *cs;

static SBMLResults_t *results = NULL;

static varySettings_t *vs = NULL;

static void teardown_doc(void)
{
	SBMLDocument_free(doc);
}

static void setup_cs(void)
{
	cs = CvodeSettings_create();
}

static void teardown_cs(void)
{
	CvodeSettings_free(cs);
}

static void teardown_results(void)
{
	SBMLResults_free(results);
}

static void setup_vs(void)
{
	vs = VarySettings_allocate(3, 4);
}

static void teardown_vs(void)
{
	VarySettings_free(vs);
}

/* test cases */
START_TEST(test_SBML_odeSolver_MAPK)
{
	doc = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	results = SBML_odeSolver(doc, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_basic_model1_forward_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_basic)
{
	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_events_1_event_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_events_2_events_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_huang96)
{
	doc = parseModel(EXAMPLES_FILENAME("huang96.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_repressilator)
{
	doc = parseModel(EXAMPLES_FILENAME("repressilator.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_VarySettings_allocate)
{
	vs = VarySettings_allocate(7, 77);
	ck_assert(vs != NULL);
	ck_assert_int_eq(vs->nrparams, 7);
	ck_assert_int_eq(vs->nrdesignpoints, 77);
}
END_TEST

START_TEST(test_VarySettings_free)
{
	VarySettings_free(NULL); /* freeing NULL is safe */
}
END_TEST

START_TEST(test_VarySettings_addDesignPoint)
{
	static const double params0[] = {0.0, 2.5, 4.0};
	static const double params1[] = {0.1, -0.1, 4.2};
	static const double params2[] = {0.2, -1.4, 5.9};
	static const double params3[] = {0.3, -2.9, 5.1};
	static const double params4[] = {0.4, -5.0, 4.1};
	int r;
	r = VarySettings_addDesignPoint(vs, params0);
	ck_assert_int_eq(r, 1);
	r = VarySettings_addDesignPoint(vs, params1);
	ck_assert_int_eq(r, 2);
	r = VarySettings_addDesignPoint(vs, params2);
	ck_assert_int_eq(r, 3);
	r = VarySettings_addDesignPoint(vs, params3);
	ck_assert_int_eq(r, 4);
	r = VarySettings_addDesignPoint(vs, params4);
	ck_assert_int_eq(r, 0);
}
END_TEST

START_TEST(test_VarySettings_addParameter)
{
	int r;
	r = VarySettings_addParameter(vs, "foo", NULL); /* global */
	ck_assert_int_eq(r, 1);
	ck_assert_str_eq(vs->id[0], "foo");
	ck_assert(vs->rid[0] == NULL);
	r = VarySettings_addParameter(vs, "foo", "local");
	ck_assert_int_eq(r, 2);
	ck_assert_str_eq(vs->id[1], "foo");
	ck_assert_str_eq(vs->rid[1], "local");
	r = VarySettings_addParameter(vs, "bar", "local");
	ck_assert_int_eq(r, 3);
	ck_assert_str_eq(vs->id[2], "bar");
	ck_assert_str_eq(vs->rid[2], "local");
	r = VarySettings_addParameter(vs, "failure", "full");
	ck_assert_int_eq(r, 0);
}
END_TEST

/* public */
Suite *create_suite_odeSolver(void)
{
	Suite *s;
	TCase *tc_SBML_odeSolver;
	TCase *tc_Model_odeSolver;
	TCase *tc_VarySettings_allocate;
	TCase *tc_VarySettings_free;
	TCase *tc_VarySettings_addDesignPoint;
	TCase *tc_VarySettings_addParameter;

	s = suite_create("odeSolver");

	tc_SBML_odeSolver = tcase_create("SBML_odeSolver");
	tcase_add_checked_fixture(tc_SBML_odeSolver,
							  NULL,
							  teardown_doc);
	tcase_add_checked_fixture(tc_SBML_odeSolver,
							  setup_cs,
							  teardown_cs);
	tcase_add_checked_fixture(tc_SBML_odeSolver,
							  NULL,
							  teardown_results);
	tcase_add_test(tc_SBML_odeSolver, test_SBML_odeSolver_MAPK);
	suite_add_tcase(s, tc_SBML_odeSolver);

	tc_Model_odeSolver = tcase_create("Model_odeSolver");
	tcase_add_checked_fixture(tc_Model_odeSolver,
							  NULL,
							  teardown_doc);
	tcase_add_checked_fixture(tc_Model_odeSolver,
							  setup_cs,
							  teardown_cs);
	tcase_add_checked_fixture(tc_Model_odeSolver,
							  NULL,
							  teardown_results);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_basic_model1_forward_l2);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_basic);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_events_1_event_1_assignment_l2);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_events_2_events_1_assignment_l2);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_huang96);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_repressilator);
	suite_add_tcase(s, tc_Model_odeSolver);

	tc_VarySettings_allocate = tcase_create("VarySettings_allocate");
	tcase_add_checked_fixture(tc_VarySettings_allocate,
							  NULL,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_allocate, test_VarySettings_allocate);
	suite_add_tcase(s, tc_VarySettings_allocate);

	tc_VarySettings_free = tcase_create("VarySettings_free");
	tcase_add_test(tc_VarySettings_free, test_VarySettings_free);
	suite_add_tcase(s, tc_VarySettings_free);

	tc_VarySettings_addDesignPoint = tcase_create("VarySettings_addDesignPoint");
	tcase_add_checked_fixture(tc_VarySettings_addDesignPoint,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_addDesignPoint, test_VarySettings_addDesignPoint);
	suite_add_tcase(s, tc_VarySettings_addDesignPoint);

	tc_VarySettings_addParameter = tcase_create("VarySettings_addParameter");
	tcase_add_checked_fixture(tc_VarySettings_addParameter,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_addParameter, test_VarySettings_addParameter);
	suite_add_tcase(s, tc_VarySettings_addParameter);

	return s;
}
