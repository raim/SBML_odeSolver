/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/odeModel.h>

/* fixtures */
static odeModel_t *model;

static void teardown_model(void)
{
	ODEModel_free(model);
}

/* helpers */
#define CHECK_VARIABLEINDEX(i, name) do {								\
		variableIndex_t *vi;											\
		vi = ODEModel_getVariableIndexByNum(model, i);					\
		ck_assert(vi != NULL);											\
		ck_assert_str_eq(VariableIndex_getName(vi, model), name);		\
		ck_assert(VariableIndex_getName(vi, model) == ODEModel_getVariableName(model, vi));	\
		ck_assert_int_eq(VariableIndex_getIndex(vi), i);				\
		VariableIndex_free(vi);											\
	} while (0)

/* test cases */
START_TEST(test_ODEModel_createFromFile_MAPK)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 8);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 10);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 4);
	ck_assert_int_eq(ODEModel_getNumValues(model), 22);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "MKKK");
	CHECK_VARIABLEINDEX(1, "MKKK_P");
	CHECK_VARIABLEINDEX(2, "MKK");
	CHECK_VARIABLEINDEX(3, "MKK_P");
	CHECK_VARIABLEINDEX(4, "MKK_PP");
	CHECK_VARIABLEINDEX(5, "MAPK");
	CHECK_VARIABLEINDEX(6, "MAPK_P");
	CHECK_VARIABLEINDEX(7, "MAPK_PP");
	CHECK_VARIABLEINDEX(8, "J0");
	CHECK_VARIABLEINDEX(9, "J1");
	CHECK_VARIABLEINDEX(10, "J2");
	CHECK_VARIABLEINDEX(11, "J3");
	CHECK_VARIABLEINDEX(12, "J4");
	CHECK_VARIABLEINDEX(13, "J5");
	CHECK_VARIABLEINDEX(14, "J6");
	CHECK_VARIABLEINDEX(15, "J7");
	CHECK_VARIABLEINDEX(16, "J8");
	CHECK_VARIABLEINDEX(17, "J9");
	CHECK_VARIABLEINDEX(18, "uVol");
	CHECK_VARIABLEINDEX(19, "V1");
	CHECK_VARIABLEINDEX(20, "Ki");
	CHECK_VARIABLEINDEX(21, "K1");
}
END_TEST

START_TEST(test_ODEModel_createFromFile_basic_model1_forward_l2)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 2);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 2);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 1);
	ck_assert_int_eq(ODEModel_getNumValues(model), 5);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "S1");
	CHECK_VARIABLEINDEX(1, "S2");
	CHECK_VARIABLEINDEX(2, "R1");
	CHECK_VARIABLEINDEX(3, "R2");
	CHECK_VARIABLEINDEX(4, "c");
}
END_TEST

START_TEST(test_ODEModel_createFromFile_basic)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("basic.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 2);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 2);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 2);
	ck_assert_int_eq(ODEModel_getNumValues(model), 6);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "S1");
	CHECK_VARIABLEINDEX(1, "S2");
	CHECK_VARIABLEINDEX(2, "R1");
	CHECK_VARIABLEINDEX(3, "R2");
	CHECK_VARIABLEINDEX(4, "c");
	CHECK_VARIABLEINDEX(5, "k_1");
}
END_TEST

START_TEST(test_ODEModel_createFromFile_events_1_event_1_assignment_l2)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 2);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 1);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 1);
	ck_assert_int_eq(ODEModel_getNumValues(model), 4);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "S1");
	CHECK_VARIABLEINDEX(1, "S2");
	CHECK_VARIABLEINDEX(2, "R");
	CHECK_VARIABLEINDEX(3, "compartment");
}
END_TEST

START_TEST(test_ODEModel_createFromFile_events_2_events_1_assignment_l2)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 2);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 1);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 1);
	ck_assert_int_eq(ODEModel_getNumValues(model), 4);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "S1");
	CHECK_VARIABLEINDEX(1, "S2");
	CHECK_VARIABLEINDEX(2, "R");
	CHECK_VARIABLEINDEX(3, "compartment");
}
END_TEST

