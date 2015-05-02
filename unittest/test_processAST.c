/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/processAST.h>
#include <sbmlsolver/cvodeData.h>
#include <sbmlsolver/odeModel.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* fixtures */
static odeModel_t *model;
static cvodeData_t *data;

static void setup_data(void)
{
  model = ODEModel_createFromFile(EXAMPLES_FILENAME("MAPK.xml"));
  data = CvodeData_create(model);
}

static void teardown_data(void)
{
  CvodeData_free(data);
  ODEModel_free(model);
}

/* helpers */
#define CHECK_EVAL(input, expected) do {        \
    ASTNode_t *node;                            \
    double v;                                   \
    node = SBML_parseFormula(input);            \
    ck_assert(node != NULL);                    \
    v = evaluateAST(node, data);                \
    CHECK_DOUBLE_WITH_TOLERANCE(v, (expected)); \
    ASTNode_free(node);                         \
  } while (0)

#define CHECK_GEN(input, expected) do {                       \
    ASTNode_t *node;                                          \
    charBuffer_t *buf;                                        \
    node = SBML_parseFormula(input);                          \
    ck_assert(node != NULL);                                  \
    buf = CharBuffer_create();                                \
    generateAST(buf, node);                                   \
    ck_assert_str_eq(CharBuffer_getBuffer(buf), (expected));  \
    CharBuffer_free(buf);                                     \
    ASTNode_free(node);                                       \
  } while (0)

#define CHECK_DIFF(input, expected) do {        \
    ASTNode_t *node, *diff;                     \
    char *actual;                               \
    node = SBML_parseFormula(input);            \
    ck_assert(node != NULL);                    \
    diff = differentiateAST(node, "x");         \
    actual = SBML_formulaToString(diff);        \
    ck_assert_str_eq(actual, (expected));       \
    free(actual);                               \
    ASTNode_free(diff);                         \
    ASTNode_free(node);                         \
  } while (0)

#define CHECK_COPY(expr) do {                   \
    ASTNode_t *node, *copied;                   \
    char *actual;                               \
    node = SBML_parseFormula(expr);             \
    ck_assert(node != NULL);                    \
    copied = copyAST(node);                     \
    ck_assert(copied != NULL);                  \
    actual = SBML_formulaToString(copied);      \
    ck_assert_str_eq(actual, (expr));           \
    free(actual);                               \
    ASTNode_free(copied);                       \
    ASTNode_free(node);                         \
  } while (0)

#define CHECK_SIMPLIFY(input, expected) do {    \
    ASTNode_t *node, *simplified;               \
    char *actual;                               \
    node = SBML_parseFormula(input);            \
    ck_assert(node != NULL);                    \
    simplified = simplifyAST(node);             \
    ck_assert(simplified != NULL);              \
    actual = SBML_formulaToString(simplified);  \
    ck_assert_str_eq(actual, (expected));       \
    free(actual);                               \
    ASTNode_free(simplified);                   \
    ASTNode_free(node);                         \
  } while (0)

#define LINE0 "<?xml version='1.0' encoding='UTF-8'?>\n"
#define MATH_OPEN "<math xmlns='http://www.w3.org/1998/Math/MathML'>\n"
#define MATH_CLOSE "\n</math>"

#define CONTAINS_TIME(input, expected) do {                         \
    ASTNode_t *node;                                                \
    node = readMathMLFromString(LINE0 MATH_OPEN input MATH_CLOSE);  \
    ck_assert(node != NULL);                                        \
    ck_assert_int_eq(ASTNode_containsTime(node), expected);         \
    ASTNode_free(node);                                             \
  } while (0)

#define CONTAINS_PIECEWISE(input, expected) do {                    \
    ASTNode_t *node;                                                \
    node = readMathMLFromString(LINE0 MATH_OPEN input MATH_CLOSE);  \
    ck_assert(node != NULL);                                        \
    ck_assert_int_eq(ASTNode_containsPiecewise(node), expected);    \
    ASTNode_free(node);                                             \
  } while (0)

