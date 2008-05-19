/* IN THIS FILE EMULATES THE ENVIRONMENT FOR THE CODE GENERATOR AND MUST BE CHANGED IN THE FRAMEWORK */

#define ARITHMETIC_TEST

#include "interfaceSimulation.c"
#include "arithmeticCompiler.c"



char *string;

/* implements tests for all tokens of the abstract syntax tree that are already implemented - except AST_NAME */

void testAST_INTEGER() {
	string = "AST_INTEGER: (5.0)";
	nodes[0].type = AST_INTEGER;
	nodes[0].childnum = 5;
	}

void testAST_REAL() {
	string = "AST_REAL: (5.3)";
	nodes[0].type = AST_REAL;
	nodes[0].value = 5.3;
	}

void testAST_NAME() {
	string = "AST_NAME: (4.25)";
	nodes[0].type = AST_NAME;
	nodes[0].childnum = 5;
	}

void testAST_FUNCTION_DELAY() {
	string = "AST_FUNCTION_DELAY: (0.0)";
	nodes[0].type = AST_FUNCTION_DELAY;
	}
	
void testAST_NAME_TIME() {
	string = "AST_NAME_TIME: (13.4)";
	nodes[0].type = AST_NAME_TIME;
	}
	
void testAST_CONSTANT_E() {
	string = "AST_CONSTANT_E: (2.718)";
	nodes[0].type = AST_CONSTANT_E;
	}
	
void testAST_CONSTANT_FALSE() {
	string = "AST_CONSTANT_FALSE: (0.0)";
	nodes[0].type = AST_CONSTANT_FALSE;
	}

void testAST_CONSTANT_PI() {
	string = "AST_CONSTANT_PI: (3.14)";
	nodes[0].type = AST_CONSTANT_PI;
	}

void testAST_CONSTANT_TRUE() {
	string = "AST_CONSTANT_TRUE: (1.0)";
	nodes[0].type = AST_CONSTANT_TRUE;
	}

void testAST_PLUS() {
	string = "AST_PLUS: (7.5)";
	nodes[0].type = AST_PLUS;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 4.0;
	nodes[2].type = AST_REAL;
	nodes[2].value = 3.5;
	}

void testAST_PLUS_large() {
	string = "AST_PLUS_large: (3.4)";
	nodes[0].type = AST_PLUS;
	nodes[0].childnum = 5;
	nodes[0].child = (int *)malloc(sizeof(int)*5);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[0].child[3] = 4;
	nodes[0].child[4] = 5;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	nodes[2].type = AST_REAL;
	nodes[2].value = 1.1;
	nodes[3].type = AST_REAL;
	nodes[3].value = 2.0;
	nodes[4].type = AST_REAL;
	nodes[4].value = 2.3;
	nodes[5].type = AST_REAL;
	nodes[5].value = -3.0;
	}

void testAST_MINUS() {
	string = "AST_MINUS: (-1.3)";
	nodes[0].type = AST_MINUS;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 2.8;
	}

void testAST_TIMES() {
	string = "AST_TIMES: (-8.75)";
	nodes[0].type = AST_TIMES;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = -3.5;
	}

void testAST_TIMES_large() {
	string = "AST_TIMES_large: (15.18)";
	nodes[0].type = AST_TIMES;
	nodes[0].childnum = 5;
	nodes[0].child = (int *)malloc(sizeof(int)*5);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[0].child[3] = 4;
	nodes[0].child[4] = 5;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	nodes[2].type = AST_REAL;
	nodes[2].value = 1.1;
	nodes[3].type = AST_REAL;
	nodes[3].value = -2.0;
	nodes[4].type = AST_REAL;
	nodes[4].value = 2.3;
	nodes[5].type = AST_REAL;
	nodes[5].value = -3.0;
	}

void testAST_DIVIDE() {
	string = "AST_DIVIDE: (-0.667)";
	nodes[0].type = AST_DIVIDE;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.0;
	nodes[2].type = AST_REAL;
	nodes[2].value = -3.0;
	}

void testAST_POWER() {
	string = "AST_POWER: (9.017)";
	nodes[0].type = AST_POWER;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 2.4;
	}

void testAST_LAMBDA() {
	string = "AST_LAMBDA: (0.0)";
	nodes[0].type = AST_LAMBDA;
	}

