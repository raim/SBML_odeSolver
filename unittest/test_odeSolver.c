/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/odeSolver.h>
#include <sbmlsolver/sbml.h>

/* fixtures */
static SBMLDocument_t *doc = NULL;

Model_t *model;

static cvodeSettings_t *cs;

static SBMLResults_t *results = NULL;

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

/* public */
Suite *create_suite_odeSolver(void)
{
	Suite *s;
	TCase *tc_SBML_odeSolver;
	TCase *tc_Model_odeSolver;

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

	return s;
}