static ASTNode_t *prepare_node(void)
{
  ASTNode_t *node;
  node = ASTNode_create();
  ASTNode_setCharacter(node, '+');
  ASTNode_addChild(node, ASTNode_create());
  ASTNode_addChild(node, ASTNode_create());
  ASTNode_setCharacter(ASTNode_getLeftChild(node), '*');
  ASTNode_addChild(ASTNode_getLeftChild(node), ASTNode_createIndexName());
  ASTNode_addChild(ASTNode_getLeftChild(node), ASTNode_create());
  ASTNode_setName(ASTNode_getLeftChild(ASTNode_getLeftChild(node)), "x");
  ASTNode_setIndex(ASTNode_getLeftChild(ASTNode_getLeftChild(node)), 100);
  ASTNode_setName(ASTNode_getRightChild(ASTNode_getLeftChild(node)), "y");
  ASTNode_setCharacter(ASTNode_getRightChild(node), '/');
  ASTNode_addChild(ASTNode_getRightChild(node), ASTNode_create());
  ASTNode_addChild(ASTNode_getRightChild(node), ASTNode_createIndexName());
  ASTNode_setName(ASTNode_getLeftChild(ASTNode_getRightChild(node)), "z");
  ASTNode_setName(ASTNode_getRightChild(ASTNode_getRightChild(node)), "a");
  ASTNode_setIndex(ASTNode_getRightChild(ASTNode_getRightChild(node)), 101);
  return node;
}

/* test cases */
START_TEST(test_evaluateAST)
{
  CHECK_EVAL("1 * -2 * -3", 6);
  CHECK_EVAL("1 * 0 * -3", 0);
  CHECK_EVAL("abs(1)", 1);
  CHECK_EVAL("abs(-2)", 2);
  CHECK_EVAL("arccosh(1)", 0);
  CHECK_EVAL("arccosh(2)", log(2+sqrt(3)));
  CHECK_EVAL("arccoth(2)", log(3)/2);
  CHECK_EVAL("arccoth(-2)", log(1./3)/2);
  CHECK_EVAL("arccsc(1)", M_PI/2);
  CHECK_EVAL("arccsc(-1)", -M_PI/2);
  CHECK_EVAL("arccsch(1)", log(1+sqrt(2)));
  CHECK_EVAL("arccsch(-1)", log(-1+sqrt(2)));
  CHECK_EVAL("arcsec(1)", 0);
  CHECK_EVAL("arcsec(-1)", M_PI);
  CHECK_EVAL("coth(1)", cosh(1)/sinh(1));
  CHECK_EVAL("coth(-1)", cosh(-1)/sinh(-1));
  CHECK_EVAL("factorial(0)", 1);
  CHECK_EVAL("factorial(10)", 3628800);
  CHECK_EVAL("eq(1)", 1);
  CHECK_EVAL("eq(1, 2)", 0);
  CHECK_EVAL("eq(1, 1)", 1);
  CHECK_EVAL("eq(1, 0, 1)", 0);
  CHECK_EVAL("eq(1, 1, 0)", 0);
  CHECK_EVAL("eq(1, 1, 1, 1)", 1);
  CHECK_EVAL("geq(1)", 1);
  CHECK_EVAL("geq(1, 2)", 0);
  CHECK_EVAL("geq(1, 1)", 1);
  CHECK_EVAL("geq(1, 0, 1)", 0);
  CHECK_EVAL("geq(1, 1, 0)", 1);
  CHECK_EVAL("geq(2, 1, 0, 1)", 0);
  CHECK_EVAL("gt(1)", 1);
  CHECK_EVAL("gt(1, 2)", 0);
  CHECK_EVAL("gt(1, 1)", 0);
  CHECK_EVAL("gt(1, 0, -1)", 1);
  CHECK_EVAL("gt(2, 1, 0)", 1);
  CHECK_EVAL("gt(2, 1, 0, 0)", 0);
  CHECK_EVAL("leq(1)", 1);
  CHECK_EVAL("leq(1, 2)", 1);
  CHECK_EVAL("leq(1, 1)", 1);
  CHECK_EVAL("leq(1, 0, 1)", 0);
  CHECK_EVAL("leq(0, 1, 2)", 1);
  CHECK_EVAL("leq(0, 1, 2, -1)", 0);
  CHECK_EVAL("lt(1)", 1);
  CHECK_EVAL("lt(1, 2)", 1);
  CHECK_EVAL("lt(1, 1)", 0);
  CHECK_EVAL("lt(1, 0, 1)", 0);
  CHECK_EVAL("lt(0, 1, 2)", 1);
  CHECK_EVAL("lt(0, 1, 2, -1)", 0);
  CHECK_EVAL("neq(0, -1)", 1);
  CHECK_EVAL("neq(1, 1)", 0);
}
END_TEST

