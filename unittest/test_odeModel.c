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
#define CHECK_VI_EQ(vi, expected) do {			\
		const ASTNode_t *node;					\
		char *actual;							\
		node = ODEModel_getOde(model, (vi));	\
		ck_assert(node != NULL);				\
		actual = SBML_formulaToString(node);	\
		ck_assert_str_eq(actual, (expected));	\
		free(actual);							\
	} while (0)

#define CHECK_VI_ASSIGNMENT(vi, expected) do {		\
		const ASTNode_t *node;						\
		char *actual;								\
		node = ODEModel_getAssignment(model, (vi));	\
		ck_assert(node != NULL);					\
		actual = SBML_formulaToString(node);		\
		ck_assert_str_eq(actual, (expected));		\
		free(actual);								\
	} while (0)

#define CHECK_VARIABLEINDEX(i, name) do {								\
		variableIndex_t *vi;											\
		vi = ODEModel_getVariableIndexByNum(model, (i));				\
		ck_assert(vi != NULL);											\
		ck_assert_str_eq(VariableIndex_getName(vi, model), (name));		\
		ck_assert(VariableIndex_getName(vi, model) == ODEModel_getVariableName(model, vi));	\
		ck_assert_int_eq(VariableIndex_getIndex(vi), (i));				\
		VariableIndex_free(vi);											\
	} while (0)

#define CHECK_EQ(i, name, expected) do {								\
		variableIndex_t *vi;											\
		vi = ODEModel_getVariableIndexByNum(model, (i));				\
		ck_assert(vi != NULL);											\
		ck_assert_str_eq(VariableIndex_getName(vi, model), (name));		\
		ck_assert(VariableIndex_getName(vi, model) == ODEModel_getVariableName(model, vi));	\
		ck_assert_int_eq(VariableIndex_getIndex(vi), (i));				\
		CHECK_VI_EQ(vi, expected);										\
		VariableIndex_free(vi);											\
	} while (0)

