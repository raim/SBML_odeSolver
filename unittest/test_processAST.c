/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include "unittest.h"

#include <sbmlsolver/processAST.h>

/* helpers */
#define CHECK_GEN(input, expected) do {								\
		ASTNode_t *node;											\
		charBuffer_t *buf;											\
		node = SBML_parseFormula(input);							\
		ck_assert(node != NULL);									\
		buf = CharBuffer_create();									\
		generateAST(buf, node);										\
		ck_assert_str_eq(CharBuffer_getBuffer(buf), (expected));	\
		CharBuffer_free(buf);										\
		ASTNode_free(node);											\
	} while (0)

/* test cases */
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

/* public */
Suite *create_suite_processAST(void)
{
	Suite *s;
	TCase *tc_generateAST;

	s = suite_create("processAST");

	tc_generateAST = tcase_create("generateAST");
	tcase_add_test(tc_generateAST, test_generateAST);
	suite_add_tcase(s, tc_generateAST);

	return s;
}