START_TEST(test_generateAST)
{
  CHECK_GEN("1 + (x * x)", "((realtype)1) + (0.0 * 0.0)");
  CHECK_GEN("-2.5 - time", "((realtype)-2.5) - 0.0");
  CHECK_GEN("1 / 2^3", "((realtype)1) / (pow(((realtype)2), ((realtype)3)))");
  CHECK_GEN("exponentiale", "2.71828");
  CHECK_GEN("false", "0");
  CHECK_GEN("pi", "3.14159");
  CHECK_GEN("true", "1");
  CHECK_GEN("abs(1.0)", "fabs(((realtype)1))");
  CHECK_GEN("acos(1.0)", "acos(((realtype)1))");
  CHECK_GEN("arccosh(1.0)", "acosh(((realtype)1))");
  CHECK_GEN("arccot(1.0)", "acot(((realtype)1))");
  CHECK_GEN("arccoth(1.0)", "acoth(((realtype)1))");
  CHECK_GEN("arccsc(1.0)", "acsc(((realtype)1))");
  CHECK_GEN("arccsch(1.0)", "acsch(((realtype)1))");
  CHECK_GEN("arcsec(1.0)", "asec(((realtype)1))");
  CHECK_GEN("arcsech(1.0)", "asech(((realtype)1))");
  CHECK_GEN("asin(1.0)", "asin(((realtype)1))");
  CHECK_GEN("arcsinh(1.0)", "asinh(((realtype)1))");
  CHECK_GEN("atan(1.0)", "atan(((realtype)1))");
  CHECK_GEN("arctanh(1.0)", "atanh(((realtype)1))");
  CHECK_GEN("ceil(1.0)", "ceil(((realtype)1))");
  CHECK_GEN("cos(1.0)", "cos(((realtype)1))");
  CHECK_GEN("cosh(1.0)", "cosh(((realtype)1))");
  CHECK_GEN("cot(1.0)", "cot(((realtype)1))");
  CHECK_GEN("coth(1.0)", "coth(((realtype)1))");
  CHECK_GEN("csc(1.0)", "csc(((realtype)1))");
  CHECK_GEN("csch(1.0)", "csch(((realtype)1))");
  CHECK_GEN("exp(1.0)", "exp(((realtype)1))");
  CHECK_GEN("factorial(1.0)", "factorial(((realtype)1))");
  CHECK_GEN("floor(1.0)", "floor(((realtype)1))");
  CHECK_GEN("ln(1.0)", "log(((realtype)1))");
  CHECK_GEN("log(1.0)", "log(((realtype)1))"); /* TODO: expected? */
  CHECK_GEN("power(1)", "pow(((realtype)1))");
  CHECK_GEN("root(1)", "root(((realtype)1))");
  CHECK_GEN("sec(1)", "sec(((realtype)1))");
  CHECK_GEN("sech(1)", "sech(((realtype)1))");
  CHECK_GEN("sin(1)", "sin(((realtype)1))");
  CHECK_GEN("sinh(1)", "sinh(((realtype)1))");
  CHECK_GEN("tan(1)", "tan(((realtype)1))");
  CHECK_GEN("tanh(1)", "tanh(((realtype)1))");
  CHECK_GEN("and(true, false)", "1 && 0");
  CHECK_GEN("and(true, false, true)", "1 && 0 && 1");
  CHECK_GEN("not(true)", "!1");
  CHECK_GEN("or(true, false)", "1 || 0");
  CHECK_GEN("or(true, false, true)", "1 || 0 || 1");
  CHECK_GEN("xor(0, 1)", "(((((realtype)0) ? 1 : 0) + (((realtype)1) ? 1 : 0)) % 2) != 0");
  CHECK_GEN("xor(0, 1, 11)", "(((((realtype)0) ? 1 : 0) + (((realtype)1) ? 1 : 0) + (((realtype)11) ? 1 : 0)) % 2) != 0");
  CHECK_GEN("eq(0, 1)", "((realtype)0) == ((realtype)1)");
  CHECK_GEN("geq(0, 1)", "((realtype)0) >= ((realtype)1)");
  CHECK_GEN("gt(0, 1)", "((realtype)0) > ((realtype)1)");
  CHECK_GEN("leq(0, 1)", "((realtype)0) <= ((realtype)1)");
  CHECK_GEN("lt(0, 1)", "((realtype)0) < ((realtype)1)");
  CHECK_GEN("neq(0, 1)", "((realtype)0) != ((realtype)1)");
}
END_TEST

