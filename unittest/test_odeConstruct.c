/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

START_TEST(test_Model_reduceToOdes)
{
	Model_t *omodel;

	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);

	omodel = Model_reduceToOdes(model);
	ck_assert(omodel != NULL);
	ck_assert(omodel != model);
	/* TODO */
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
	tcase_add_test(tc_Model_reduceToOdes, test_Model_reduceToOdes);
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
