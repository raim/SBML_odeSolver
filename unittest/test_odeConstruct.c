/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/odeConstruct.h>
#include <sbmlsolver/sbml.h>

/* fixtures */
static SBMLDocument_t *doc = NULL;
static Model_t *model;

static void teardown_doc(void)
{
	SBMLDocument_free(doc);
}

/* helpers */
static void check_species(int i, const char *expected)
{
	Species_t *s;
	ASTNode_t *n;
	char *ns;

	s = Model_getSpecies(model, i);
	n = Species_odeFromReactions(s, model);
	ck_assert(n != NULL);
	ns = SBML_formulaToString(n);
	ck_assert(ns != NULL);
	ck_assert_str_eq(ns, expected);
	free(ns);
	ASTNode_free(n);
}

/* test cases */
START_TEST(test_Model_getValueById)
{
	double v;

	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	v = Model_getValueById(model, "no_such_id");
	ck_assert(v == 0);
	/* compartments */
	v = Model_getValueById(model, "c");
	ck_assert(v == 1);
	/* species */
	v = Model_getValueById(model, "S1");
	ck_assert(v == 1.5e-15);
	v = Model_getValueById(model, "S2");
	ck_assert(v == 1.5e-15);
	/* parameters */
	v = Model_getValueById(model, "k_1");
	ck_assert(v == 1);
	v = Model_getValueById(model, "k_2");
	ck_assert(v == 0);
	/* reactions */
	v = Model_getValueById(model, "R1");
	ck_assert(v == 0);
	v = Model_getValueById(model, "R2");
	ck_assert(v == 0);
}
END_TEST

START_TEST(test_Model_setValue)
{
	int r;

	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	r = Model_setValue(model, "no_such_id", NULL, 7.77);
	ck_assert(r == 0);
	/* compartments */
	r = Model_setValue(model, "c", NULL, 7.77);
	ck_assert(r == 1);
	/* species */
	r = Model_setValue(model, "S1", NULL, 7.77);
	ck_assert(r == 1);
	r = Model_setValue(model, "S2", NULL, 7.77);
	ck_assert(r == 1);
	/* parameters */
	r = Model_setValue(model, "k_1", NULL, 7.77);
	ck_assert(r == 1);
	r = Model_setValue(model, "k_2", NULL, 7.77);
	ck_assert(r == 0);
	r = Model_setValue(model, "k_2", "R1", 7.77);
	ck_assert(r == 0);
	r = Model_setValue(model, "k_2", "R2", 7.77);
	ck_assert(r == 1);
}
END_TEST

START_TEST(test_Model_reduceToOdes_MAPK)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 8);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 13);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 18);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 0);
	CHECK_PARAMETER(omodel, 0, "V1");
	CHECK_PARAMETER(omodel, 1, "Ki");
	CHECK_PARAMETER(omodel, 2, "K1");
	CHECK_PARAMETER(omodel, 3, "J0");
	CHECK_PARAMETER(omodel, 4, "J1");
	CHECK_PARAMETER(omodel, 5, "J2");
	CHECK_PARAMETER(omodel, 6, "J3");
	CHECK_PARAMETER(omodel, 7, "J4");
	CHECK_PARAMETER(omodel, 8, "J5");
	CHECK_PARAMETER(omodel, 9, "J6");
	CHECK_PARAMETER(omodel, 10, "J7");
	CHECK_PARAMETER(omodel, 11, "J8");
	CHECK_PARAMETER(omodel, 12, "J9");
	CHECK_RULE(omodel, 0, "(J1 - J0) / uVol");
	CHECK_RULE(omodel, 1, "(J0 - J1) / uVol");
	CHECK_RULE(omodel, 2, "(J5 - J2) / uVol");
	CHECK_RULE(omodel, 3, "(J2 - J3 + J4 - J5) / uVol");
	CHECK_RULE(omodel, 4, "(J3 - J4) / uVol");
	CHECK_RULE(omodel, 5, "(J9 - J6) / uVol");
	CHECK_RULE(omodel, 6, "(J6 - J7 + J8 - J9) / uVol");
	CHECK_RULE(omodel, 7, "(J7 - J8) / uVol");
	CHECK_RULE(omodel, 8, "V1 * MKKK / ((1 + (MAPK_PP / Ki)^1) * (K1 + MKKK))");
	CHECK_RULE(omodel, 9, "0.25 * MKKK_P / (8 + MKKK_P)");
	CHECK_RULE(omodel, 10, "0.025 * MKKK_P * MKK / (15 + MKK)");
	CHECK_RULE(omodel, 11, "0.025 * MKKK_P * MKK_P / (15 + MKK_P)");
	CHECK_RULE(omodel, 12, "0.75 * MKK_PP / (15 + MKK_PP)");
	CHECK_RULE(omodel, 13, "0.75 * MKK_P / (15 + MKK_P)");
	CHECK_RULE(omodel, 14, "0.025 * MKK_PP * MAPK / (15 + MAPK)");
	CHECK_RULE(omodel, 15, "0.025 * MKK_PP * MAPK_P / (15 + MAPK_P)");
	CHECK_RULE(omodel, 16, "0.5 * MAPK_PP / (15 + MAPK_PP)");
	CHECK_RULE(omodel, 17, "0.5 * MAPK_P / (15 + MAPK_P)");

	Model_free(omodel);
}
END_TEST

