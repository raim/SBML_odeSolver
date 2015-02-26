/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/modelSimplify.h>
#include <sbmlsolver/sbml.h>

/* fixtures */
static SBMLDocument_t *doc = NULL;
static char *formula = NULL;
static ASTNode_t *node;

static void setup_node(void)
{
	node = SBML_parseFormula("1 + (x * x)");
	ck_assert(node != NULL);
}

static void teardown_doc(void)
{
	SBMLDocument_free(doc);
}

static void teardown_formula(void)
{
	free(formula);
}

static void teardown_node(void)
{
	ASTNode_free(node);
}

static void setup_node_with_function_definition(void)
{
	node = SBML_parseFormula("f(1, f(x, y))");
	ck_assert(node != NULL);
}

/* test cases */
START_TEST(test_AST_replaceNameByFormula)
{
	ASTNode_t *n;
	ASTNode_t *c, *gc, *ggc;
	int i;

	n = SBML_parseFormula("y/z");
	ck_assert(n != NULL);

	AST_replaceNameByFormula(node, "x", n);
	ck_assert(ASTNode_getType(node) == AST_PLUS);
	ck_assert(ASTNode_getNumChildren(node) == 2);
	c = ASTNode_getChild(node, 0);
	ck_assert(ASTNode_getType(c) == AST_INTEGER);
	ck_assert(ASTNode_getInteger(c) == 1);
	c = ASTNode_getChild(node, 1);
	ck_assert(ASTNode_getType(c) == AST_TIMES);
	ck_assert(ASTNode_getNumChildren(c) == 2);
	for (i = 0; i < 2; i++) {
		gc = ASTNode_getChild(c, i);
		ck_assert(ASTNode_getType(gc) == AST_DIVIDE);
		ck_assert(ASTNode_getNumChildren(gc) == 2);
		ggc = ASTNode_getChild(gc, 0);
		ck_assert(ASTNode_getType(ggc) == AST_NAME);
		ck_assert_str_eq(ASTNode_getName(ggc), "y");
		ggc = ASTNode_getChild(gc, 1);
		ck_assert(ASTNode_getType(ggc) == AST_NAME);
		ck_assert_str_eq(ASTNode_getName(ggc), "z");
	}

	ASTNode_free(n);
}
END_TEST

START_TEST(test_AST_replaceNameByName)
{
	ASTNode_t *c, *gc;
	int i;

	AST_replaceNameByName(node, "x", "y");
	ck_assert(ASTNode_getType(node) == AST_PLUS);
	ck_assert(ASTNode_getNumChildren(node) == 2);
	c = ASTNode_getChild(node, 0);
	ck_assert(ASTNode_getType(c) == AST_INTEGER);
	ck_assert(ASTNode_getInteger(c) == 1);
	c = ASTNode_getChild(node, 1);
	ck_assert(ASTNode_getType(c) == AST_TIMES);
	ck_assert(ASTNode_getNumChildren(c) == 2);
	for (i = 0; i < 2; i++) {
		gc = ASTNode_getChild(c, i);
		ck_assert(ASTNode_getType(gc) == AST_NAME);
		ck_assert_str_eq(ASTNode_getName(gc), "y");
	}
}
END_TEST

START_TEST(test_AST_replaceNameByValue)
{
	ASTNode_t *c, *gc;
	int i;

	AST_replaceNameByValue(node, "x", 42.0);
	ck_assert(ASTNode_getType(node) == AST_PLUS);
	ck_assert(ASTNode_getNumChildren(node) == 2);
	c = ASTNode_getChild(node, 0);
	ck_assert(ASTNode_getType(c) == AST_INTEGER);
	ck_assert(ASTNode_getInteger(c) == 1);
	c = ASTNode_getChild(node, 1);
	ck_assert(ASTNode_getType(c) == AST_TIMES);
	ck_assert(ASTNode_getNumChildren(c) == 2);
	for (i = 0; i < 2; i++) {
		gc = ASTNode_getChild(c, i);
		ck_assert(ASTNode_getType(gc) == AST_REAL);
		ck_assert(ASTNode_getReal(gc) == 42.0);
	}
}
END_TEST