#define CHECK_ASSIGNMENT(i, name, expected) do {						\
		variableIndex_t *vi;											\
		vi = ODEModel_getVariableIndexByNum(model, (i));				\
		ck_assert(vi != NULL);											\
		ck_assert_str_eq(VariableIndex_getName(vi, model), (name));		\
		ck_assert(VariableIndex_getName(vi, model) == ODEModel_getVariableName(model, vi));	\
		ck_assert_int_eq(VariableIndex_getIndex(vi), (i));				\
		CHECK_VI_ASSIGNMENT(vi, expected);								\
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
	CHECK_EQ(0, "MKKK", "(J1 - J0) / uVol");
	CHECK_EQ(1, "MKKK_P", "(J0 - J1) / uVol");
	CHECK_EQ(2, "MKK", "(J5 - J2) / uVol");
	CHECK_EQ(3, "MKK_P", "(J2 - J3 + J4 - J5) / uVol");
	CHECK_EQ(4, "MKK_PP", "(J3 - J4) / uVol");
	CHECK_EQ(5, "MAPK", "(J9 - J6) / uVol");
	CHECK_EQ(6, "MAPK_P", "(J6 - J7 + J8 - J9) / uVol");
	CHECK_EQ(7, "MAPK_PP", "(J7 - J8) / uVol");
	CHECK_ASSIGNMENT(8, "J0", "V1 * MKKK / ((1 + (MAPK_PP / Ki)^1) * (K1 + MKKK))");
	CHECK_ASSIGNMENT(9, "J1", "0.25 * MKKK_P / (8 + MKKK_P)");
	CHECK_ASSIGNMENT(10, "J2", "0.025 * MKKK_P * MKK / (15 + MKK)");
	CHECK_ASSIGNMENT(11, "J3", "0.025 * MKKK_P * MKK_P / (15 + MKK_P)");
	CHECK_ASSIGNMENT(12, "J4", "0.75 * MKK_PP / (15 + MKK_PP)");
	CHECK_ASSIGNMENT(13, "J5", "0.75 * MKK_P / (15 + MKK_P)");
	CHECK_ASSIGNMENT(14, "J6", "0.025 * MKK_PP * MAPK / (15 + MAPK)");
	CHECK_ASSIGNMENT(15, "J7", "0.025 * MKK_PP * MAPK_P / (15 + MAPK_P)");
	CHECK_ASSIGNMENT(16, "J8", "0.5 * MAPK_PP / (15 + MAPK_PP)");
	CHECK_ASSIGNMENT(17, "J9", "0.5 * MAPK_P / (15 + MAPK_P)");
	CHECK_VARIABLEINDEX(18, "uVol");
	CHECK_VARIABLEINDEX(19, "V1");
	CHECK_VARIABLEINDEX(20, "Ki");
	CHECK_VARIABLEINDEX(21, "K1");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 10);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
	CHECK_EQ(0, "S1", "(R2 - R1) / c");
	CHECK_EQ(1, "S2", "(R1 - R2) / c");
	CHECK_ASSIGNMENT(2, "R1", "1 * S1");
	CHECK_ASSIGNMENT(3, "R2", "0 * S2");
	CHECK_VARIABLEINDEX(4, "c");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 2);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
	CHECK_EQ(0, "S1", "(R2 - R1) / c");
	CHECK_EQ(1, "S2", "(R1 - R2) / c");
	CHECK_ASSIGNMENT(2, "R1", "k_1 * S1");
	CHECK_ASSIGNMENT(3, "R2", "0 * S2");
	CHECK_VARIABLEINDEX(4, "c");
	CHECK_VARIABLEINDEX(5, "k_1");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 2);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
	CHECK_EQ(0, "S1", "-(R / compartment)");
	CHECK_EQ(1, "S2", "R / compartment");
	CHECK_ASSIGNMENT(2, "R", "S1");
	CHECK_VARIABLEINDEX(3, "compartment");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 1);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
	CHECK_EQ(0, "S1", "-(R / compartment)");
	CHECK_EQ(1, "S2", "R / compartment");
	CHECK_ASSIGNMENT(2, "R", "S1");
	CHECK_VARIABLEINDEX(3, "compartment");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 1);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
	CHECK_EQ(0, "E1", "(r1b - r1a) / compartment");
	CHECK_EQ(1, "E2", "(r2b - r2a) / compartment");
	CHECK_EQ(2, "KKK", "(r2b - r1a) / compartment");
	CHECK_EQ(3, "P_KKK", "(r1b - r2a - r3a + r3b - r5a + r5b) / compartment");
	CHECK_EQ(4, "KK", "(r4b - r3a) / compartment");
	CHECK_EQ(5, "P_KK", "(r3b - r4a - r5a + r6b) / compartment");
	CHECK_EQ(6, "PP_KK", "(r5b - r6a - r7a + r7b - r9a + r9b) / compartment");
	CHECK_EQ(7, "K", "(r8b - r7a) / compartment");
	CHECK_EQ(8, "P_K", "(r7b - r8a - r9a + r10b) / compartment");
	CHECK_EQ(9, "PP_K", "(r9b - r10a) / compartment");
	CHECK_EQ(10, "KPase", "(r8b - r8a - r10a + r10b) / compartment");
	CHECK_EQ(11, "KKPase", "(r4b - r4a - r6a + r6b) / compartment");
	CHECK_EQ(12, "E1_KKK", "(r1a - r1b) / compartment");
	CHECK_EQ(13, "E2_P_KKK", "(r2a - r2b) / compartment");
	CHECK_EQ(14, "P_KKK_KK", "(r3a - r3b) / compartment");
	CHECK_EQ(15, "P_KKK_P_KK", "(r5a - r5b) / compartment");
	CHECK_EQ(16, "PP_KK_K", "(r7a - r7b) / compartment");
	CHECK_EQ(17, "PP_KK_P_K", "(r9a - r9b) / compartment");
	CHECK_EQ(18, "KKPase_PP_KK", "(r6a - r6b) / compartment");
	CHECK_EQ(19, "KKPase_P_KK", "(r4a - r4b) / compartment");
	CHECK_EQ(20, "KPase_PP_K", "(r10a - r10b) / compartment");
	CHECK_EQ(21, "KPase_P_K", "(r8a - r8b) / compartment");
	CHECK_ASSIGNMENT(22, "r1a", "1000 * E1 * KKK - 150 * E1_KKK");
	CHECK_ASSIGNMENT(23, "r1b", "150 * E1_KKK");
	CHECK_ASSIGNMENT(24, "r2a", "1000 * E2 * P_KKK - 150 * E2_P_KKK");
	CHECK_ASSIGNMENT(25, "r2b", "150 * E2_P_KKK");
	CHECK_ASSIGNMENT(26, "r3a", "1000 * KK * P_KKK - 150 * P_KKK_KK");
	CHECK_ASSIGNMENT(27, "r3b", "150 * P_KKK_KK");
	CHECK_ASSIGNMENT(28, "r4a", "1000 * P_KK * KKPase - 150 * KKPase_P_KK");
	CHECK_ASSIGNMENT(29, "r4b", "150 * KKPase_P_KK");
	CHECK_ASSIGNMENT(30, "r5a", "1000 * P_KK * P_KKK - 150 * P_KKK_P_KK");
	CHECK_ASSIGNMENT(31, "r5b", "150 * P_KKK_P_KK");
	CHECK_ASSIGNMENT(32, "r6a", "1000 * PP_KK * KKPase - 150 * KKPase_PP_KK");
	CHECK_ASSIGNMENT(33, "r6b", "150 * KKPase_PP_KK");
	CHECK_ASSIGNMENT(34, "r7a", "1000 * K * PP_KK - 150 * PP_KK_K");
	CHECK_ASSIGNMENT(35, "r7b", "150 * PP_KK_K");
	CHECK_ASSIGNMENT(36, "r8a", "1000 * P_K * KPase - 150 * KPase_P_K");
	CHECK_ASSIGNMENT(37, "r8b", "150 * KPase_P_K");
	CHECK_ASSIGNMENT(38, "r9a", "1000 * P_K * PP_KK - 150 * PP_KK_P_K");
	CHECK_ASSIGNMENT(39, "r9b", "150 * PP_KK_P_K");
	CHECK_ASSIGNMENT(40, "r10a", "1000 * PP_K * KPase - 150 * KPase_PP_K");
	CHECK_ASSIGNMENT(41, "r10b", "150 * KPase_PP_K");
	CHECK_VARIABLEINDEX(42, "compartment");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 20);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
	CHECK_EQ(0, "x1", "beta * (y1 - x1)");
	CHECK_EQ(1, "x2", "beta * (y2 - x2)");
	CHECK_EQ(2, "x3", "beta * (y3 - x3)");
	CHECK_EQ(3, "y1", "alpha * x1 / (1 + x1 + rho * x3) - y1");
	CHECK_EQ(4, "y2", "alpha * x2 / (1 + x2 + rho * x1) - y2");
	CHECK_EQ(5, "y3", "alpha * x3 / (1 + x3 + rho * x2) - y3");
	CHECK_VARIABLEINDEX(6, "compartment");
	CHECK_VARIABLEINDEX(7, "alpha");
	CHECK_VARIABLEINDEX(8, "beta");
	CHECK_VARIABLEINDEX(9, "rho");
	ck_assert(!ODEModel_hasCycle(model));
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeODEs(model), 0);
	ck_assert_int_eq(ODEModel_getNumAssignmentsBeforeEvents(model), 0);
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
