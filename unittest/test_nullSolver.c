/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/nullSolver.h>

/* fixtures */
static odeModel_t *model = NULL;

static cvodeSettings_t *cs = NULL;

static integratorInstance_t *ii = NULL;

static void setup_integratorInstance(void)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	ii = IntegratorInstance_create(model, cs);
}

static void teardown_integratorInstance(void)
{
#if 0 /* FIXME */
	IntegratorInstance_free(ii);
#endif
	CvodeSettings_free(cs);
	ODEModel_free(model);
}

/* test cases */
START_TEST(test_IntegratorInstance_createKINSolverStructures)
{
	int r;
	r = IntegratorInstance_createKINSolverStructures(ii);
	ck_assert_int_eq(r, 1);
}
END_TEST

START_TEST(test_IntegratorInstance_freeKINSolverStructures)
{
#if 0 /* FIXME */
	IntegratorInstance_createKINSolverStructures(ii);
	/* It is OK to call IntegratorInstance_freeKINSolverStructures consecutively. */
	IntegratorInstance_freeKINSolverStructures(ii);
	IntegratorInstance_freeKINSolverStructures(ii);
#endif
}
END_TEST

START_TEST(test_IntegratorInstance_nullSolver)
{
	int r;
	IntegratorInstance_createKINSolverStructures(ii);
	r = IntegratorInstance_nullSolver(ii);
	ck_assert_int_eq(r, 1);
}
END_TEST

START_TEST(test_IntegratorInstance_printKINSOLStatistics)
{
	FILE *fp;
	IntegratorInstance_createKINSolverStructures(ii);
	IntegratorInstance_nullSolver(ii);
	OPEN_TMPFILE_OR_ABORT(fp);
	IntegratorInstance_printKINSOLStatistics(ii, fp);
	/* TODO: check the printed text */
	fclose(fp);
}
END_TEST

/* public */
Suite *create_suite_nullSolver(void)
{
	Suite *s;
	TCase *tc_IntegratorInstance_createKINSolverStructures;
	TCase *tc_IntegratorInstance_freeKINSolverStructures;
	TCase *tc_IntegratorInstance_nullSolver;
	TCase *tc_IntegratorInstance_printKINSOLStatistics;

	s = suite_create("nullSolver");

	tc_IntegratorInstance_createKINSolverStructures = tcase_create("IntegratorInstance_createKINSolverStructures");
	tcase_add_checked_fixture(tc_IntegratorInstance_createKINSolverStructures,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_createKINSolverStructures, test_IntegratorInstance_createKINSolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_createKINSolverStructures);

	tc_IntegratorInstance_freeKINSolverStructures = tcase_create("IntegratorInstance_freeKINSolverStructures");
	tcase_add_test(tc_IntegratorInstance_freeKINSolverStructures, test_IntegratorInstance_freeKINSolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_freeKINSolverStructures);

	tc_IntegratorInstance_nullSolver = tcase_create("IntegratorInstance_nullSolver");
	tcase_add_checked_fixture(tc_IntegratorInstance_nullSolver,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_nullSolver, test_IntegratorInstance_nullSolver);
	suite_add_tcase(s, tc_IntegratorInstance_nullSolver);

	tc_IntegratorInstance_printKINSOLStatistics = tcase_create("IntegratorInstance_printKINSOLStatistics");
	tcase_add_checked_fixture(tc_IntegratorInstance_printKINSOLStatistics,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_printKINSOLStatistics, test_IntegratorInstance_printKINSOLStatistics);
	suite_add_tcase(s, tc_IntegratorInstance_printKINSOLStatistics);

	return s;
}