START_TEST(test_differentiateAST)
{
  CHECK_DIFF("1", "0");
  CHECK_DIFF("x", "1");
  CHECK_DIFF("x^2", "2 * x^(2 - 1)");
  CHECK_DIFF("x^3", "3 * x^(3 - 1)");
  CHECK_DIFF("x + x^2", "1 + 2 * x^(2 - 1)");
  CHECK_DIFF("x - x^2", "1 - 2 * x^(2 - 1)");
  CHECK_DIFF("- x", "-1");
  CHECK_DIFF("- x", "-1");
  CHECK_DIFF("x * x", "x + x");
  CHECK_DIFF("2 * x", "2");
  CHECK_DIFF("x * 2", "2");
  CHECK_DIFF("x / 2", "1 / 2");
  CHECK_DIFF("2 / x", "-(2 / x^2)");
  CHECK_DIFF("x^2 / x", "2 * x^(2 - 1) / x - x^2 / x^2");
  CHECK_DIFF("x^x", "x^x * (x / x + log(x))");
  CHECK_DIFF("abs(x)", "piecewise(-1, lt(x, 0), 0, eq(x, 0), 1, gt(x, 0))");
  CHECK_DIFF("arccos(x)", "-(1 / sqrt(1 - x^2))");
  CHECK_DIFF("arccosh(x)", "1 / sqrt(x^2 - 1)");
  CHECK_DIFF("arccot(x)", "-(1 / (1 + x^2))");
  CHECK_DIFF("arccoth(x)", "-(1 / (x^2 - 1))");
  CHECK_DIFF("arccsc(x)", "-(1 / (abs(x) * sqrt(x^2 - 1)))");
  CHECK_DIFF("arccsch(x)", "-(1 / (x^2 * sqrt(1 + 1 / x^2)))");
  CHECK_DIFF("arcsec(x)", "-(1 / (x^2 * sqrt(1 - 1 / x^2)))");
  CHECK_DIFF("arcsech(x)", "sqrt((1 - x) / (1 + x)) / (x * (x - 1))");
  CHECK_DIFF("arcsin(x)", "1 / sqrt(1 - x^2)");
  CHECK_DIFF("arcsinh(x)", "1 / sqrt(1 + x^2)");
  CHECK_DIFF("arctan(x)", "1 / (1 + x^2)");
  CHECK_DIFF("arctanh(x)", "1 / (1 - x^2)");
  CHECK_DIFF("ceil(x)", "ceil(1)"); /* TODO */
  CHECK_DIFF("cos(x)", "-sin(x)");
  CHECK_DIFF("cosh(x)", "sinh(x)");
  CHECK_DIFF("cot(x)", "-(1 / sin(x)^2)");
  CHECK_DIFF("coth(x)", "-(1 / sinh(x)^2)");
  CHECK_DIFF("csc(x)", "-(1 * csc(x) * cot(x))");
  CHECK_DIFF("csch(x)", "-(1 * csch(x) * coth(x))");
  CHECK_DIFF("exp(x)", "exp(x)");
  CHECK_DIFF("floor(x)", "floor(1)"); /* TODO */
  CHECK_DIFF("ln(x)", "1 / x");
  CHECK_DIFF("log(3, x)", "1 / log(3) * (1 / x)");
  CHECK_DIFF("log(x, 3)", "0"); /* TODO */
  CHECK_DIFF("root(x, 3)", "1 / 3 * x^(1 / 3 - 1)");
  CHECK_DIFF("root(3, x)", "-(pow(3, 1 / x) * log(3) * (1 / x^2))");
  CHECK_DIFF("sec(x)", "sec(x) * tan(x)");
  CHECK_DIFF("sech(x)", "-(1 * sech(x) * tanh(x))");
  CHECK_DIFF("sin(x)", "cos(x)");
  CHECK_DIFF("sinh(x)", "cosh(x)");
  CHECK_DIFF("tan(x)", "1 / cos(x)^2");
  CHECK_DIFF("tanh(x)", "1 / cosh(x)^2");
}
END_TEST