START_TEST(test_AST_replaceNameByParameters)
{
	Parameter_t *p;
	ListOf_t *lo;
	ASTNode_t *c, *gc;
	int i;

#if LIBSBML_VERSION >= 40000
	p = Parameter_create(2, 4);
	Parameter_setId(p, "x");
	Parameter_setName(p, "name");
#else
	p = Parameter_createWith("x", "name");
#endif
	Parameter_setValue(p, 2.5);
#if LIBSBML_VERSION >= 50000
	lo = ListOf_create(2, 4);
#else
	lo = ListOf_create();
#endif
	ListOf_appendAndOwn(lo, (SBase_t *)p);

	AST_replaceNameByParameters(node, lo);
	ck_assert(ASTNode_getType(node) == AST_PLUS);
	ck_assert(ASTNode_getNumChildren(node) == 2);
	c = ASTNode_getChild(node, 0);
	ck_assert(ASTNode_getType(c) == AST_INTEGER);
	ck_assert(ASTNode_getInteger(c) == 1);
	c = ASTNode_getChild(node, 1);
	ck_assert(ASTNode_getType(c) == AST_TIMES);
	ck_assert(ASTNode_getNumChildren(c) == 2);
	for (i = 0; i < 2; i++) {
		gc = ASTNode_getChild(c, i);
		ck_assert(ASTNode_getType(gc) == AST_REAL);
		ck_assert(ASTNode_getReal(gc) == 2.5);
	}

	ListOf_free(lo);
}
END_TEST

START_TEST(test_AST_replaceFunctionDefinition)
{
	ASTNode_t *n;
	ASTNode_t *c, *gc;

	n = SBML_parseFormula("lambda(a, b, a - b)");
	ck_assert(n != NULL);

	AST_replaceFunctionDefinition(node, "f", n);
	ck_assert(ASTNode_getType(node) == AST_MINUS);
	ck_assert(ASTNode_getNumChildren(node) == 2);

	c = ASTNode_getChild(node, 0);
	ck_assert(ASTNode_getType(c) == AST_INTEGER);
	ck_assert(ASTNode_getInteger(c) == 1);

	c = ASTNode_getChild(node, 1);
	ck_assert(ASTNode_getType(c) == AST_MINUS);
	ck_assert(ASTNode_getNumChildren(c) == 2);
	gc = ASTNode_getChild(c, 0);
	ck_assert(ASTNode_getType(gc) == AST_NAME);
	ck_assert_str_eq(ASTNode_getName(gc), "x");
	gc = ASTNode_getChild(c, 1);
	ck_assert(ASTNode_getType(gc) == AST_NAME);
	ck_assert_str_eq(ASTNode_getName(gc), "y");

	ASTNode_free(n);
}
END_TEST

START_TEST(test_AST_replaceConstants_MAPK)
{
	Model_t *m;

	doc = parseModel(EXAMPLES_FILENAME("MAPK.xml"), 0, 1);
	ck_assert(doc != NULL);
	m = SBMLDocument_getModel(doc);
	ck_assert(m != NULL);
	node = SBML_parseFormula("V1 + Ki + K1");
	ck_assert(node != NULL);
	AST_replaceConstants(m, node);
	formula = SBML_formulaToString(node);
	ck_assert(formula != NULL);
	ck_assert_str_eq(formula, "2.5 + 9 + 10");
}
END_TEST

START_TEST(test_AST_replaceConstants_basic_model1_forward_l2)
{
	Model_t *m;

	doc = parseModel(EXAMPLES_FILENAME("basic-model1-forward-l2.xml"), 0, 1);
	ck_assert(doc != NULL);
	m = SBMLDocument_getModel(doc);
	ck_assert(m != NULL);
	node = SBML_parseFormula("c + k_1 + k_2");
	ck_assert(node != NULL);
	AST_replaceConstants(m, node);
	formula = SBML_formulaToString(node);
	ck_assert(formula != NULL);
	ck_assert_str_eq(formula, "1 + k_1 + k_2");
}
END_TEST