START_TEST(test_Model_reduceToOdes_basic_model1_forward_l2)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 2);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 4);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 0);
	CHECK_PARAMETER(omodel, 0, "R1");
	CHECK_PARAMETER(omodel, 1, "R2");
	CHECK_RULE(omodel, 0, "(R2 - R1) / c");
	CHECK_RULE(omodel, 1, "(R1 - R2) / c");
	CHECK_RULE(omodel, 2, "1 * S1");
	CHECK_RULE(omodel, 3, "0 * S2");

	Model_free(omodel);
}
END_TEST

START_TEST(test_Model_reduceToOdes_basic)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 3);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 4);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 0);
	CHECK_PARAMETER(omodel, 0, "k_1");
	CHECK_PARAMETER(omodel, 1, "R1");
	CHECK_PARAMETER(omodel, 2, "R2");
	CHECK_RULE(omodel, 0, "(R2 - R1) / c");
	CHECK_RULE(omodel, 1, "(R1 - R2) / c");
	CHECK_RULE(omodel, 2, "k_1 * S1");
	CHECK_RULE(omodel, 3, "0 * S2");

	Model_free(omodel);
}
END_TEST

START_TEST(test_Model_reduceToOdes_events_1_event_1_assignment_l2)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 1);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 3);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 1);
	CHECK_PARAMETER(omodel, 0, "R");
	CHECK_RULE(omodel, 0, "-(R / compartment)");
	CHECK_RULE(omodel, 1, "R / compartment");
	CHECK_RULE(omodel, 2, "S1");
	/* TODO */

	Model_free(omodel);
}
END_TEST

START_TEST(test_Model_reduceToOdes_events_2_events_1_assignment_l2)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 2);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 1);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 3);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 2);
	CHECK_PARAMETER(omodel, 0, "R");
	CHECK_RULE(omodel, 0, "-(R / compartment)");
	CHECK_RULE(omodel, 1, "R / compartment");
	CHECK_RULE(omodel, 2, "S1");
	/* TODO */

	Model_free(omodel);
}
END_TEST