START_TEST(test_copyAST)
{
  CHECK_COPY("0");
  CHECK_COPY("1.5");
  CHECK_COPY("foo");
  CHECK_COPY("(x + 1) * exp(y) / (pi * log(y - exponentiale))");
  CHECK_COPY("lambda(x, y, x + y)");
}
END_TEST

START_TEST(test_indexAST)
{
  static const char input[] = "x + tan(y) / (2 * z)";
  static const char *names[] = {"a", "b", "x", "y", "z"};
  ASTNode_t *node, *indexed;
  char *s;
  node = SBML_parseFormula(input);
  indexed = indexAST(node, 5, (char **)names);
  s = SBML_formulaToString(indexed);
  ck_assert_str_eq(s, input);
  free(s);
  ck_assert_int_eq(ASTNode_isIndexName(ASTNode_getLeftChild(indexed)), 1);
  ck_assert(ASTNode_getIndex(ASTNode_getLeftChild(indexed)) == 2);
  ck_assert_int_eq(ASTNode_isIndexName(ASTNode_getLeftChild(ASTNode_getLeftChild(ASTNode_getRightChild(indexed)))), 1);
  ck_assert(ASTNode_getIndex(ASTNode_getLeftChild(ASTNode_getLeftChild(ASTNode_getRightChild(indexed)))) == 3);
  ck_assert_int_eq(ASTNode_isIndexName(ASTNode_getRightChild(ASTNode_getRightChild(ASTNode_getRightChild(indexed)))), 1);
  ck_assert(ASTNode_getIndex(ASTNode_getRightChild(ASTNode_getRightChild(ASTNode_getRightChild(indexed)))) == 4);
  ASTNode_free(indexed);
  ASTNode_free(node);
}
END_TEST

