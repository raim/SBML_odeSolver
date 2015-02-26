/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/sbml.h>

/* fixtures */
static SBMLDocument_t *doc;
static Model_t *model;

static void teardown_doc(void)
{
	SBMLDocument_free(doc);
}

/* test cases */
START_TEST(test_parseModel_MAPK)
{
	doc = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 8);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 3);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 0);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 10);
	ck_assert(Model_getNumEvents(model) == 0);
	CHECK_PARAMETER(model, 0, "V1");
	CHECK_PARAMETER(model, 1, "Ki");
	CHECK_PARAMETER(model, 2, "K1");
	CHECK_REACTION(model, 0, "J0", "V1*MKKK/((1+(MAPK_PP/Ki)^n)*(K1+MKKK))");
	CHECK_REACTION(model, 1, "J1", "V2*MKKK_P/(KK2+MKKK_P)");
	CHECK_REACTION(model, 2, "J2", "k3*MKKK_P*MKK/(KK3+MKK)");
	CHECK_REACTION(model, 3, "J3", "k4*MKKK_P*MKK_P/(KK4+MKK_P)");
	CHECK_REACTION(model, 4, "J4", "V5*MKK_PP/(KK5+MKK_PP)");
	CHECK_REACTION(model, 5, "J5", "V6*MKK_P/(KK6+MKK_P)");
	CHECK_REACTION(model, 6, "J6", "k7*MKK_PP*MAPK/(KK7+MAPK)");
	CHECK_REACTION(model, 7, "J7", "k8*MKK_PP*MAPK_P/(KK8+MAPK_P)");
	CHECK_REACTION(model, 8, "J8", "V9*MAPK_PP/(KK9+MAPK_PP)");
	CHECK_REACTION(model, 9, "J9", "V10*MAPK_P/(KK10+MAPK_P)");
}
END_TEST

START_TEST(test_parseModel_basic_model1_forward_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 0);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 0);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 2);
	ck_assert(Model_getNumEvents(model) == 0);
	CHECK_REACTION(model, 0, "R1", "k_1 * S1");
	CHECK_REACTION(model, 1, "R2", "k_2 * S2");
}
END_TEST

START_TEST(test_parseModel_basic)
{
	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 1);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 0);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 2);
	ck_assert(Model_getNumEvents(model) == 0);
	CHECK_PARAMETER(model, 0, "k_1");
	CHECK_REACTION(model, 0, "R1", "k_1 * S1");
	CHECK_REACTION(model, 1, "R2", "k_2 * S2");
}
END_TEST

START_TEST(test_parseModel_events_1_event_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 0);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 0);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 1);
	ck_assert(Model_getNumEvents(model) == 1);
	CHECK_REACTION(model, 0, "R", "S1");
	/* TODO */
}
END_TEST

START_TEST(test_parseModel_events_2_events_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 0);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 0);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 1);
	ck_assert(Model_getNumEvents(model) == 2);
	CHECK_REACTION(model, 0, "R", "S1");
	/* TODO */
}
END_TEST

