/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

static Suite *create_root_suite(void)
{
	return suite_create("root");
}

int main(void)
{
	SRunner *sr;
	int n;

	sr = srunner_create(create_root_suite());
	if (!sr) return EXIT_FAILURE;

	srunner_add_suite(sr, create_suite_ASTIndexNameNode());
	srunner_add_suite(sr, create_suite_charBuffer());
	srunner_add_suite(sr, create_suite_cvodeData());
	srunner_add_suite(sr, create_suite_cvodeSolver());
	srunner_add_suite(sr, create_suite_daeSolver());
	srunner_add_suite(sr, create_suite_integratorInstance());
	srunner_add_suite(sr, create_suite_integratorSettings());
	srunner_add_suite(sr, create_suite_interpol());
	srunner_add_suite(sr, create_suite_modelSimplify());
	srunner_add_suite(sr, create_suite_nullSolver());
	srunner_add_suite(sr, create_suite_odeConstruct());
	srunner_add_suite(sr, create_suite_odeModel());
	srunner_add_suite(sr, create_suite_odeSolver());
	srunner_add_suite(sr, create_suite_processAST());
	srunner_add_suite(sr, create_suite_sbml());
	srunner_add_suite(sr, create_suite_sbmlResults());
	srunner_add_suite(sr, create_suite_sensSolver());
	srunner_add_suite(sr, create_suite_solverError());
	srunner_add_suite(sr, create_suite_util());

	srunner_run_all(sr, CK_ENV);
	n = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (n == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
