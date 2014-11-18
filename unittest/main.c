/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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
	srunner_add_suite(sr, create_suite_sbml());
	srunner_add_suite(sr, create_suite_sbmlResults());
	srunner_add_suite(sr, create_suite_solverError());
	srunner_add_suite(sr, create_suite_util());

	srunner_run_all(sr, CK_ENV);
	n = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (n == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