START_TEST(test_parseModel_huang96)
{
	doc = parseModel(EXAMPLES_FILENAME("huang96.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 22);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 0);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 0);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 20);
	ck_assert(Model_getNumEvents(model) == 0);
	CHECK_REACTION(model, 0, "r1a", "a1 * E1 * KKK - d1 * E1_KKK");
	CHECK_REACTION(model, 1, "r1b", "k2 * E1_KKK");
	CHECK_REACTION(model, 2, "r2a", "a2 * E2 * P_KKK - d2 * E2_P_KKK");
	CHECK_REACTION(model, 3, "r2b", "k2 * E2_P_KKK");
	CHECK_REACTION(model, 4, "r3a", "a3 * KK * P_KKK - d3 * P_KKK_KK");
	CHECK_REACTION(model, 5, "r3b", "k3 * P_KKK_KK");
	CHECK_REACTION(model, 6, "r4a", "a4 * P_KK * KKPase - d4 * KKPase_P_KK");
	CHECK_REACTION(model, 7, "r4b", "k4 * KKPase_P_KK");
	CHECK_REACTION(model, 8, "r5a", "a5 * P_KK * P_KKK - d5 * P_KKK_P_KK");
	CHECK_REACTION(model, 9, "r5b", "k5 * P_KKK_P_KK");
	CHECK_REACTION(model, 10, "r6a", "a6 * PP_KK * KKPase - d6 * KKPase_PP_KK");
	CHECK_REACTION(model, 11, "r6b", "k6 * KKPase_PP_KK");
	CHECK_REACTION(model, 12, "r7a", "a7 * K * PP_KK - d7 * PP_KK_K");
	CHECK_REACTION(model, 13, "r7b", "k7 * PP_KK_K");
	CHECK_REACTION(model, 14, "r8a", "a8 * P_K * KPase - d8 * KPase_P_K");
	CHECK_REACTION(model, 15, "r8b", "k8 * KPase_P_K");
	CHECK_REACTION(model, 16, "r9a", "a9 * P_K * PP_KK - d9 * PP_KK_P_K");
	CHECK_REACTION(model, 17, "r9b", "k9 * PP_KK_P_K");
	CHECK_REACTION(model, 18, "r10a", "a10 * PP_K * KPase - d10 * KPase_PP_K");
	CHECK_REACTION(model, 19, "r10b", "k10 * KPase_PP_K");
	/* TODO */
}
END_TEST

START_TEST(test_parseModel_repressilator)
{
	doc = parseModel(EXAMPLES_FILENAME("repressilator.xml"), 0, 1);
	ck_assert(doc != NULL);
	model = SBMLDocument_getModel(doc);
	ck_assert(model != NULL);
	ck_assert(Model_getNumFunctionDefinitions(model) == 0);
	ck_assert(Model_getNumUnitDefinitions(model) == 0);
	ck_assert(Model_getNumCompartmentTypes(model) == 0);
	ck_assert(Model_getNumSpeciesTypes(model) == 0);
	ck_assert(Model_getNumCompartments(model) == 1);
	ck_assert(Model_getNumSpecies(model) == 6);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(model) == 0);
	ck_assert(Model_getNumParameters(model) == 3);
	ck_assert(Model_getNumInitialAssignments(model) == 0);
	ck_assert(Model_getNumRules(model) == 6);
	ck_assert(Model_getNumConstraints(model) == 0);
	ck_assert(Model_getNumReactions(model) == 0);
	ck_assert(Model_getNumEvents(model) == 0);
	CHECK_PARAMETER(model, 0, "alpha");
	CHECK_PARAMETER(model, 1, "beta");
	CHECK_PARAMETER(model, 2, "rho");
	CHECK_RULE(model, 0, "beta * (y1 - x1)");
	CHECK_RULE(model, 1, "beta * (y2 - x2)");
	CHECK_RULE(model, 2, "beta * (y3 - x3)");
	CHECK_RULE(model, 3, "alpha * x1 / (1 + x1 + rho * x3) - y1");
	CHECK_RULE(model, 4, "alpha * x2 / (1 + x2 + rho * x1) - y2");
	CHECK_RULE(model, 5, "alpha * x3 / (1 + x3 + rho * x2) - y3");
}
END_TEST

/* public */
Suite *create_suite_sbml(void)
{
	Suite *s;
	TCase *tc_parseModel;

	s = suite_create("sbml");

	tc_parseModel = tcase_create("parseModel");
	tcase_add_checked_fixture(tc_parseModel,
							  NULL,
							  teardown_doc);
	tcase_add_test(tc_parseModel, test_parseModel_MAPK);
	tcase_add_test(tc_parseModel, test_parseModel_basic_model1_forward_l2);
	tcase_add_test(tc_parseModel, test_parseModel_basic);
	tcase_add_test(tc_parseModel, test_parseModel_events_1_event_1_assignment_l2);
	tcase_add_test(tc_parseModel, test_parseModel_events_2_events_1_assignment_l2);
	tcase_add_test(tc_parseModel, test_parseModel_huang96);
	tcase_add_test(tc_parseModel, test_parseModel_repressilator);
	suite_add_tcase(s, tc_parseModel);

	return s;
}
