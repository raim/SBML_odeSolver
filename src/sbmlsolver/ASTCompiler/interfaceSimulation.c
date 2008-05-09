#include <stdlib.h>
/* defines all types of nodes in the abstract syntax tree, to work stand alone with the code generator */
typedef enum {AST_INTEGER, AST_REAL, AST_REAL_E, AST_RATIONAL, AST_NAME,
AST_FUNCTION_DELAY, AST_NAME_TIME, AST_CONSTANT_E, AST_CONSTANT_FALSE, AST_CONSTANT_PI, AST_CONSTANT_TRUE,
AST_PLUS, AST_MINUS, AST_TIMES, AST_DIVIDE, AST_POWER, AST_LAMBDA, AST_FUNCTION, AST_FUNCTION_ABS,
AST_FUNCTION_ARCCOS, AST_FUNCTION_ARCCOSH, AST_FUNCTION_ARCCOT, AST_FUNCTION_ARCCOTH,
AST_FUNCTION_ARCCSC, AST_FUNCTION_ARCCSCH, AST_FUNCTION_ARCSEC, AST_FUNCTION_ARCSECH,
AST_FUNCTION_ARCSIN, AST_FUNCTION_ARCSINH, AST_FUNCTION_ARCTAN, AST_FUNCTION_ARCTANH,
AST_FUNCTION_CEILING, AST_FUNCTION_COS, AST_FUNCTION_COSH, AST_FUNCTION_COT, AST_FUNCTION_COTH,
AST_FUNCTION_CSC, AST_FUNCTION_CSCH, AST_FUNCTION_EXP, AST_FUNCTION_FACTORIAL, AST_FUNCTION_FLOOR,
AST_FUNCTION_LN, AST_FUNCTION_LOG, AST_FUNCTION_PIECEWISE, AST_FUNCTION_POWER, AST_FUNCTION_ROOT,
AST_FUNCTION_SEC, AST_FUNCTION_SECH, AST_FUNCTION_SIN, AST_FUNCTION_SINH, AST_FUNCTION_TAN, AST_FUNCTION_TANH,
AST_LOGICAL_AND, AST_LOGICAL_NOT, AST_LOGICAL_OR, AST_LOGICAL_XOR,
AST_RELATIONAL_EQ, AST_RELATIONAL_GEQ, AST_RELATIONAL_GT, AST_RELATIONAL_LEQ, AST_RELATIONAL_LT
} ASTNodeType_t;

/* defines a node in the abstract syntax tree, to work stand alone with the code generator */
typedef struct {
	ASTNodeType_t type;
	int childnum;
	int *child;
	char *name;
	double value;
} ASTNode_t;

typedef struct {
	double *time;
	double **data;
} time_series_t;

typedef struct {
	int discrete_observation_data;
	int compute_vector_v;
	char **names;
	time_series_t *time_series;
} model_t;

typedef struct {
	int nvalues;
	int TimeSeriesIndex;
	double currenttime;
	double *value;
	model_t *model;
} cvodeData_t;

static double (*UsrDefFunc)(char*, int, double*) = NULL;
/* nodes: AST - abstract syntax tree */
ASTNode_t *nodes;

/* simulation of the ASTNode_t functions */
double ASTNode_getReal(ASTNode_t *n) {
	return n->value;
	}

int ASTNode_getInteger(ASTNode_t *n) {
	return n->childnum;
	}

ASTNodeType_t ASTNode_getType(ASTNode_t *n) {
	return n->type;
	}

int ASTNode_getNumChildren(ASTNode_t *n) {
	return n->childnum;
	}

char *ASTNode_getName(ASTNode_t *n) {
	return n->name;
	}

int ASTNode_isSetIndex(ASTNode_t *n) {
	return 0;
	}

int ASTNode_isSetData(ASTNode_t *n) {
	return 0;
	}

int ASTNode_getIndex(ASTNode_t *n) {
	return 0;
	}
	
double call( int index, double currenttime, time_series_t *ts) {
	return 0.0;
	}

ASTNode_t *child(ASTNode_t *n, int i) {
	return &nodes[n->child[i]];
	}
