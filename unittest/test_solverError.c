/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/solverError.h>

/* helpers */
static void add_some_errors(void)
{
	SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_ODE_COULD_NOT_BE_CONSTRUCTED_FOR_SPECIES, "fatal0");
	SolverError_error(ERROR_ERROR_TYPE, SOLVER_ERROR_THE_MODEL_CONTAINS_EVENTS, "error0");
	SolverError_error(WARNING_ERROR_TYPE, SOLVER_ERROR_THE_MODEL_CONTAINS_ALGEBRAIC_RULES, "warning0");
	SolverError_error(MESSAGE_ERROR_TYPE, SOLVER_ERROR_ODE_MODEL_COULD_NOT_BE_CONSTRUCTED, "message0");
	SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_NO_KINETIC_LAW_FOUND_FOR_REACTION, "fatal1");
	SolverError_error(ERROR_ERROR_TYPE, SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED, "error1");
}

/* test cases */
START_TEST(test_SolverError_getNum)
{
	int n;

	SolverError_clear();
	n = SolverError_getNum(FATAL_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
	n = SolverError_getNum(ERROR_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
	n = SolverError_getNum(WARNING_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
	n = SolverError_getNum(MESSAGE_ERROR_TYPE);
	ck_assert_int_eq(n, 0);

	add_some_errors();
	n = SolverError_getNum(FATAL_ERROR_TYPE);
	ck_assert_int_eq(n, 2);
	n = SolverError_getNum(ERROR_ERROR_TYPE);
	ck_assert_int_eq(n, 2);
	n = SolverError_getNum(WARNING_ERROR_TYPE);
	ck_assert_int_eq(n, 1);
	n = SolverError_getNum(MESSAGE_ERROR_TYPE);
	ck_assert_int_eq(n, 1);
}
END_TEST

START_TEST(test_SolverError_getMessage)
{
	char *m;

	add_some_errors();
	m = SolverError_getMessage(FATAL_ERROR_TYPE, 0);
	ck_assert(m != NULL);
	ck_assert_str_eq(m, "fatal0");
	m = SolverError_getMessage(FATAL_ERROR_TYPE, 1);
	ck_assert(m != NULL);
	ck_assert_str_eq(m, "fatal1");
	m = SolverError_getMessage(ERROR_ERROR_TYPE, 0);
	ck_assert(m != NULL);
	ck_assert_str_eq(m, "error0");
	m = SolverError_getMessage(ERROR_ERROR_TYPE, 1);
	ck_assert(m != NULL);
	ck_assert_str_eq(m, "error1");
	m = SolverError_getMessage(WARNING_ERROR_TYPE, 0);
	ck_assert(m != NULL);
	ck_assert_str_eq(m, "warning0");
	m = SolverError_getMessage(MESSAGE_ERROR_TYPE, 0);
	ck_assert(m != NULL);
	ck_assert_str_eq(m, "message0");
}
END_TEST

START_TEST(test_SolverError_getCode)
{
	errorCode_t e;

	add_some_errors();
	e = SolverError_getCode(FATAL_ERROR_TYPE, 0);
	ck_assert(e == SOLVER_ERROR_ODE_COULD_NOT_BE_CONSTRUCTED_FOR_SPECIES);
	e = SolverError_getCode(FATAL_ERROR_TYPE, 1);
	ck_assert(e == SOLVER_ERROR_NO_KINETIC_LAW_FOUND_FOR_REACTION);
	e = SolverError_getCode(ERROR_ERROR_TYPE, 0);
	ck_assert(e == SOLVER_ERROR_THE_MODEL_CONTAINS_EVENTS);
	e = SolverError_getCode(ERROR_ERROR_TYPE, 1);
	ck_assert(e == SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED);
	e = SolverError_getCode(WARNING_ERROR_TYPE, 0);
	ck_assert(e == SOLVER_ERROR_THE_MODEL_CONTAINS_ALGEBRAIC_RULES);
	e = SolverError_getCode(MESSAGE_ERROR_TYPE, 0);
	ck_assert(e == SOLVER_ERROR_ODE_MODEL_COULD_NOT_BE_CONSTRUCTED);
}
END_TEST

START_TEST(test_SolverError_getLastCode)
{
	errorCode_t e;

	add_some_errors();
	e = SolverError_getLastCode(FATAL_ERROR_TYPE);
	ck_assert(e == SOLVER_ERROR_NO_KINETIC_LAW_FOUND_FOR_REACTION);
	e = SolverError_getLastCode(ERROR_ERROR_TYPE);
	ck_assert(e == SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED);
	e = SolverError_getLastCode(WARNING_ERROR_TYPE);
	ck_assert(e == SOLVER_ERROR_THE_MODEL_CONTAINS_ALGEBRAIC_RULES);
	e = SolverError_getLastCode(MESSAGE_ERROR_TYPE);
	ck_assert(e == SOLVER_ERROR_ODE_MODEL_COULD_NOT_BE_CONSTRUCTED);
}
END_TEST

START_TEST(test_SolverError_haltOnErrors)
{
	SolverError_clear();
	SolverError_haltOnErrors();
	/* SolverError_haltOnErrors() returned */
}
END_TEST

START_TEST(test_SolverError_clear)
{
	int n;

	add_some_errors();
	SolverError_clear();
	n = SolverError_getNum(FATAL_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
	n = SolverError_getNum(ERROR_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
	n = SolverError_getNum(WARNING_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
	n = SolverError_getNum(MESSAGE_ERROR_TYPE);
	ck_assert_int_eq(n, 0);
}
END_TEST

START_TEST(test_SolverError_dumpToString)
{
	char *s;

	SolverError_clear();
	add_some_errors();
	s = SolverError_dumpToString();
	ck_assert(s != NULL);
	ck_assert_str_eq(s,
					 "Fatal Error\t100000\tfatal0\n"
					 "Fatal Error\t100004\tfatal1\n"
					 "      Error\t100001\terror0\n"
					 "      Error\t100005\terror1\n"
					 "    Warning\t100002\twarning0\n"
					 "    Message\t100003\tmessage0\n");
	SolverError_freeDumpString(s);
}
END_TEST

/* public */
Suite *create_suite_solverError(void)
{
	Suite *s;
	TCase *tc_SolverError_getNum;
	TCase *tc_SolverError_getMessage;
	TCase *tc_SolverError_getCode;
	TCase *tc_SolverError_getLastCode;
	TCase *tc_SolverError_haltOnErrors;
	TCase *tc_SolverError_clear;
	TCase *tc_SolverError_dumpToString;

	s = suite_create("solverError");

	tc_SolverError_getNum = tcase_create("SolverError_getNum");
	tcase_add_test(tc_SolverError_getNum, test_SolverError_getNum);
	suite_add_tcase(s, tc_SolverError_getNum);

	tc_SolverError_getMessage = tcase_create("SolverError_getMessage");
	tcase_add_test(tc_SolverError_getMessage, test_SolverError_getMessage);
	suite_add_tcase(s, tc_SolverError_getMessage);

	tc_SolverError_getCode = tcase_create("SolverError_getCode");
	tcase_add_test(tc_SolverError_getCode, test_SolverError_getCode);
	suite_add_tcase(s, tc_SolverError_getCode);

	tc_SolverError_getLastCode = tcase_create("SolverError_getLastCode");
	tcase_add_test(tc_SolverError_getLastCode, test_SolverError_getLastCode);
	suite_add_tcase(s, tc_SolverError_getLastCode);

	tc_SolverError_haltOnErrors = tcase_create("SolverError_haltOnErrors");
	tcase_add_test(tc_SolverError_haltOnErrors, test_SolverError_haltOnErrors);
	suite_add_tcase(s, tc_SolverError_haltOnErrors);

	tc_SolverError_clear = tcase_create("SolverError_clear");
	tcase_add_test(tc_SolverError_clear, test_SolverError_clear);
	suite_add_tcase(s, tc_SolverError_clear);

	tc_SolverError_dumpToString = tcase_create("SolverError_dumpToString");
	tcase_add_test(tc_SolverError_dumpToString, test_SolverError_dumpToString);
	suite_add_tcase(s, tc_SolverError_dumpToString);

	return s;
}