static double testFunc(char* name, int parameternum, double* values) {
	int i;
	double result = 0.0;
	printf("%s", name);
	for(i = 0 ; i < parameternum ; i++)
		result += values[i];
	return result;
	}

void testAST_FUNCTION() {
	UsrDefFunc = testFunc;
	string = "AST_FUNCTION: (4.0)";
	nodes[0].type = AST_FUNCTION;
	nodes[0].name = "Test!";
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	nodes[2].type = AST_REAL;
	nodes[2].value = 3.0;
	}

void testAST_FUNCTION_ABS() {
	string = "AST_FUNCTION_ABS: (0.54)";
	nodes[0].type = AST_FUNCTION_ABS;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = -0.54;
	}

void testAST_FUNCTION_ARCCOS() {
	string = "AST_FUNCTION_ARCCOS: (0.723)";
	nodes[0].type = AST_FUNCTION_ARCCOS;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.75;
	}

void testAST_FUNCTION_ARCCOSH() {
	string = "AST_FUNCTION_ARCCOSH: (2.993)";
	nodes[0].type = AST_FUNCTION_ARCCOSH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 10.0;
	}

void testAST_FUNCTION_ARCCOT() {
	string = "AST_FUNCTION_ARCCOT: (0.785)";
	nodes[0].type = AST_FUNCTION_ARCCOT;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	}

void testAST_FUNCTION_ARCCOTH() {
	string = "AST_FUNCTION_ARCCOTH: (0.549)";
	nodes[0].type = AST_FUNCTION_ARCCOTH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.0;
	}

void testAST_FUNCTION_ARCCSC() {
	string = "AST_FUNCTION_ARCCSC: (0.985)";
	nodes[0].type = AST_FUNCTION_ARCCSC;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.2;
	}

void testAST_FUNCTION_ARCCSCH() {
	string = "AST_FUNCTION_ARCCSCH: (0.481)";
	nodes[0].type = AST_FUNCTION_ARCCSCH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.0;
	}

void testAST_FUNCTION_ARCSEC() {
	string = "AST_FUNCTION_ARCSEC: (1.427)";
	nodes[0].type = AST_FUNCTION_ARCSEC;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 7.0;
	}

void testAST_FUNCTION_ARCSECH() {
	string = "AST_FUNCTION_ARCSECH: (2.993)";
	nodes[0].type = AST_FUNCTION_ARCSECH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.1;
	}

void testAST_FUNCTION_ARCSIN() {
	string = "AST_FUNCTION_ARCSIN: (0.848)";
	nodes[0].type = AST_FUNCTION_ARCSIN;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.75;
	}

void testAST_FUNCTION_ARCSINH() {
	string = "AST_FUNCTION_ARCSINH: (0.481)";
	nodes[0].type = AST_FUNCTION_ARCSINH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.5;
	}

void testAST_FUNCTION_ARCTAN() {
	string = "AST_FUNCTION_ARCTAN: (0.464)";
	nodes[0].type = AST_FUNCTION_ARCTAN;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.5;
	}

void testAST_FUNCTION_ARCTANH() {
	string = "AST_FUNCTION_ARCTANH: (0.549)";
	nodes[0].type = AST_FUNCTION_ARCTANH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.5;
	}

void testAST_FUNCTION_CEILING() {
	string = "AST_FUNCTION_CEILING: (13.0)";
	nodes[0].type = AST_FUNCTION_CEILING;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 12.345;
	}

void testAST_FUNCTION_COS() {
	string = "AST_FUNCTION_COS: (0.54)";
	nodes[0].type = AST_FUNCTION_COS;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	}

void testAST_FUNCTION_COSH() {
	string = "AST_FUNCTION_COSH: (1.543)";
	nodes[0].type = AST_FUNCTION_COSH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	}

void testAST_FUNCTION_COT() {
	string = "AST_FUNCTION_COT: (1.542)";
	nodes[0].type = AST_FUNCTION_COT;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 10.0;
	}

void testAST_FUNCTION_COTH() {
	string = "AST_FUNCTION_COTH: (2.164)";
	nodes[0].type = AST_FUNCTION_COTH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.5;
	}

void testAST_FUNCTION_CSC() {
	string = "AST_FUNCTION_CSC: (-1.838)";
	nodes[0].type = AST_FUNCTION_CSC;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 10.0;
	}