START_TEST(test_ODEModel_createFromFile_huang96)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("huang96.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 22);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 20);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 1);
	ck_assert_int_eq(ODEModel_getNumValues(model), 43);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "E1");
	CHECK_VARIABLEINDEX(1, "E2");
	CHECK_VARIABLEINDEX(2, "KKK");
	CHECK_VARIABLEINDEX(3, "P_KKK");
	CHECK_VARIABLEINDEX(4, "KK");
	CHECK_VARIABLEINDEX(5, "P_KK");
	CHECK_VARIABLEINDEX(6, "PP_KK");
	CHECK_VARIABLEINDEX(7, "K");
	CHECK_VARIABLEINDEX(8, "P_K");
	CHECK_VARIABLEINDEX(9, "PP_K");
	CHECK_VARIABLEINDEX(10, "KPase");
	CHECK_VARIABLEINDEX(11, "KKPase");
	CHECK_VARIABLEINDEX(12, "E1_KKK");
	CHECK_VARIABLEINDEX(13, "E2_P_KKK");
	CHECK_VARIABLEINDEX(14, "P_KKK_KK");
	CHECK_VARIABLEINDEX(15, "P_KKK_P_KK");
	CHECK_VARIABLEINDEX(16, "PP_KK_K");
	CHECK_VARIABLEINDEX(17, "PP_KK_P_K");
	CHECK_VARIABLEINDEX(18, "KKPase_PP_KK");
	CHECK_VARIABLEINDEX(19, "KKPase_P_KK");
	CHECK_VARIABLEINDEX(20, "KPase_PP_K");
	CHECK_VARIABLEINDEX(21, "KPase_P_K");
	CHECK_VARIABLEINDEX(22, "r1a");
	CHECK_VARIABLEINDEX(23, "r1b");
	CHECK_VARIABLEINDEX(24, "r2a");
	CHECK_VARIABLEINDEX(25, "r2b");
	CHECK_VARIABLEINDEX(26, "r3a");
	CHECK_VARIABLEINDEX(27, "r3b");
	CHECK_VARIABLEINDEX(28, "r4a");
	CHECK_VARIABLEINDEX(29, "r4b");
	CHECK_VARIABLEINDEX(30, "r5a");
	CHECK_VARIABLEINDEX(31, "r5b");
	CHECK_VARIABLEINDEX(32, "r6a");
	CHECK_VARIABLEINDEX(33, "r6b");
	CHECK_VARIABLEINDEX(34, "r7a");
	CHECK_VARIABLEINDEX(35, "r7b");
	CHECK_VARIABLEINDEX(36, "r8a");
	CHECK_VARIABLEINDEX(37, "r8b");
	CHECK_VARIABLEINDEX(38, "r9a");
	CHECK_VARIABLEINDEX(39, "r9b");
	CHECK_VARIABLEINDEX(40, "r10a");
	CHECK_VARIABLEINDEX(41, "r10b");
	CHECK_VARIABLEINDEX(42, "compartment");
}
END_TEST

START_TEST(test_ODEModel_createFromFile_repressilator)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("repressilator.xml"));
	ck_assert(model != NULL);
	ck_assert_int_eq(ODEModel_getNalg(model), 0);
	ck_assert_int_eq(ODEModel_getNeq(model), 6);
	ck_assert_int_eq(ODEModel_getNumAssignments(model), 0);
	ck_assert_int_eq(ODEModel_getNumConstants(model), 4);
	ck_assert_int_eq(ODEModel_getNumValues(model), 10);
	ck_assert_int_eq(ODEModel_hasVariable(model, "no_such_variable"), 0);
	CHECK_VARIABLEINDEX(0, "x1");
	CHECK_VARIABLEINDEX(1, "x2");
	CHECK_VARIABLEINDEX(2, "x3");
	CHECK_VARIABLEINDEX(3, "y1");
	CHECK_VARIABLEINDEX(4, "y2");
	CHECK_VARIABLEINDEX(5, "y3");
	CHECK_VARIABLEINDEX(6, "compartment");
	CHECK_VARIABLEINDEX(7, "alpha");
	CHECK_VARIABLEINDEX(8, "beta");
	CHECK_VARIABLEINDEX(9, "rho");
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
