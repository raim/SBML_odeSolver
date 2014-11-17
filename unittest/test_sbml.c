/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/sbml.h>

/* test cases */
START_TEST(test_parseModel_MAPK)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

START_TEST(test_parseModel_basic_model1_forward_l2)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

START_TEST(test_parseModel_basic)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

START_TEST(test_parseModel_events_1_event_1_assignment_l2)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

START_TEST(test_parseModel_events_2_events_1_assignment_l2)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

START_TEST(test_parseModel_huang96)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("huang96.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

START_TEST(test_parseModel_repressilator)
{
	SBMLDocument_t *d;

	d = parseModel(EXAMPLES_FILENAME("repressilator.xml"), 0, 1);
	ck_assert(d != NULL);
	SBMLDocument_free(d);
}
END_TEST

/* public */
Suite *create_suite_sbml(void)
{
	Suite *s;
	TCase *tc_parseModel;

	s = suite_create("sbml");

	tc_parseModel = tcase_create("parseModel");
	tcase_add_test(tc_parseModel, test_parseModel_MAPK);
	tcase_add_test(tc_parseModel, test_parseModel_basic_model1_forward_l2);
	tcase_add_test(tc_parseModel, test_parseModel_basic);
	tcase_add_test(tc_parseModel, test_parseModel_events_1_event_1_assignment_l2);
	tcase_add_test(tc_parseModel, test_parseModel_events_2_events_1_assignment_l2);
	tcase_add_test(tc_parseModel, test_parseModel_huang96);
	tcase_add_test(tc_parseModel, test_parseModel_repressilator);
	suite_add_tcase(s, tc_parseModel);

	return s;
}