START_TEST(test_Model_reduceToOdes_huang96)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("huang96.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 22);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 20);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 42);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 0);
	CHECK_PARAMETER(omodel, 0, "r1a");
	CHECK_PARAMETER(omodel, 1, "r1b");
	CHECK_PARAMETER(omodel, 2, "r2a");
	CHECK_PARAMETER(omodel, 3, "r2b");
	CHECK_PARAMETER(omodel, 4, "r3a");
	CHECK_PARAMETER(omodel, 5, "r3b");
	CHECK_PARAMETER(omodel, 6, "r4a");
	CHECK_PARAMETER(omodel, 7, "r4b");
	CHECK_PARAMETER(omodel, 8, "r5a");
	CHECK_PARAMETER(omodel, 9, "r5b");
	CHECK_PARAMETER(omodel, 10, "r6a");
	CHECK_PARAMETER(omodel, 11, "r6b");
	CHECK_PARAMETER(omodel, 12, "r7a");
	CHECK_PARAMETER(omodel, 13, "r7b");
	CHECK_PARAMETER(omodel, 14, "r8a");
	CHECK_PARAMETER(omodel, 15, "r8b");
	CHECK_PARAMETER(omodel, 16, "r9a");
	CHECK_PARAMETER(omodel, 17, "r9b");
	CHECK_PARAMETER(omodel, 18, "r10a");
	CHECK_PARAMETER(omodel, 19, "r10b");
	CHECK_RULE(omodel, 0, "(r1b - r1a) / compartment");
	CHECK_RULE(omodel, 1, "(r2b - r2a) / compartment");
	CHECK_RULE(omodel, 2, "(r2b - r1a) / compartment");
	CHECK_RULE(omodel, 3, "(r1b - r2a - r3a + r3b - r5a + r5b) / compartment");
	CHECK_RULE(omodel, 4, "(r4b - r3a) / compartment");
	CHECK_RULE(omodel, 5, "(r3b - r4a - r5a + r6b) / compartment");
	CHECK_RULE(omodel, 6, "(r5b - r6a - r7a + r7b - r9a + r9b) / compartment");
	CHECK_RULE(omodel, 7, "(r8b - r7a) / compartment");
	CHECK_RULE(omodel, 8, "(r7b - r8a - r9a + r10b) / compartment");
	CHECK_RULE(omodel, 9, "(r9b - r10a) / compartment");
	CHECK_RULE(omodel, 10, "(r8b - r8a - r10a + r10b) / compartment");
	CHECK_RULE(omodel, 11, "(r4b - r4a - r6a + r6b) / compartment");
	CHECK_RULE(omodel, 12, "(r1a - r1b) / compartment");
	CHECK_RULE(omodel, 13, "(r2a - r2b) / compartment");
	CHECK_RULE(omodel, 14, "(r3a - r3b) / compartment");
	CHECK_RULE(omodel, 15, "(r5a - r5b) / compartment");
	CHECK_RULE(omodel, 16, "(r7a - r7b) / compartment");
	CHECK_RULE(omodel, 17, "(r9a - r9b) / compartment");
	CHECK_RULE(omodel, 18, "(r6a - r6b) / compartment");
	CHECK_RULE(omodel, 19, "(r4a - r4b) / compartment");
	CHECK_RULE(omodel, 20, "(r10a - r10b) / compartment");
	CHECK_RULE(omodel, 21, "(r8a - r8b) / compartment");
	CHECK_RULE(omodel, 22, "1000 * E1 * KKK - 150 * E1_KKK");
	CHECK_RULE(omodel, 23, "150 * E1_KKK");
	CHECK_RULE(omodel, 24, "1000 * E2 * P_KKK - 150 * E2_P_KKK");
	CHECK_RULE(omodel, 25, "150 * E2_P_KKK");
	CHECK_RULE(omodel, 26, "1000 * KK * P_KKK - 150 * P_KKK_KK");
	CHECK_RULE(omodel, 27, "150 * P_KKK_KK");
	CHECK_RULE(omodel, 28, "1000 * P_KK * KKPase - 150 * KKPase_P_KK");
	CHECK_RULE(omodel, 29, "150 * KKPase_P_KK");
	CHECK_RULE(omodel, 30, "1000 * P_KK * P_KKK - 150 * P_KKK_P_KK");
	CHECK_RULE(omodel, 31, "150 * P_KKK_P_KK");
	CHECK_RULE(omodel, 32, "1000 * PP_KK * KKPase - 150 * KKPase_PP_KK");
	CHECK_RULE(omodel, 33, "150 * KKPase_PP_KK");
	CHECK_RULE(omodel, 34, "1000 * K * PP_KK - 150 * PP_KK_K");
	CHECK_RULE(omodel, 35, "150 * PP_KK_K");
	CHECK_RULE(omodel, 36, "1000 * P_K * KPase - 150 * KPase_P_K");
	CHECK_RULE(omodel, 37, "150 * KPase_P_K");
	CHECK_RULE(omodel, 38, "1000 * P_K * PP_KK - 150 * PP_KK_P_K");
	CHECK_RULE(omodel, 39, "150 * PP_KK_P_K");
	CHECK_RULE(omodel, 40, "1000 * PP_K * KPase - 150 * KPase_PP_K");
	CHECK_RULE(omodel, 41, "150 * KPase_PP_K");

	Model_free(omodel);
}
END_TEST