void testAST_FUNCTION_CSCH() {
	string = "AST_FUNCTION_CSCH: (0.276)";
	nodes[0].type = AST_FUNCTION_CSCH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.0;
	}

void testAST_FUNCTION_EXP() {
	string = "AST_FUNCTION_EXP: (330.3)";
	nodes[0].type = AST_FUNCTION_EXP;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 5.8;
	}

void testAST_FUNCTION_FACTORIAL() {
	string = "AST_FUNCTION_FACTORIAL: (720.0)";
	nodes[0].type = AST_FUNCTION_FACTORIAL;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 6.0;
	}

void testAST_FUNCTION_FLOOR() {
	string = "AST_FUNCTION_FLOOR: (12.0)";
	nodes[0].type = AST_FUNCTION_FLOOR;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 12.98;
	}

void testAST_FUNCTION_LN() {
	string = "AST_FUNCTION_LN: (1.609)";
	nodes[0].type = AST_FUNCTION_LN;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 5.0;
	}

void testAST_FUNCTION_LOG() {
	string = "AST_FUNCTION_LOG: (1.813)";
	nodes[0].type = AST_FUNCTION_LOG;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 65.0;
	}

void testAST_FUNCTION_PIECEWISE() {
	string = "AST_FUNCTION_PIECEWISE: (1.4)";
	nodes[0].type = AST_FUNCTION_PIECEWISE;
	nodes[0].childnum = 6;
	nodes[0].child = (int *)malloc(sizeof(int)*6);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[0].child[3] = 4;
	nodes[0].child[4] = 5;
	nodes[0].child[5] = 6;
	nodes[1].type = AST_REAL;
	nodes[1].value = 3.4;
	nodes[2].type = AST_CONSTANT_FALSE;
	nodes[3].type = AST_REAL;
	nodes[3].value = 1.4;
	nodes[4].type = AST_CONSTANT_TRUE;
	nodes[5].type = AST_REAL;
	nodes[5].value = 0.4;
	nodes[6].type = AST_CONSTANT_FALSE;
	}

void testAST_FUNCTION_POWER() {
	string = "AST_FUNCTION_POWER: (9.017)";
	nodes[0].type = AST_FUNCTION_POWER;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 2.4;
	}

void testAST_FUNCTION_ROOT() {
	string = "AST_FUNCTION_ROOT: (1.842)";
	nodes[0].type = AST_FUNCTION_ROOT;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 2.5;
	}

void testAST_FUNCTION_SEC() {
	string = "AST_FUNCTION_SEC: (-1.192)";
	nodes[0].type = AST_FUNCTION_SEC;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 10.0;
	}

void testAST_FUNCTION_SECH() {
	string = "AST_FUNCTION_SECH: (0.887)";
	nodes[0].type = AST_FUNCTION_SECH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 0.5;
	}

void testAST_FUNCTION_SIN() {
	string = "AST_FUNCTION_SIN: (0.841)";
	nodes[0].type = AST_FUNCTION_SIN;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	}

void testAST_FUNCTION_SINH() {
	string = "AST_FUNCTION_SINH: (1.175)";
	nodes[0].type = AST_FUNCTION_SINH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	}

void testAST_FUNCTION_TAN() {
	string = "AST_FUNCTION_TAN: (0.648)";
	nodes[0].type = AST_FUNCTION_TAN;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 10.0;
	}

void testAST_FUNCTION_TANH() {
	string = "AST_FUNCTION_TANH: (0.762)";
	nodes[0].type = AST_FUNCTION_TANH;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.0;
	}

void testAST_LOGICAL_AND() {
	string = "AST_LOGICAL_AND: (1.000)";
	nodes[0].type = AST_LOGICAL_AND;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_CONSTANT_TRUE;
	nodes[2].type = AST_CONSTANT_TRUE;
	}

void testAST_LOGICAL_NOT() {
	string = "AST_LOGICAL_NOT: (0.000)";
	nodes[0].type = AST_LOGICAL_NOT;
	nodes[0].childnum = 1;
	nodes[0].child = (int *)malloc(sizeof(int)*1);
	nodes[0].child[0] = 1;
	nodes[1].type = AST_CONSTANT_TRUE;
	}

void testAST_LOGICAL_OR() {
	string = "AST_LOGICAL_OR: (1.000)";
	nodes[0].type = AST_LOGICAL_OR;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_CONSTANT_TRUE;
	nodes[2].type = AST_CONSTANT_FALSE;
	}

