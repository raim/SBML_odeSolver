/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/cvodeSolver.h>
#include <sbmlsolver/sensSolver.h>

/* fixtures */
static odeModel_t *model = NULL;

static cvodeSettings_t *cs = NULL;

static integratorInstance_t *ii = NULL;

static const char *params[] = {
	"MAPK",
	"MAPK_P",
	"K1",
	"Ki"
};

static const int PARAMS_SIZE = (int)sizeof(params)/sizeof(params[0]);

static void setup_integratorInstance(void)
{
	model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
	cs = CvodeSettings_create();
	/* enable sensitivity analysis */
	CvodeSettings_setSensitivity(cs, 1);
	/* 0: simultaneous, 1: staggered, 2: staggered1 */
	CvodeSettings_setSensMethod(cs, 0);
	CvodeSettings_setSensParams(cs, (char **)params, PARAMS_SIZE);
	ii = IntegratorInstance_create(model, cs);
}

static void teardown_integratorInstance(void)
{
	IntegratorInstance_free(ii);
	CvodeSettings_free(cs);
	ODEModel_free(model);
}

/* test cases */
START_TEST(test_IntegratorInstance_createCVODESSolverStructures)
{
#if 0 /* FIXME */
	int r;
	r = IntegratorInstance_createCVODESSolverStructures(ii);
	ck_assert_int_eq(r, 1);
#endif
}
END_TEST

START_TEST(test_IntegratorInstance_CVODEQuad)
{
	int r;
    while (!IntegratorInstance_timeCourseCompleted(ii)) {
		if (!IntegratorInstance_cvodeOneStep(ii)) {
			ck_abort_msg("failed to advance one step");
		}
	}
    r = IntegratorInstance_CVODEQuad(ii);
	ck_assert_int_eq(r, 1);
}
END_TEST

START_TEST(test_IntegratorInstance_printQuad)
{
	FILE *fp;
	int r;
    while (!IntegratorInstance_timeCourseCompleted(ii)) {
		if (!IntegratorInstance_cvodeOneStep(ii)) {
			ck_abort_msg("failed to advance one step");
		}
	}
    IntegratorInstance_CVODEQuad(ii);
	OPEN_TMPFILE_OR_ABORT(fp);
	r = IntegratorInstance_printQuad(ii, fp);
	ck_assert_int_eq(r, 1);
	/* TODO: check the printed text */
	fclose(fp);
}
END_TEST

START_TEST(test_IntegratorInstance_printCVODESStatistics)
{
	FILE *fp;
	int r;
	IntegratorInstance_cvodeOneStep(ii);
	OPEN_TMPFILE_OR_ABORT(fp);
	r = IntegratorInstance_printCVODESStatistics(ii, fp);
	ck_assert_int_eq(r, 1);
	/* TODO: check the printed text */
	fclose(fp);
}
END_TEST

/* public */
Suite *create_suite_sensSolver(void)
{
	Suite *s;
	TCase *tc_IntegratorInstance_createCVODESSolverStructures;
	TCase *tc_IntegratorInstance_CVODEQuad;
	TCase *tc_IntegratorInstance_printQuad;
	TCase *tc_IntegratorInstance_printCVODESStatistics;

	s = suite_create("sensSolver");

	tc_IntegratorInstance_createCVODESSolverStructures = tcase_create("IntegratorInstance_createCVODESSolverStructures");
	tcase_add_checked_fixture(tc_IntegratorInstance_createCVODESSolverStructures,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_createCVODESSolverStructures, test_IntegratorInstance_createCVODESSolverStructures);
	suite_add_tcase(s, tc_IntegratorInstance_createCVODESSolverStructures);

	tc_IntegratorInstance_CVODEQuad = tcase_create("IntegratorInstance_CVODEQuad");
	tcase_add_checked_fixture(tc_IntegratorInstance_CVODEQuad,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_CVODEQuad, test_IntegratorInstance_CVODEQuad);
	suite_add_tcase(s, tc_IntegratorInstance_CVODEQuad);

	tc_IntegratorInstance_printQuad = tcase_create("IntegratorInstance_printQuad");
	tcase_add_checked_fixture(tc_IntegratorInstance_printQuad,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_printQuad, test_IntegratorInstance_printQuad);
	suite_add_tcase(s, tc_IntegratorInstance_printQuad);

	tc_IntegratorInstance_printCVODESStatistics = tcase_create("IntegratorInstance_printCVODESStatistics");
	tcase_add_checked_fixture(tc_IntegratorInstance_printCVODESStatistics,
							  setup_integratorInstance,
							  teardown_integratorInstance);
	tcase_add_test(tc_IntegratorInstance_printCVODESStatistics, test_IntegratorInstance_printCVODESStatistics);
	suite_add_tcase(s, tc_IntegratorInstance_printCVODESStatistics);

	return s;
}