START_TEST(test_simplifyAST)
{
  CHECK_SIMPLIFY("0", "0");
  CHECK_SIMPLIFY("1.5", "1.5");
  CHECK_SIMPLIFY("foo", "foo");
  CHECK_SIMPLIFY("x + 0", "x");
  CHECK_SIMPLIFY("0 + x", "x");
  CHECK_SIMPLIFY("x - 0", "x");
  CHECK_SIMPLIFY("0 - x", "-x");
  CHECK_SIMPLIFY("x * 0", "0");
  CHECK_SIMPLIFY("0 * x", "0");
  CHECK_SIMPLIFY("x * 1", "x");
  CHECK_SIMPLIFY("1 * x", "x");
  CHECK_SIMPLIFY("0 / x", "0");
  CHECK_SIMPLIFY("x / 1", "x");
  CHECK_SIMPLIFY("x^0", "1");
  CHECK_SIMPLIFY("x^1", "x");
  CHECK_SIMPLIFY("0^x", "0");
  CHECK_SIMPLIFY("1^x", "1");
  CHECK_SIMPLIFY("--x", "x");
  CHECK_SIMPLIFY("-x + -y", "-(x + y)");
  CHECK_SIMPLIFY("-x + y", "y - x");
  CHECK_SIMPLIFY("x + -y", "x - y");
  CHECK_SIMPLIFY("-x - -y", "y - x");
  CHECK_SIMPLIFY("-x - y", "-(x + y)");
  CHECK_SIMPLIFY("x - -y", "x + y");
  CHECK_SIMPLIFY("-x * -y", "x * y");
  CHECK_SIMPLIFY("-x * y", "-(x * y)");
  CHECK_SIMPLIFY("x * -y", "-(x * y)");
  CHECK_SIMPLIFY("-x / -y", "x / y");
  CHECK_SIMPLIFY("-x / y", "-(x / y)");
  CHECK_SIMPLIFY("x / -y", "-(x / y)");
  CHECK_SIMPLIFY("log(0 + pi) / -(1 * x)", "-(log(pi) / x)");
}
END_TEST

START_TEST(test_ASTNode_containsTime)
{
  CONTAINS_TIME("<cn>0</cn>", 0);
  CONTAINS_TIME("<ci>x</ci>", 0);
  CONTAINS_TIME("<ci>time</ci>", 0);
  CONTAINS_TIME("<csymbol encoding='text' definitionURL='http://www.sbml.org/sbml/symbols/time'>t</csymbol>", 1);
  CONTAINS_TIME("<apply><plus/><cn>4</cn><ci>t</ci></apply>", 0);
  CONTAINS_TIME("<apply><plus/><csymbol encoding='text' definitionURL='http://www.sbml.org/sbml/symbols/time'>t</csymbol><cn>4</cn></apply>", 1);
}
END_TEST

START_TEST(test_ASTNode_containsPiecewise)
{
  CONTAINS_PIECEWISE("<cn>0</cn>", 0);
  CONTAINS_PIECEWISE("<ci>x</ci>", 0);
  CONTAINS_PIECEWISE("<piecewise><otherwise><ci>foo</ci></otherwise></piecewise>", 1);
  CONTAINS_PIECEWISE("<apply>"
             " <times/>"
             " <ci>piece</ci>"
             " <ci>piecewise</ci>"
             "</apply>",
             0);
  CONTAINS_PIECEWISE("<apply>"
             " <times/>"
             " <ci>x</ci>"
             " <piecewise>"
             "  <piece>"
             "   <ci>y</ci>"
             "   <apply><gt/><ci>a</ci><cn>1</cn></apply>"
                       "  </piece>"
             "  <otherwise>"
                       "   <ci>z</ci>"
             "  </otherwise>"
             " </piecewise>"
             "</apply>",
             1);
}
END_TEST

START_TEST(test_ASTNode_getIndices)
{
  ASTNode_t *node;
  List_t *indices;
  int r;
  void *p;

  node = prepare_node();

  indices = List_create();
  r = ASTNode_getIndices(node, indices);
  ck_assert_int_eq(r, 1);
  ck_assert(List_size(indices) == 2);
  p = List_get(indices, 0);
  ck_assert_int_eq(*((unsigned int *)p), 100);
  free(p);
  p = List_get(indices, 1);
  ck_assert_int_eq(*((unsigned int *)p), 101);
  free(p);
  List_free(indices);

  ASTNode_free(node);
}
END_TEST