START_TEST(test_Model_reduceToOdes_repressilator)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("repressilator.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	omodel = Model_reduceToOdes(model);

	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	ck_assert(Model_getNumFunctionDefinitions(omodel) == 0);
	ck_assert(Model_getNumUnitDefinitions(omodel) == 0);
	ck_assert(Model_getNumCompartmentTypes(omodel) == 0);
	ck_assert(Model_getNumSpeciesTypes(omodel) == 0);
	ck_assert(Model_getNumCompartments(omodel) == 1);
	ck_assert(Model_getNumSpecies(omodel) == 6);
	ck_assert(Model_getNumSpeciesWithBoundaryCondition(omodel) == 0);
	ck_assert(Model_getNumParameters(omodel) == 3);
	ck_assert(Model_getNumInitialAssignments(omodel) == 0);
	ck_assert(Model_getNumRules(omodel) == 6);
	ck_assert(Model_getNumConstraints(omodel) == 0);
	ck_assert(Model_getNumReactions(omodel) == 0);
	ck_assert(Model_getNumEvents(omodel) == 0);
	CHECK_PARAMETER(omodel, 0, "alpha");
	CHECK_PARAMETER(omodel, 1, "beta");
	CHECK_PARAMETER(omodel, 2, "rho");
	CHECK_RULE(omodel, 0, "beta * (y1 - x1)");
	CHECK_RULE(omodel, 1, "beta * (y2 - x2)");
	CHECK_RULE(omodel, 2, "beta * (y3 - x3)");
	CHECK_RULE(omodel, 3, "alpha * x1 / (1 + x1 + rho * x3) - y1");
	CHECK_RULE(omodel, 4, "alpha * x2 / (1 + x2 + rho * x1) - y2");
	CHECK_RULE(omodel, 5, "alpha * x3 / (1 + x3 + rho * x2) - y3");

	Model_free(omodel);
}
END_TEST

START_TEST(test_Species_odeFromReactions_MAPK)
{
	doc = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "(J1 - J0) / uVol"); /* MKKK */
	check_species(1, "(J0 - J1) / uVol"); /* MKKK_P */
	check_species(2, "(J5 - J2) / uVol"); /* MKK */
	check_species(3, "(J2 - J3 + J4 - J5) / uVol"); /* MKK_P */
	check_species(4, "(J3 - J4) / uVol"); /* MKK_PP */
	check_species(5, "(J9 - J6) / uVol"); /* MAPK */
	check_species(6, "(J6 - J7 + J8 - J9) / uVol"); /* MAPK_P */
	check_species(7, "(J7 - J8) / uVol"); /* MAPK_PP */
}
END_TEST

START_TEST(test_Species_odeFromReactions_basic_model1_forward_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "(R2 - R1) / c"); /* S1 */
	check_species(1, "(R1 - R2) / c"); /* S2 */
}
END_TEST

START_TEST(test_Species_odeFromReactions_basic)
{
	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "(R2 - R1) / c"); /* S1 */
	check_species(1, "(R1 - R2) / c"); /* S2 */
}
END_TEST

START_TEST(test_Species_odeFromReactions_events_1_event_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "-(R / compartment)"); /* S1 */
	check_species(1, "R / compartment"); /* S2 */
}
END_TEST

START_TEST(test_Species_odeFromReactions_events_2_events_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "-(R / compartment)"); /* S1 */
	check_species(1, "R / compartment"); /* S2 */
}
END_TEST

