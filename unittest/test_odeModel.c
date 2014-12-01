/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/odeModel.h>

/* fixtures */
static odeModel_t *model;

void teardown_model(void)
{
	ODEModel_free(model);
}

/* test cases */
START_TEST(test_ODEModel_createFromFile_MAPK)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	ck_assert(model != NULL);
}
END_TEST

START_TEST(test_ODEModel_createFromFile_basic_model1_forward_l2)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"));
	ck_assert(model != NULL);
}
END_TEST

START_TEST(test_ODEModel_createFromFile_basic)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("basic.xml"));
	ck_assert(model != NULL);
}
END_TEST

START_TEST(test_ODEModel_createFromFile_events_1_event_1_assignment_l2)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"));
	ck_assert(model != NULL);
}
END_TEST

START_TEST(test_ODEModel_createFromFile_events_2_events_1_assignment_l2)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"));
	ck_assert(model != NULL);
}
END_TEST

START_TEST(test_ODEModel_createFromFile_huang96)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("huang96.xml"));
	ck_assert(model != NULL);
}
END_TEST

START_TEST(test_ODEModel_createFromFile_repressilator)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("repressilator.xml"));
	ck_assert(model != NULL);
}
END_TEST

/* public */
Suite *create_suite_odeModel(void)
{
	Suite *s;
	TCase *tc_ODEModel_createFromFile;

	s = suite_create("odeModel");

	tc_ODEModel_createFromFile = tcase_create("ODEModel_createFromFile");
	tcase_add_checked_fixture(tc_ODEModel_createFromFile,
							  NULL,
							  teardown_model);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_MAPK);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_basic_model1_forward_l2);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_basic);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_events_1_event_1_assignment_l2);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_events_2_events_1_assignment_l2);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_huang96);
	tcase_add_test(tc_ODEModel_createFromFile, test_ODEModel_createFromFile_repressilator);
	suite_add_tcase(s, tc_ODEModel_createFromFile);

	return s;
}