START_TEST(test_AST_replaceConstants_basic)
{
	Model_t *m;

	doc = parseModel(EXAMPLES_FILENAME("basic.xml"), 0, 1);
	ck_assert(doc != NULL);
	m = SBMLDocument_getModel(doc);
	ck_assert(m != NULL);
	node = SBML_parseFormula("c + k_1 + k_2");
	ck_assert(node != NULL);
	AST_replaceConstants(m, node);
	formula = SBML_formulaToString(node);
	ck_assert(formula != NULL);
	ck_assert_str_eq(formula, "1 + 1 + k_2");
}
END_TEST

/* public */
Suite *create_suite_modelSimplify(void)
{
	Suite *s;
	TCase *tc_AST_replaceNameByFormula;
	TCase *tc_AST_replaceNameByName;
	TCase *tc_AST_replaceNameByValue;
	TCase *tc_AST_replaceNameByParameters;
	TCase *tc_AST_replaceFunctionDefinition;
	TCase *tc_AST_replaceConstants;

	s = suite_create("modelSimplify");

	tc_AST_replaceNameByFormula = tcase_create("AST_replaceNameByFormula");
	tcase_add_checked_fixture(tc_AST_replaceNameByFormula,
							  setup_node,
							  teardown_node);
	tcase_add_test(tc_AST_replaceNameByFormula, test_AST_replaceNameByFormula);
	suite_add_tcase(s, tc_AST_replaceNameByFormula);

	tc_AST_replaceNameByName = tcase_create("AST_replaceNameByName");
	tcase_add_checked_fixture(tc_AST_replaceNameByName,
							  setup_node,
							  teardown_node);
	tcase_add_test(tc_AST_replaceNameByName, test_AST_replaceNameByName);
	suite_add_tcase(s, tc_AST_replaceNameByName);

	tc_AST_replaceNameByValue = tcase_create("AST_replaceNameByValue");
	tcase_add_checked_fixture(tc_AST_replaceNameByValue,
							  setup_node,
							  teardown_node);
	tcase_add_test(tc_AST_replaceNameByValue, test_AST_replaceNameByValue);
	suite_add_tcase(s, tc_AST_replaceNameByValue);

	tc_AST_replaceNameByParameters = tcase_create("AST_replaceNameByParameters");
	tcase_add_checked_fixture(tc_AST_replaceNameByParameters,
							  setup_node,
							  teardown_node);
	tcase_add_test(tc_AST_replaceNameByParameters, test_AST_replaceNameByParameters);
	suite_add_tcase(s, tc_AST_replaceNameByParameters);

	tc_AST_replaceFunctionDefinition = tcase_create("AST_replaceFunctionDefinition");
	tcase_add_checked_fixture(tc_AST_replaceFunctionDefinition,
							  setup_node_with_function_definition,
							  teardown_node);
	tcase_add_test(tc_AST_replaceFunctionDefinition, test_AST_replaceFunctionDefinition);
	suite_add_tcase(s, tc_AST_replaceFunctionDefinition);

	tc_AST_replaceConstants = tcase_create("AST_replaceConstants");
	tcase_add_checked_fixture(tc_AST_replaceConstants,
							  NULL,
							  teardown_doc);
	tcase_add_checked_fixture(tc_AST_replaceConstants,
							  NULL,
							  teardown_formula);
	tcase_add_checked_fixture(tc_AST_replaceConstants,
							  NULL,
							  teardown_node);
	tcase_add_test(tc_AST_replaceConstants, test_AST_replaceConstants_MAPK);
	tcase_add_test(tc_AST_replaceConstants, test_AST_replaceConstants_basic_model1_forward_l2);
	tcase_add_test(tc_AST_replaceConstants, test_AST_replaceConstants_basic);
	suite_add_tcase(s, tc_AST_replaceConstants);

	return s;
}