START_TEST(test_Species_odeFromReactions_huang96)
{
	doc = parseModel(EXAMPLES_FILENAME("huang96.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "(r1b - r1a) / compartment"); /* E1 */
	check_species(1, "(r2b - r2a) / compartment"); /* E2 */
	check_species(2, "(r2b - r1a) / compartment"); /* KKK */
	check_species(3, "(r1b - r2a - r3a + r3b - r5a + r5b) / compartment"); /* P_KKK */
	check_species(4, "(r4b - r3a) / compartment"); /* KK */
	check_species(5, "(r3b - r4a - r5a + r6b) / compartment"); /* P_KK */
	check_species(6, "(r5b - r6a - r7a + r7b - r9a + r9b) / compartment"); /* PP_KK */
	check_species(7, "(r8b - r7a) / compartment"); /* K */
	check_species(8, "(r7b - r8a - r9a + r10b) / compartment"); /* P_K */
	check_species(9, "(r9b - r10a) / compartment"); /* PP_K */
	check_species(10, "(r8b - r8a - r10a + r10b) / compartment"); /* KPase */
	check_species(11, "(r4b - r4a - r6a + r6b) / compartment"); /* KKPase */
	check_species(12, "(r1a - r1b) / compartment"); /* E1_KKK */
	check_species(13, "(r2a - r2b) / compartment"); /* E2_P_KKK */
	check_species(14, "(r3a - r3b) / compartment"); /* P_KKK_KK */
	check_species(15, "(r5a - r5b) / compartment"); /* P_KKK_P_KK */
	check_species(16, "(r7a - r7b) / compartment"); /* PP_KK_K */
	check_species(17, "(r9a - r9b) / compartment"); /* PP_KK_P_K */
	check_species(18, "(r6a - r6b) / compartment"); /* KKPase_PP_KK */
	check_species(19, "(r4a - r4b) / compartment"); /* KKPase_P_KK */
	check_species(20, "(r10a - r10b) / compartment"); /* KPase_PP_K */
	check_species(21, "(r8a - r8b) / compartment"); /* KPase_P_K */
}
END_TEST

START_TEST(test_Species_odeFromReactions_repressilator)
{
	doc = parseModel(EXAMPLES_FILENAME("repressilator.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	check_species(0, "0"); /* x1 */
	check_species(1, "0"); /* x2 */
	check_species(2, "0"); /* x3 */
	check_species(3, "0"); /* y1 */
	check_species(4, "0"); /* y2 */
	check_species(5, "0"); /* y3 */
}
END_TEST

/* public */
Suite *create_suite_odeConstruct(void)
{
	Suite *s;
	TCase *tc_Model_getValueById;
	TCase *tc_Model_setValue;
	TCase *tc_Model_reduceToOdes;
	TCase *tc_Species_odeFromReactions;

	s = suite_create("odeConstruct");

	tc_Model_getValueById = tcase_create("Model_getValueById");
	tcase_add_checked_fixture(tc_Model_getValueById,
							  NULL,
							  teardown_doc);
	tcase_add_test(tc_Model_getValueById, test_Model_getValueById);
	suite_add_tcase(s, tc_Model_getValueById);

	tc_Model_setValue = tcase_create("Model_setValue");
	tcase_add_checked_fixture(tc_Model_setValue,
							  NULL,
							  teardown_doc);
	tcase_add_test(tc_Model_setValue, test_Model_setValue);
	suite_add_tcase(s, tc_Model_setValue);

	tc_Model_reduceToOdes = tcase_create("Model_reduceToOdes");
	tcase_add_checked_fixture(tc_Model_reduceToOdes,
							  NULL,
							  teardown_doc);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_MAPK);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_basic_model1_forward_l2);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_basic);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_events_1_event_1_assignment_l2);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_events_2_events_1_assignment_l2);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_huang96);
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes_repressilator);
	suite_add_tcase(s, tc_Model_reduceToOdes);

	tc_Species_odeFromReactions = tcase_create("Species_odeFromReactions");
	tcase_add_checked_fixture(tc_Species_odeFromReactions,
							  NULL,
							  teardown_doc);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_MAPK);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_basic_model1_forward_l2);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_basic);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_events_1_event_1_assignment_l2);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_events_2_events_1_assignment_l2);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_huang96);
	tcase_add_test(tc_Species_odeFromReactions, test_Species_odeFromReactions_repressilator);
	suite_add_tcase(s, tc_Species_odeFromReactions);

	return s;
}
