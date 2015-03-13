/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/odeSolver.h>
#include <sbmlsolver/sbml.h>

/* fixtures */
static SBMLDocument_t *doc = NULL;

Model_t *model;

static cvodeSettings_t *cs;

static SBMLResults_t *results = NULL;

static varySettings_t *vs = NULL;

static void teardown_doc(void)
{
	SBMLDocument_free(doc);
}

static void setup_cs(void)
{
	cs = CvodeSettings_create();
}

static void teardown_cs(void)
{
	CvodeSettings_free(cs);
}

static void teardown_results(void)
{
	SBMLResults_free(results);
}

static void setup_vs(void)
{
	vs = VarySettings_allocate(3, 4);
}

static void teardown_vs(void)
{
	VarySettings_free(vs);
}

/* test cases */
START_TEST(test_SBML_odeSolver_MAPK)
{
	doc = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	results = SBML_odeSolver(doc, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_basic_model1_forward_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_basic)
{
	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_events_1_event_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-1-event-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_events_2_events_1_assignment_l2)
{
	doc = parseModel(EXAMPLES_FILENAME("events-2-events-1-assignment-l2.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_huang96)
{
	doc = parseModel(EXAMPLES_FILENAME("huang96.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_Model_odeSolver_repressilator)
{
	doc = parseModel(EXAMPLES_FILENAME("repressilator.xml"), 0, 1);
	model = SBMLDocument_getModel(doc);
	results = Model_odeSolver(model, cs);
	ck_assert(results != NULL);
}
END_TEST

START_TEST(test_VarySettings_allocate)
{
	vs = VarySettings_allocate(7, 77);
	ck_assert(vs != NULL);
	ck_assert_int_eq(vs->nrparams, 7);
	ck_assert_int_eq(vs->nrdesignpoints, 77);
}
END_TEST

START_TEST(test_VarySettings_free)
{
	VarySettings_free(NULL); /* freeing NULL is safe */
}
END_TEST

START_TEST(test_VarySettings_addDesignPoint)
{
	static const double params0[] = {0.0, 2.5, 4.0};
	static const double params1[] = {0.1, -0.1, 4.2};
	static const double params2[] = {0.2, -1.4, 5.9};
	static const double params3[] = {0.3, -2.9, 5.1};
	static const double params4[] = {0.4, -5.0, 4.1};
	int r;
	r = VarySettings_addDesignPoint(vs, params0);
	ck_assert_int_eq(r, 1);
	r = VarySettings_addDesignPoint(vs, params1);
	ck_assert_int_eq(r, 2);
	r = VarySettings_addDesignPoint(vs, params2);
	ck_assert_int_eq(r, 3);
	r = VarySettings_addDesignPoint(vs, params3);
	ck_assert_int_eq(r, 4);
	r = VarySettings_addDesignPoint(vs, params4);
	ck_assert_int_eq(r, 0);
}
END_TEST

START_TEST(test_VarySettings_addParameter)
{
	int r;
	r = VarySettings_addParameter(vs, "foo", NULL); /* global */
	ck_assert_int_eq(r, 1);
	ck_assert_str_eq(vs->id[0], "foo");
	ck_assert(vs->rid[0] == NULL);
	r = VarySettings_addParameter(vs, "foo", "local");
	ck_assert_int_eq(r, 2);
	ck_assert_str_eq(vs->id[1], "foo");
	ck_assert_str_eq(vs->rid[1], "local");
	r = VarySettings_addParameter(vs, "bar", "local");
	ck_assert_int_eq(r, 3);
	ck_assert_str_eq(vs->id[2], "bar");
	ck_assert_str_eq(vs->rid[2], "local");
	r = VarySettings_addParameter(vs, "failure", "full");
	ck_assert_int_eq(r, 0);
}
END_TEST

START_TEST(test_VarySettings_setName)
{
	int r;
	r = VarySettings_setName(vs, 0, "foo", NULL); /* global */
	ck_assert_int_eq(r, 1);
	r = VarySettings_setName(vs, 1, "foo", "local");
	ck_assert_int_eq(r, 1);
	r = VarySettings_setName(vs, 2, "bar", "local");
	ck_assert_int_eq(r, 1);
	r = VarySettings_setName(vs, 3, "failure", "fencepost");
	ck_assert_int_eq(r, 0);
	ck_assert_str_eq(vs->id[0], "foo");
	ck_assert(vs->rid[0] == NULL);
	ck_assert_str_eq(vs->id[1], "foo");
	ck_assert_str_eq(vs->rid[1], "local");
	ck_assert_str_eq(vs->id[2], "bar");
	ck_assert_str_eq(vs->rid[2], "local");
}
END_TEST

START_TEST(test_VarySettings_setValue)
{
	int r;
	r = VarySettings_setValue(vs, 0, 0, 7.7);
	ck_assert_int_eq(r, 1);
	r = VarySettings_setValue(vs, 2, 2, 2.3);
	ck_assert_int_eq(r, 1);
	r = VarySettings_setValue(vs, 4, 1, 0.1);
	ck_assert_int_eq(r, 0);
	r = VarySettings_setValue(vs, 2, 3, 0.2);
	ck_assert_int_eq(r, 0);
	ck_assert(vs->params[0][0] == 7.7);
	ck_assert(vs->params[2][2] == 2.3);
}
END_TEST

START_TEST(test_VarySettings_getValue)
{
	double d;
	VarySettings_setValue(vs, 0, 0, 7.7);
	VarySettings_setValue(vs, 2, 2, 2.3);
	d = VarySettings_getValue(vs, 0, 0);
	ck_assert(d == 7.7);
	d = VarySettings_getValue(vs, 2, 2);
	ck_assert(d == 2.3);
}
END_TEST

START_TEST(test_VarySettings_setValueByID)
{
	int r;
	double d;
	VarySettings_setName(vs, 0, "foo", NULL); /* global */
	VarySettings_setName(vs, 1, "foo", "local");
	VarySettings_setName(vs, 2, "bar", "local");
	r = VarySettings_setValueByID(vs, 0, "foo", NULL, 1.2);
	ck_assert_int_eq(r, 1);
	r = VarySettings_setValueByID(vs, 1, "foo", NULL, 2.3);
	ck_assert_int_eq(r, 1);
	r = VarySettings_setValueByID(vs, 1, "foo", "local", 3.4);
	ck_assert_int_eq(r, 1);
	r = VarySettings_setValueByID(vs, 3, "bar", "local", 4.5);
	ck_assert_int_eq(r, 1);
	d = VarySettings_getValue(vs, 0, 0);
	ck_assert(d == 1.2);
	d = VarySettings_getValue(vs, 1, 0);
	ck_assert(d == 2.3);
	d = VarySettings_getValue(vs, 1, 1);
	ck_assert(d == 3.4);
	d = VarySettings_getValue(vs, 3, 2);
	ck_assert(d == 4.5);
}
END_TEST

START_TEST(test_VarySettings_getValueByID)
{
	double d;
	VarySettings_setName(vs, 0, "foo", NULL); /* global */
	VarySettings_setName(vs, 1, "foo", "local");
	VarySettings_setName(vs, 2, "bar", "local");
	VarySettings_setValueByID(vs, 0, "foo", NULL, 1.2);
	VarySettings_setValueByID(vs, 1, "foo", NULL, 2.3);
	VarySettings_setValueByID(vs, 1, "foo", "local", 3.4);
	VarySettings_setValueByID(vs, 3, "bar", "local", 4.5);
	d = VarySettings_getValueByID(vs, 0, "foo", NULL);
	ck_assert(d == 1.2);
	d = VarySettings_getValueByID(vs, 1, "foo", NULL);
	ck_assert(d == 2.3);
	d = VarySettings_getValueByID(vs, 1, "foo", "local");
	ck_assert(d == 3.4);
	d = VarySettings_getValueByID(vs, 3, "bar", "local");
	ck_assert(d == 4.5);
}
END_TEST

START_TEST(test_VarySettings_getName)
{
	const char *name;
	VarySettings_setName(vs, 0, "foo", NULL); /* global */
	VarySettings_setName(vs, 1, "foo", "local");
	VarySettings_setName(vs, 2, "bar", "local");
	name = VarySettings_getName(vs, 0);
	ck_assert_str_eq(name, "foo");
	name = VarySettings_getName(vs, 1);
	ck_assert_str_eq(name, "foo");
	name = VarySettings_getName(vs, 2);
	ck_assert_str_eq(name, "bar");
	name = VarySettings_getName(vs, 3);
	ck_assert(name == NULL);
}
END_TEST

START_TEST(test_VarySettings_getReactionName)
{
	const char *name;
	VarySettings_setName(vs, 0, "foo", NULL); /* global */
	VarySettings_setName(vs, 1, "foo", "local");
	VarySettings_setName(vs, 2, "bar", "local");
	name = VarySettings_getReactionName(vs, 0);
	ck_assert(name == NULL);
	name = VarySettings_getReactionName(vs, 1);
	ck_assert_str_eq(name, "local");
	name = VarySettings_getReactionName(vs, 2);
	ck_assert_str_eq(name, "local");
	name = VarySettings_getReactionName(vs, 3);
	ck_assert(name == NULL);
}
END_TEST

/* public */
Suite *create_suite_odeSolver(void)
{
	Suite *s;
	TCase *tc_SBML_odeSolver;
	TCase *tc_Model_odeSolver;
	TCase *tc_VarySettings_allocate;
	TCase *tc_VarySettings_free;
	TCase *tc_VarySettings_addDesignPoint;
	TCase *tc_VarySettings_addParameter;
	TCase *tc_VarySettings_setName;
	TCase *tc_VarySettings_setValue;
	TCase *tc_VarySettings_getValue;
	TCase *tc_VarySettings_setValueByID;
	TCase *tc_VarySettings_getValueByID;
	TCase *tc_VarySettings_getName;
	TCase *tc_VarySettings_getReactionName;

	s = suite_create("odeSolver");

	tc_SBML_odeSolver = tcase_create("SBML_odeSolver");
	tcase_add_checked_fixture(tc_SBML_odeSolver,
							  NULL,
							  teardown_doc);
	tcase_add_checked_fixture(tc_SBML_odeSolver,
							  setup_cs,
							  teardown_cs);
	tcase_add_checked_fixture(tc_SBML_odeSolver,
							  NULL,
							  teardown_results);
	tcase_add_test(tc_SBML_odeSolver, test_SBML_odeSolver_MAPK);
	suite_add_tcase(s, tc_SBML_odeSolver);

	tc_Model_odeSolver = tcase_create("Model_odeSolver");
	tcase_add_checked_fixture(tc_Model_odeSolver,
							  NULL,
							  teardown_doc);
	tcase_add_checked_fixture(tc_Model_odeSolver,
							  setup_cs,
							  teardown_cs);
	tcase_add_checked_fixture(tc_Model_odeSolver,
							  NULL,
							  teardown_results);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_basic_model1_forward_l2);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_basic);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_events_1_event_1_assignment_l2);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_events_2_events_1_assignment_l2);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_huang96);
	tcase_add_test(tc_Model_odeSolver, test_Model_odeSolver_repressilator);
	suite_add_tcase(s, tc_Model_odeSolver);

	tc_VarySettings_allocate = tcase_create("VarySettings_allocate");
	tcase_add_checked_fixture(tc_VarySettings_allocate,
							  NULL,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_allocate, test_VarySettings_allocate);
	suite_add_tcase(s, tc_VarySettings_allocate);

	tc_VarySettings_free = tcase_create("VarySettings_free");
	tcase_add_test(tc_VarySettings_free, test_VarySettings_free);
	suite_add_tcase(s, tc_VarySettings_free);

	tc_VarySettings_addDesignPoint = tcase_create("VarySettings_addDesignPoint");
	tcase_add_checked_fixture(tc_VarySettings_addDesignPoint,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_addDesignPoint, test_VarySettings_addDesignPoint);
	suite_add_tcase(s, tc_VarySettings_addDesignPoint);

	tc_VarySettings_addParameter = tcase_create("VarySettings_addParameter");
	tcase_add_checked_fixture(tc_VarySettings_addParameter,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_addParameter, test_VarySettings_addParameter);
	suite_add_tcase(s, tc_VarySettings_addParameter);

	tc_VarySettings_setName = tcase_create("VarySettings_setName");
	tcase_add_checked_fixture(tc_VarySettings_setName,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_setName, test_VarySettings_setName);
	suite_add_tcase(s, tc_VarySettings_setName);

	tc_VarySettings_setValue = tcase_create("VarySettings_setValue");
	tcase_add_checked_fixture(tc_VarySettings_setValue,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_setValue, test_VarySettings_setValue);
	suite_add_tcase(s, tc_VarySettings_setValue);

	tc_VarySettings_getValue = tcase_create("VarySettings_getValue");
	tcase_add_checked_fixture(tc_VarySettings_getValue,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_getValue, test_VarySettings_getValue);
	suite_add_tcase(s, tc_VarySettings_getValue);

	tc_VarySettings_setValueByID = tcase_create("VarySettings_setValueByID");
	tcase_add_checked_fixture(tc_VarySettings_setValueByID,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_setValueByID, test_VarySettings_setValueByID);
	suite_add_tcase(s, tc_VarySettings_setValueByID);

	tc_VarySettings_getValueByID = tcase_create("VarySettings_getValueByID");
	tcase_add_checked_fixture(tc_VarySettings_getValueByID,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_getValueByID, test_VarySettings_getValueByID);
	suite_add_tcase(s, tc_VarySettings_getValueByID);

	tc_VarySettings_getName = tcase_create("VarySettings_getName");
	tcase_add_checked_fixture(tc_VarySettings_getName,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_getName, test_VarySettings_getName);
	suite_add_tcase(s, tc_VarySettings_getName);

	tc_VarySettings_getReactionName = tcase_create("VarySettings_getReactionName");
	tcase_add_checked_fixture(tc_VarySettings_getReactionName,
							  setup_vs,
							  teardown_vs);
	tcase_add_test(tc_VarySettings_getReactionName, test_VarySettings_getReactionName);
	suite_add_tcase(s, tc_VarySettings_getReactionName);

	return s;
}
