/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/daeSolver.h>

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
START_TEST(test_IntegratorInstance_createIDASolverStructures)
{
	int r;
	r = IntegratorInstance_createIDASolverStructures(ii);
	ck_assert_int_eq(r, 1);
}
END_TEST

START_TEST(test_IntegratorInstance_freeIDASolverStructures)
{
#if 0 /* FIXME */
	IntegratorInstance_createIDASolverStructures(ii);
	/* It is OK to call IntegratorInstance_freeIDASolverStructures consecutively. */
	IntegratorInstance_freeIDASolverStructures(ii);
	IntegratorInstance_freeIDASolverStructures(ii);
#endif
}
END_TEST

START_TEST(test_IntegratorInstance_idaOneStep)
{
#if 0 /* FIXME */
	int r;
	IntegratorInstance_createIDASolverStructures(ii);
	r = IntegratorInstance_idaOneStep(ii);
	ck_assert_int_eq(r, 1);
#endif
}
END_TEST

START_TEST(test_IntegratorInstance_printIDAStatistics)
{
#if 0 /* FIXME */
	FILE *fp;
	IntegratorInstance_createIDASolverStructures(ii);
	IntegratorInstance_idaOneStep(ii);
	OPEN_TMPFILE_OR_ABORT(fp);
	IntegratorInstance_printIDAStatistics(ii, fp);
	/* TODO: check the printed text */
	fclose(fp);
#endif
}
END_TEST

/* public */
Suite *create_suite_daeSolver(void)
{
	Suite *s;
	TCase *tc_IntegratorInstance_createIDASolverStructures;
	TCase *tc_IntegratorInstance_freeIDASolverStructures;
	TCase *tc_IntegratorInstance_idaOneStep;
	TCase *tc_IntegratorInstance_printIDAStatistics;

	s = suite_create("daeSolver");

	tc_IntegratorInstance_createIDASolverStructures = tcase_create("IntegratorInstance_createIDASolverStructures");
	tcase_add_checked_fixture(tc_IntegratorInstance_createIDASolverStructures,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_createIDASolverStructures, test_IntegratorInstance_createIDASolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_createIDASolverStructures);

	tc_IntegratorInstance_freeIDASolverStructures = tcase_create("IntegratorInstance_freeIDASolverStructures");
	tcase_add_test(tc_IntegratorInstance_freeIDASolverStructures, test_IntegratorInstance_freeIDASolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_freeIDASolverStructures);

	tc_IntegratorInstance_idaOneStep = tcase_create("IntegratorInstance_idaOneStep");
	tcase_add_checked_fixture(tc_IntegratorInstance_idaOneStep,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_idaOneStep, test_IntegratorInstance_idaOneStep);
	suite_add_tcase(s, tc_IntegratorInstance_idaOneStep);

	tc_IntegratorInstance_printIDAStatistics = tcase_create("IntegratorInstance_printIDAStatistics");
	tcase_add_checked_fixture(tc_IntegratorInstance_printIDAStatistics,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_printIDAStatistics, test_IntegratorInstance_printIDAStatistics);
	suite_add_tcase(s, tc_IntegratorInstance_printIDAStatistics);

	return s;
}