START_TEST(test_ASTNode_getIndexArray)
{
  ASTNode_t *node;
  int *arr;
  int i;
  node = prepare_node();
  arr = ASTNode_getIndexArray(node, 102);
  ck_assert(arr != NULL);
  for (i=0; i<100; i++) {
    ck_assert_int_eq(arr[i], 0);
  }
  ck_assert_int_eq(arr[100], 1);
  ck_assert_int_eq(arr[101], 1);
  free(arr);
  ASTNode_free(node);
}
END_TEST

START_TEST(test_ASTNode_getIndexArray__null)
{
  int *arr;
  int i;
  arr = ASTNode_getIndexArray(NULL, 7);
  ck_assert(arr != NULL);
  for (i=0; i<7; i++) {
    ck_assert_int_eq(arr[i], 0);
  }
  free(arr);
}
END_TEST

/* public */
Suite *create_suite_processAST(void)
{
  Suite *s;
  TCase *tc_evaluateAST;
  TCase *tc_generateAST;
  TCase *tc_differentiateAST;
  TCase *tc_copyAST;
  TCase *tc_simplifyAST;
  TCase *tc_indexAST;
  TCase *tc_ASTNode_containsTime;
  TCase *tc_ASTNode_containsPiecewise;
  TCase *tc_ASTNode_getIndices;
  TCase *tc_ASTNode_getIndexArray;

  s = suite_create("processAST");

  tc_evaluateAST = tcase_create("evaluateAST");
  tcase_add_checked_fixture(tc_evaluateAST,
                setup_data,
                teardown_data);
  tcase_add_test(tc_evaluateAST, test_evaluateAST);
  suite_add_tcase(s, tc_evaluateAST);

  tc_generateAST = tcase_create("generateAST");
  tcase_add_test(tc_generateAST, test_generateAST);
  suite_add_tcase(s, tc_generateAST);

  tc_differentiateAST = tcase_create("differentiateAST");
  tcase_add_test(tc_differentiateAST, test_differentiateAST);
  suite_add_tcase(s, tc_differentiateAST);

  tc_copyAST = tcase_create("copyAST");
  tcase_add_test(tc_copyAST, test_copyAST);
  suite_add_tcase(s, tc_copyAST);

  tc_simplifyAST = tcase_create("simplifyAST");
  tcase_add_test(tc_simplifyAST, test_simplifyAST);
  suite_add_tcase(s, tc_simplifyAST);

  tc_indexAST = tcase_create("indexAST");
  tcase_add_test(tc_indexAST, test_indexAST);
  suite_add_tcase(s, tc_indexAST);

  tc_ASTNode_containsTime = tcase_create("ASTNode_containsTime");
  tcase_add_test(tc_ASTNode_containsTime, test_ASTNode_containsTime);
  suite_add_tcase(s, tc_ASTNode_containsTime);

  tc_ASTNode_containsPiecewise = tcase_create("ASTNode_containsPiecewise");
  tcase_add_test(tc_ASTNode_containsPiecewise, test_ASTNode_containsPiecewise);
  suite_add_tcase(s, tc_ASTNode_containsPiecewise);

  tc_ASTNode_getIndices = tcase_create("ASTNode_getIndices");
  tcase_add_test(tc_ASTNode_getIndices, test_ASTNode_getIndices);
  suite_add_tcase(s, tc_ASTNode_getIndices);

  tc_ASTNode_getIndexArray = tcase_create("ASTNode_getIndexArray");
  tcase_add_test(tc_ASTNode_getIndexArray, test_ASTNode_getIndexArray);
  tcase_add_test(tc_ASTNode_getIndexArray, test_ASTNode_getIndexArray__null);
  suite_add_tcase(s, tc_ASTNode_getIndexArray);

  return s;
}
