/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/cvodeSolver.h>

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
	IntegratorInstance_free(ii);
	CvodeSettings_free(cs);
	ODEModel_free(model);
}

/* test cases */
START_TEST(test_IntegratorInstance_createCVODESolverStructures)
{
	int r;
	r = IntegratorInstance_createCVODESolverStructures(ii);
	ck_assert_int_eq(r, 1);
}
END_TEST

START_TEST(test_IntegratorInstance_freeCVODESolverStructures)
{
	IntegratorInstance_createCVODESolverStructures(ii);
	/* It is OK to call IntegratorInstance_freeCVODESolverStructures() consecutively. */
	IntegratorInstance_freeCVODESolverStructures(ii);
	IntegratorInstance_freeCVODESolverStructures(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_freeForwardSensitivity)
{
	IntegratorInstance_createCVODESolverStructures(ii);
	/* It is OK to call IntegratorInstance_freeForwardSensitivity() consecutively. */
	IntegratorInstance_freeForwardSensitivity(ii);
	IntegratorInstance_freeForwardSensitivity(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_freeAdjointSensitivity)
{
	IntegratorInstance_createCVODESolverStructures(ii);
	/* It is OK to call IntegratorInstance_freeAdjointSensitivity() consecutively. */
	IntegratorInstance_freeAdjointSensitivity(ii);
	IntegratorInstance_freeAdjointSensitivity(ii);
}
END_TEST

START_TEST(test_IntegratorInstance_cvodeOneStep)
{
	int r;
	r = IntegratorInstance_cvodeOneStep(ii);
	ck_assert_int_eq(r, 1);
}
END_TEST

START_TEST(test_IntegratorInstance_printCVODEStatistics)
{
	FILE *fp;
	int r;
	IntegratorInstance_cvodeOneStep(ii);
	OPEN_TMPFILE_OR_ABORT(fp);
	r = IntegratorInstance_printCVODEStatistics(ii, fp);
	ck_assert_int_eq(r, 1);
	/* TODO: check the printed text */
	fclose(fp);
}
END_TEST

/* public */
Suite *create_suite_cvodeSolver(void)
{
	Suite *s;
	TCase *tc_IntegratorInstance_createCVODESolverStructures;
	TCase *tc_IntegratorInstance_freeCVODESolverStructures;
	TCase *tc_IntegratorInstance_freeForwardSensitivity;
	TCase *tc_IntegratorInstance_freeAdjointSensitivity;
	TCase *tc_IntegratorInstance_cvodeOneStep;
	TCase *tc_IntegratorInstance_printCVODEStatistics;

	s = suite_create("cvodeSolver");

	tc_IntegratorInstance_createCVODESolverStructures = tcase_create("IntegratorInstance_createCVODESolverStructures");
	tcase_add_checked_fixture(tc_IntegratorInstance_createCVODESolverStructures,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_createCVODESolverStructures, test_IntegratorInstance_createCVODESolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_createCVODESolverStructures);

	tc_IntegratorInstance_freeCVODESolverStructures = tcase_create("IntegratorInstance_freeCVODESolverStructures");
	tcase_add_checked_fixture(tc_IntegratorInstance_freeCVODESolverStructures,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_freeCVODESolverStructures, test_IntegratorInstance_freeCVODESolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_freeCVODESolverStructures);

	tc_IntegratorInstance_freeForwardSensitivity = tcase_create("IntegratorInstance_freeForwardSensitivity");
	tcase_add_checked_fixture(tc_IntegratorInstance_freeForwardSensitivity,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_freeForwardSensitivity, test_IntegratorInstance_freeForwardSensitivity);
	suite_add_tcase(s, tc_IntegratorInstance_freeForwardSensitivity);

	tc_IntegratorInstance_freeAdjointSensitivity = tcase_create("IntegratorInstance_freeAdjointSensitivity");
	tcase_add_checked_fixture(tc_IntegratorInstance_freeAdjointSensitivity,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_freeAdjointSensitivity, test_IntegratorInstance_freeAdjointSensitivity);
	suite_add_tcase(s, tc_IntegratorInstance_freeAdjointSensitivity);

	tc_IntegratorInstance_cvodeOneStep = tcase_create("IntegratorInstance_cvodeOneStep");
	tcase_add_checked_fixture(tc_IntegratorInstance_cvodeOneStep,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_cvodeOneStep, test_IntegratorInstance_cvodeOneStep);
	suite_add_tcase(s, tc_IntegratorInstance_cvodeOneStep);

	tc_IntegratorInstance_printCVODEStatistics = tcase_create("IntegratorInstance_printCVODEStatistics");
	tcase_add_checked_fixture(tc_IntegratorInstance_printCVODEStatistics,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_printCVODEStatistics, test_IntegratorInstance_printCVODEStatistics);
	suite_add_tcase(s, tc_IntegratorInstance_printCVODEStatistics);

	return s;
}