void testAST_LOGICAL_XOR() {
	string = "AST_LOGICAL_XOR: (1.000)";
	nodes[0].type = AST_LOGICAL_XOR;
	nodes[0].childnum = 3;
	nodes[0].child = (int *)malloc(sizeof(int)*3);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[1].type = AST_CONSTANT_TRUE;
	nodes[2].type = AST_CONSTANT_TRUE;
	nodes[3].type = AST_CONSTANT_TRUE;
	}

void testAST_RELATIONAL_EQ() {
	string = "AST_RELATIONAL_EQ: (1.000)";
	nodes[0].type = AST_RELATIONAL_EQ;
	nodes[0].childnum = 3;
	nodes[0].child = (int *)malloc(sizeof(int)*3);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 1.5;
	nodes[3].type = AST_REAL;
	nodes[3].value = 1.5;
	}

void testAST_RELATIONAL_GEQ() {
	string = "AST_RELATIONAL_GEQ: (1.000)";
	nodes[0].type = AST_RELATIONAL_GEQ;
	nodes[0].childnum = 3;
	nodes[0].child = (int *)malloc(sizeof(int)*3);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 1.5;
	nodes[3].type = AST_REAL;
	nodes[3].value = 2.5;
	}

void testAST_RELATIONAL_GT() {
	string = "AST_RELATIONAL_GT: (1.000)";
	nodes[0].type = AST_RELATIONAL_GT;
	nodes[0].childnum = 3;
	nodes[0].child = (int *)malloc(sizeof(int)*3);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 1.7;
	nodes[3].type = AST_REAL;
	nodes[3].value = 2.5;
	}

void testAST_RELATIONAL_LEQ() {
	string = "AST_RELATIONAL_LEQ: (1.000)";
	nodes[0].type = AST_RELATIONAL_LEQ;
	nodes[0].childnum = 3;
	nodes[0].child = (int *)malloc(sizeof(int)*3);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[1].type = AST_REAL;
	nodes[1].value = 2.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 2.5;
	nodes[3].type = AST_REAL;
	nodes[3].value = 1.5;
	}

void testAST_RELATIONAL_LT() {
	string = "AST_RELATIONAL_LT: (1.000)";
	nodes[0].type = AST_RELATIONAL_LT;
	nodes[0].childnum = 3;
	nodes[0].child = (int *)malloc(sizeof(int)*3);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[0].child[2] = 3;
	nodes[1].type = AST_REAL;
	nodes[1].value = 1.5;
	nodes[2].type = AST_REAL;
	nodes[2].value = 1.2;
	nodes[3].type = AST_REAL;
	nodes[3].value = 0.5;
	}

void complexTest1() { /* (1.0 + pi + 3.0)*(-10.0/7.0) */
	string = "complexTest1: (-10.20)";
	nodes[0].type = AST_TIMES;
	nodes[0].childnum = 2;
	nodes[0].child = (int *)malloc(sizeof(int)*2);
	nodes[0].child[0] = 1;
	nodes[0].child[1] = 2;
	nodes[1].type = AST_PLUS;
	nodes[1].childnum = 3;
	nodes[1].child = (int *)malloc(sizeof(int)*3);
	nodes[1].child[0] = 3;
	nodes[1].child[1] = 4;
	nodes[1].child[2] = 5;
	nodes[2].type = AST_MINUS;
	nodes[2].childnum = 1;
	nodes[2].child = (int *)malloc(sizeof(int)*1);
	nodes[2].child[0] = 6;
	nodes[3].type = AST_REAL;
	nodes[3].value = 1.0;
	nodes[4].type = AST_CONSTANT_PI;
	nodes[5].type = AST_REAL;
	nodes[5].value = 3.0;
	nodes[6].type = AST_DIVIDE;
	nodes[6].childnum = 2;
	nodes[6].child = (int *)malloc(sizeof(int)*2);
	nodes[6].child[0] = 7;
	nodes[6].child[1] = 8;
	nodes[7].type = AST_REAL;
	nodes[7].value = 10.0;
	nodes[8].type = AST_REAL;
	nodes[8].value = 7.0;
	}

int main (int argc, char **argv) {
    
    int i, num = 0;
	int maxnodes = 20;
	int functionnum = 60;
	double x;
	directCode_t *code = (directCode_t *)malloc(sizeof(directCode_t));
	cvodeData_t *data = (cvodeData_t *)malloc(sizeof(cvodeData_t));
	void (**tests)() = (void (**)())malloc(sizeof(void (*)())*functionnum);
	nodes = (ASTNode_t *)malloc(sizeof(ASTNode_t)*maxnodes);
	data->value = (double *)malloc(sizeof(double)*10);
	data->value[5] = 4.25;
	data->currenttime = 13.4;
	
	tests[num++] = testAST_INTEGER;
	tests[num++] = testAST_REAL;
	tests[num++] = testAST_NAME;
	tests[num++] = testAST_FUNCTION_DELAY;
	tests[num++] = testAST_NAME_TIME;
	tests[num++] = testAST_CONSTANT_E;
	tests[num++] = testAST_CONSTANT_FALSE;
	tests[num++] = testAST_CONSTANT_PI;
	tests[num++] = testAST_CONSTANT_TRUE;
	tests[num++] = testAST_PLUS;
	tests[num++] = testAST_PLUS_large;
	tests[num++] = testAST_MINUS;
	tests[num++] = testAST_TIMES;
	tests[num++] = testAST_TIMES_large;
	tests[num++] = testAST_DIVIDE;
	tests[num++] = testAST_POWER;
	tests[num++] = testAST_FUNCTION;
	tests[num++] = testAST_FUNCTION_ABS;
	tests[num++] = testAST_FUNCTION_ARCCOS;
	tests[num++] = testAST_FUNCTION_ARCCOSH;
	tests[num++] = testAST_FUNCTION_ARCCOT;
	tests[num++] = testAST_FUNCTION_ARCCOTH;
	tests[num++] = testAST_FUNCTION_ARCCSC;
	tests[num++] = testAST_FUNCTION_ARCCSCH;
	tests[num++] = testAST_FUNCTION_ARCSEC;
	tests[num++] = testAST_FUNCTION_ARCSECH;
	tests[num++] = testAST_FUNCTION_ARCSIN;
	tests[num++] = testAST_FUNCTION_ARCSINH;
	tests[num++] = testAST_FUNCTION_ARCTAN;
	tests[num++] = testAST_FUNCTION_ARCTANH;
	tests[num++] = testAST_FUNCTION_CEILING;
	tests[num++] = testAST_FUNCTION_COS;
	tests[num++] = testAST_FUNCTION_COSH;
	tests[num++] = testAST_FUNCTION_COT;
	tests[num++] = testAST_FUNCTION_COTH;
	tests[num++] = testAST_FUNCTION_CSC;
	tests[num++] = testAST_FUNCTION_CSCH;
	tests[num++] = testAST_FUNCTION_EXP;
	tests[num++] = testAST_FUNCTION_FACTORIAL;
	tests[num++] = testAST_FUNCTION_FLOOR;
	tests[num++] = testAST_FUNCTION_LN;
	tests[num++] = testAST_FUNCTION_LOG;
	tests[num++] = testAST_FUNCTION_PIECEWISE;
	tests[num++] = testAST_FUNCTION_POWER;
	tests[num++] = testAST_FUNCTION_ROOT;
	tests[num++] = testAST_FUNCTION_SEC;
	tests[num++] = testAST_FUNCTION_SECH;
	tests[num++] = testAST_FUNCTION_SIN;
	tests[num++] = testAST_FUNCTION_SINH;
	tests[num++] = testAST_FUNCTION_TAN;
	tests[num++] = testAST_FUNCTION_TANH;
	tests[num++] = testAST_LOGICAL_AND;
	tests[num++] = testAST_LOGICAL_NOT;
	tests[num++] = testAST_LOGICAL_OR;
	tests[num++] = testAST_LOGICAL_XOR;
	tests[num++] = testAST_RELATIONAL_EQ;
	tests[num++] = testAST_RELATIONAL_GEQ;
	tests[num++] = testAST_RELATIONAL_GT;
	tests[num++] = testAST_RELATIONAL_LEQ;
	tests[num++] = testAST_RELATIONAL_LT;
	tests[num++] = complexTest1;
	
	/* tests */
	for(i = 0 ; i < num ; i++) {
		tests[i]();
		generateFunction(code, &nodes[0]);
		x = code->evaluate(data);
		destructFunction(code);
		printf("%s %8.3f\n", string, x);
		}

	return 0;
	}
