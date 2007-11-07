#include <stdio.h>
#include <math.h>

/* THE FOLLOWING CODE SECTION EMULATES THE ENVIRONMENT FOR THE STANDALONE VERSION OF THE CODE GENERATOR AND MUST BE
OMITTED DURING THE INTEGRATION IN THE FRAMEWORK */

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

/* RELEVANT CODE STARTS HERE */

/* variables for the code generator */
/*codeSize: maximal code length (adjusts automatically) */
/* codePosition: actual writing code->codePosition in the code array */
/* FPUstackSize: maximal number of external saved values from the FPU stack */
/* FPUstackPosition: actual code->codePosition in the FPU extended stack */
/* storageSize: maximal number of constants that can be stored */
/* storagePosition: actual number of constants stored */
/* prog: array for the program code */
/* FPUstack: extention for the internal FPU stack */
/* storage: array for storing constants in the arithmetic expression */
typedef struct {
	int codeSize, FPUstackSize, storageSize;
	int codePosition, FPUstackPosition, storagePosition;
	unsigned char *prog;
	double *FPUstack, *storage;
} directCode;

directCode *code;
/* evaluate: name of the generated function - CALL THIS FUNCTION TO EVALUATE THE ARITHMETIC EXPRESSION */
double (*evaluate)(void);

/* initializes the basic parameters and allocated the memory - CONSTANTS FOR storageSize AND FPUstackSize MIGHT BE TO SMALL */
void initCode () {
	code = (directCode *)malloc(sizeof(directCode));
	code->codeSize = 1024;
	code->codePosition = 0;
	code->prog = (unsigned char *)malloc(sizeof(unsigned char)*code->codeSize);
	evaluate = (double (*)())code->prog;
	
	code->storageSize = 1024;
	code->storagePosition = 0;
	code->storage = (double *)malloc(sizeof(double)*code->storageSize);

	code->FPUstackSize = 1024;
	code->FPUstackPosition = 0;
	code->FPUstack = (double *)malloc(sizeof(double)*code->FPUstackSize);
	}

/* adds a byte to the code, extends the memory, if necessary */
void addByte (unsigned char byte) {

	int i;
	unsigned char *bigger;
	
	if(code->codePosition >= code->codeSize) {
		code->codeSize *= 2;
		bigger = (unsigned char *)malloc(sizeof(unsigned char)*code->codeSize);
		for(i = 0 ; i < code->codePosition ; i++)
			bigger[i] = code->prog[i];
		free(code->prog);
		code->prog = bigger;
		evaluate = (double (*)())code->prog;
		}
	
	code->prog[code->codePosition++] = byte;
	}

/* adds an code word of the code->codeSize of an integer (addresses or integers) to the code */
void addInt (int number) {
	unsigned int num;
	num = number;
	addByte(num%256); num /= 256;
	addByte(num%256); num /= 256;
	addByte(num%256); num /= 256;
	addByte(num%256);
	}

/* computes the necessary jump parameters for a function call and adds a call to the code */
void callFunction (unsigned int fun) {
	unsigned int num;
	num = fun - ((unsigned int)code->prog + code->codePosition + 5);
	addByte(0xe8); /* CALL */
	addInt(num);
	}

/* adds an element of the FPU stack to the external stack */
void pushStorage() {
	if(code->FPUstackPosition >= code->FPUstackSize)
		printf("code->FPUstack overflow\n");
	addByte(0xdd); addByte(0x1d); /* FSTP */
	addInt((int)&code->FPUstack[code->FPUstackPosition++]); /* code->FPUstack place */
	}

/* pops an element from the external stack into the FPU stack */
void popAddress() {
	if(code->FPUstackPosition <= 0)
		printf("code->FPUstack underflow\n");
//	addByte(0xdd); addByte(0x05); /* FLD */
	addInt((int)&code->FPUstack[--code->FPUstackPosition]); /* code->FPUstack code->codePosition */
	}

/* saves a constant in the code->storage an adds the address to the code */
void addConstant(double value) {
	if(code->storagePosition >= code->storageSize)
		printf("code->storage overflow\n");
	code->storage[code->storagePosition] = value;
	addInt((int)&code->storage[code->storagePosition++]);
	}

/* extern function for the case AST_NAME */
double getAST_Name(ASTNode_t *n, cvodeData_t *data) {
	int i, j;
	double found = 0, datafound, findtol=1e-5, result;
	time_series_t *ts = data->model->time_series;
	
	if (ASTNode_isSetIndex(n)) {
		if (ASTNode_isSetData(n)) {
		/* if continuous data is observed, obtain interpolated result */  
		if ((data->model->discrete_observation_data != 1) || (data->model->compute_vector_v != 1)) {
			result = call(ASTNode_getIndex(n),
			data->currenttime, ts);
			}
			else  /* if discrete data is observed, simply obtain value from time_series */
			{
			datafound = 0;
			i = data->TimeSeriesIndex;
			if (fabs(data->currenttime - ts->time[i]) < findtol) {
				result = ts->data[ASTNode_getIndex(n)][i];
				datafound++;
				}
			if (datafound != 1) {
/*				SolverError_error(FATAL_ERROR_TYPE,
					SOLVER_ERROR_AST_EVALUATION_FAILED_DISCRETE_DATA,
					"use of discrete time series data failed; none or several time points matching current time"); */
				result = 0; /*  break;  */
				}
				else
				found = 1;
			}
		}
		else
		{
		result = data->value[ASTNode_getIndex(n)];
		}
	found++;
	}
	if (found == 0) {
		if (strcmp(ASTNode_getName(n),"time") == 0 ||
			strcmp(ASTNode_getName(n),"Time") == 0 ||
			strcmp(ASTNode_getName(n),"TIME") == 0) {
			result = (double) data->currenttime;
			found++;
			}
		}
	if (found == 0) {
		for (j=0; j<data->nvalues; j++) {
			if (strcmp(ASTNode_getName(n),data->model->names[j]) == 0) {
				result = data->value[j];
				found++;
				}
			}
		}
	if (found == 0) {
/*		SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_EVALUATION_FAILED_MISSING_VALUE,
			"No value found for AST_NAME %s . Defaults to Zero "
			"to avoid program crash", ASTNode_getName(n)); */
		result = 0;
		}
	return result;
	}

/* some mathematical functions */
static double aCosh(double x) {
	return log(x + (sqrt(x - 1) * sqrt(x + 1)));
	}

static double aCoth(double x) {
	return log((x + 1.) / (x - 1.))/2.;
	}

static double aCsch(double x) {
	return log((1 + sqrt(1 + x * x)) / x);
	}

static double aSech(double x) {
	return log((1 + sqrt(1 - x * x)) / x);
	}

static double aSinh(double x) {
	return log(x + sqrt((x * x) + 1));
	}

static double aTanh(double x) {
	return (log(1 + x) - log(1-x))/2 ;
	}

static double factorial(double x) {

	int result = 0;
	for( result = 1 ; x > 1 ; x--)
		result *= x;
	return (double) result;
	}

/* generates the code */
void generate (ASTNode_t *AST, cvodeData_t *data) {
	
	int i, childnum, save;
	double st;
	ASTNodeType_t type;
	type = ASTNode_getType(AST);
	childnum = ASTNode_getNumChildren(AST);
	
	switch(type) {
		case AST_INTEGER:
			st = (double) ASTNode_getInteger(AST);
			addByte(0xdd); addByte(0x05); /* load double */
			addConstant(st);
			break;
		case AST_REAL: /* load double */
		case AST_REAL_E:
		case AST_RATIONAL:
			st = ASTNode_getReal(AST);
			addByte(0xdd); addByte(0x05); /* load double */
			addConstant(st);
			break;
		case AST_NAME:
			addByte(0x68); addInt((unsigned int)data); /* PUSH data */
			addByte(0x68); addInt((unsigned int)AST); /* PUSH AST */
			callFunction((unsigned int)getAST_Name);
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_DELAY: /* not implemented */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			break;
		case AST_NAME_TIME:
			addByte(0xdd); addByte(0x05); /* load double */
			addInt((int)&data->currenttime);
			break;

		case AST_CONSTANT_E:
			addByte(0xd9); addByte(0xea); /* log_2(e) */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe9); /* sub values */
			addByte(0xd9); addByte(0xf0); /* 2^x - 1 */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xc1); /* adds 1.0  */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd8); addByte(0xc0); /* -> 2.0 */
			addByte(0xde); addByte(0xc9); /* adds 2.0  */
			break;
		case AST_CONSTANT_FALSE: /* 0.0 */
			addByte(0xd9); addByte(0xee);
			break;
		case AST_CONSTANT_PI: /* pi */
			addByte(0xd9); addByte(0xeb);
			break;
		case AST_CONSTANT_TRUE: /* 1.0 */
			addByte(0xd9); addByte(0xe8);
			break;

		case AST_PLUS: /* add values */
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result */
				generate(child(AST,i), data);
				addByte(0xdc); addByte(0x05); popAddress(); /* reload old result and add */
				}
			break;
		case AST_MINUS: /* sub values */
		    if ( childnum<2 ) {
				generate(child(AST,0), data);
				addByte(0xd9); addByte(0xe0); /* inverse sign */
				}
				else
				{
				generate(child(AST,0), data);
				pushStorage(); /* save old result */
				generate(child(AST,1), data);
				addByte(0xdc); addByte(0x2d); popAddress(); /* reload old result and subtract */
				}
			break;
		case AST_TIMES: /* mult values */
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result */
				generate(child(AST,i), data);
				addByte(0xdc); addByte(0x0d); popAddress(); /* reload old result and mult */
				}
			break;
		case AST_DIVIDE: /* divide values */
			generate(child(AST,0), data);
			pushStorage(); /* save old result */
			generate(child(AST,1), data);
			addByte(0xdc); addByte(0x3d); popAddress(); /* reload old result and divide */
			break;
		case AST_POWER: /* calculates value 1 to the power of value 2 */
		case AST_FUNCTION_POWER:
			generate(child(AST,0), data);
			pushStorage(); /* save old result */
			generate(child(AST,1), data);
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xdd); addByte(0x05); popAddress(); /* load value 1 */
			addByte(0xd9); addByte(0xf1); /* log_2(value 1) */
			addByte(0xde); addByte(0xc9); /* mult */
			addByte(0xd9); addByte(0xc0); /* two times the value on the stack */
			addByte(0xd9); addByte(0xfc); /* round st0 to integer */
			addByte(0xdc); addByte(0xe9); /* sub value - round(value) */
			addByte(0xd9); addByte(0xc9); /* work with small part */
			addByte(0xd9); addByte(0xf0); /* 2^small(value) - 1.0 */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xc1); /* add */
			addByte(0xd9); addByte(0xfd); /* s^round(value) * smallerPart */
			addByte(0xdd); addByte(0xd9); /* clear stack */
			break;
		case AST_LAMBDA: /* not implemented */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			break;

		case AST_FUNCTION: /* user defined function */
			if (UsrDefFunc == NULL) { /* no function defined */
				addByte(0xd9); addByte(0xee); /* 0.0 */
				}
				else
				{
				save = (int)&code->FPUstack[code->FPUstackPosition];
				for (i = 0 ; i < childnum ; i++) { /* compute parameters and store them on the external stack */
					generate(child(AST,i), data);
					addByte(0xdd); addByte(0x1d); /* FSTP */
					addInt((int)&code->FPUstack[code->FPUstackPosition++]); /* code->FPUstack place */
					}
				addByte(0x68); addInt(save); /* PUSH array pointer */
				addByte(0x68); addInt(childnum); /* PUSH childnum */
				addByte(0x68); addInt((int)(char *)ASTNode_getName(AST)); /* PUSH name */
				callFunction((unsigned int)UsrDefFunc);
				addByte(0x83); addByte(0xc4); addByte(0x0c); /* ADD ESP 12 (parameter) */
				code->FPUstackPosition -= childnum;
				}
			break;
		case AST_FUNCTION_ABS: /* absolut value */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xe1); /* abs */
			break;
		case AST_FUNCTION_ARCCOS: /* arccos = pi/2.0 - arcsin */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xc0); /* load value again */
			addByte(0xd8); addByte(0xc8); /* add value^2 to stack */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe1); /* 1.0 - value^2 */
			addByte(0xd9); addByte(0xfa); /* sqrt */
			addByte(0xd9); addByte(0xf3); /* computes the arcsin(value) = arctan(value / sqrt(1.0-value^2)) */
			addByte(0xd9); addByte(0xeb); /* pi */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd8); addByte(0xc0); /* -> 2.0 */
			addByte(0xde); addByte(0xf9); /* pi/2.0 */
			addByte(0xde); addByte(0xe1); /* pi/2.0 - arcsin */
			break;
		case AST_FUNCTION_ARCCOSH: /* arccos hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)aCosh); /* aCosh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_ARCCOT: /* arccot = pi/2.0 - arctan */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xf3); /* computes the arctan(value / 1.0) */
			addByte(0xd9); addByte(0xeb); /* pi */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd8); addByte(0xc0); /* -> 2.0 */
			addByte(0xde); addByte(0xf9); /* pi/2.0 */
			addByte(0xde); addByte(0xe1); /* pi/2.0 - arctan */
			break;
		case AST_FUNCTION_ARCCOTH: /* arccot hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)aCoth); /* aCoth */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_ARCCSC: /* arccsc = arcsin(1.0/value) */
			generate(child(AST,0), data);
			addByte(0xd8); addByte(0xc8); /* add value^2 to stack */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe9); /* value^2 - 1.0 */
			addByte(0xd9); addByte(0xfa); /* sqrt */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and sqrt(value^2 - 1.0) */
			addByte(0xd9); addByte(0xf3); /* computes the arcsin(value) = arctan(1.0 / sqrt(value^2 - 1.0)) */
			break;
		case AST_FUNCTION_ARCCSCH: /* arccsc hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)aCsch); /* aCsch */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_ARCSEC: /* arccsc = arccos(1.0/value) =  pi/2.0 - arcsin(1.0/value) */
			generate(child(AST,0), data);
			addByte(0xd8); addByte(0xc8); /* add value^2 to stack */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe9); /* value^2 - 1.0 */
			addByte(0xd9); addByte(0xfa); /* sqrt */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and sqrt(value^2 - 1.0) */
			addByte(0xd9); addByte(0xf3); /* computes the arcsin(value) = arctan(1.0 / sqrt(value^2 - 1.0)) */
			addByte(0xd9); addByte(0xeb); /* pi */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd8); addByte(0xc0); /* -> 2.0 */
			addByte(0xde); addByte(0xf9); /* pi/2.0 */
			addByte(0xde); addByte(0xe1); /* pi/2.0 - arcsin */
			break;
		case AST_FUNCTION_ARCSECH: /* arcsec hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)aSech); /* aSech */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_ARCSIN: /* arcsin */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xc0); /* load value again */
			addByte(0xd8); addByte(0xc8); /* add value^2 to stack */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe1); /* 1.0 - value^2 */
			addByte(0xd9); addByte(0xfa); /* sqrt */
			addByte(0xd9); addByte(0xf3); /* computes the arcsin(value) = arctan(value / sqrt(1.0-value^2)) */
			break;
		case AST_FUNCTION_ARCSINH: /* arcsin hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)aSinh); /* asinh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_ARCTAN: /* arctan */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xf3); /* computes the arctan(value / 1.0) */
			break;
		case AST_FUNCTION_ARCTANH: /* arctan hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)aTanh); /* atanh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_CEILING: /* rounds value to next integer */
			generate(child(AST,0), data);
			addByte(0x9b); addByte(0xd9); addByte(0x3d); /* FSTCW */
			if(code->FPUstackPosition >= code->FPUstackSize)
				printf("code->FPUstack overflow\n");
			addInt((int)&code->FPUstack[code->FPUstackPosition++]); /* code->FPUstack place */
			addByte(0xc7); addByte(0x05); /* MOV DWORD */
			if(code->FPUstackPosition >= code->FPUstackSize)
				printf("code->FPUstack overflow\n");
			addInt((int)&code->FPUstack[code->FPUstackPosition++]); /* code->FPUstack place */
			addInt(0x0b63);
			addByte(0xd9); addByte(0x2d); /* FLDCW */
			popAddress();
			addByte(0xd9); addByte(0xfc); /* frndint */
			addByte(0x9b); addByte(0xdb); addByte(0xe2); /*fclex */
			addByte(0xd9); addByte(0x2d); /* FLDCW */
			popAddress();
			break;
		case AST_FUNCTION_COS: /* cosinus */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xff); /* computes the cosinus */
			break;
		case AST_FUNCTION_COSH: /* cosinus hyperbilic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)cosh); /* cosh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_COT: /* cotangens = 1/tan */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xfb); /* computes the sinus and cosinus */
			addByte(0xde); addByte(0xf1); /* cos(value) / sin(value) */
			break;
		case AST_FUNCTION_COTH: /* coth = 1/tanh */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)tanh); /* tanh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xf1); /* 1.0/tanh */
			break;
		case AST_FUNCTION_CSC: /* cosec = 1/sinus */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xfe); /* computes the sinus */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xf1); /* 1.0 / sin(value) */
			break;
		case AST_FUNCTION_CSCH: /* csch = 1/sinh */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)sinh); /* sinh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xf1); /* 1.0/sinh */
			break;
		case AST_FUNCTION_EXP: /* exp( value ) */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xea); /* log_2(e) */
			addByte(0xde); addByte(0xc9); /* mult */
			addByte(0xd9); addByte(0xc0); /* two times the value on the stack */
			addByte(0xd9); addByte(0xfc); /* round st0 to integer */
			addByte(0xdc); addByte(0xe9); /* sub value - round(value) */
			addByte(0xd9); addByte(0xc9); /* work with small part */
			addByte(0xd9); addByte(0xf0); /* 2^small(value) - 1.0 */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xc1); /* add */
			addByte(0xd9); addByte(0xfd); /* s^round(value) * smallerPart */
			addByte(0xdd); addByte(0xd9); /* clear stack */
			break;
		case AST_FUNCTION_FACTORIAL:
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)factorial); /* factorial */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_FLOOR: /* rounds value to next lower integer */
			generate(child(AST,0), data);
			addByte(0x9b); addByte(0xd9); addByte(0x3d); /* FSTCW */
			if(code->FPUstackPosition >= code->FPUstackSize)
				printf("code->FPUstack overflow\n");
			addInt((int)&code->FPUstack[code->FPUstackPosition++]); /* code->FPUstack place */
			addByte(0xc7); addByte(0x05); /* MOV DWORD */
			if(code->FPUstackPosition >= code->FPUstackSize)
				printf("code->FPUstack overflow\n");
			addInt((int)&code->FPUstack[code->FPUstackPosition++]); /* code->FPUstack place */
			addInt(0x0763);
			addByte(0xd9); addByte(0x2d); /* FLDCW */
			popAddress();
			addByte(0xd9); addByte(0xfc); /* frndint */
			addByte(0x9b); addByte(0xdb); addByte(0xe2); /*fclex */
			addByte(0xd9); addByte(0x2d); /* FLDCW */
			popAddress();
			break;
		case AST_FUNCTION_LN: /* log_e */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xed); /* log_e(2) */
			addByte(0xd9); addByte(0xc9); /* fxch (change order) */
			addByte(0xd9); addByte(0xf1); /* log_e(2) * log_2(value) */
			break;
		case AST_FUNCTION_LOG: /* log_10 */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xec); /* log_10(2) */
			addByte(0xd9); addByte(0xc9); /* fxch (change order) */
			addByte(0xd9); addByte(0xf1); /* log_10(2) * log_2(value) */
			break;
		case AST_FUNCTION_PIECEWISE: /* USEFUL? NOT CORRECTLY IMPLEMENTED! */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			pushStorage(); /* save result */
		    for ( i = 0 ; i < childnum-1 ; i = i + 2) {
				generate(child(AST,i+1), data);
				pushStorage(); /* save old result */
				generate(child(AST,i), data);
				addByte(0xdd); addByte(0x05); popAddress(); /* load value 1 */
				addByte(0xd9); addByte(0xee); /* 0.0 */
				addByte(0xdf); addByte(0xf1); /* compare value with 0.0 and pop the zero */
				addByte(0xdd); addByte(0xc0);
				addByte(0xd9); addByte(0xf7); /* delete compare value */
				addByte(0xdd); addByte(0x05); popAddress(); /* load result */
				addByte(0xdb); addByte(0xc9); /* if not 0.0 store generated value in result */
				pushStorage(); /* save result */
				addByte(0xdd); addByte(0xc0);
				addByte(0xd9); addByte(0xf7); /* delete computed value */
				}
			addByte(0xdd); addByte(0x05); popAddress(); /* load result */
			break;
		/* AST_FUNCTION_POWER see AST_POWER */
		case AST_FUNCTION_ROOT:
			generate(child(AST,1), data);
			pushStorage(); /* save old result */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xdd); addByte(0x05); popAddress(); /* load value 1 */
			addByte(0xd9); addByte(0xf1); /* log_2(value 1) */
			addByte(0xde); addByte(0xf1); /* mult */
			addByte(0xd9); addByte(0xc0); /* two times the value on the stack */
			addByte(0xd9); addByte(0xfc); /* round st0 to integer */
			addByte(0xdc); addByte(0xe9); /* sub value - round(value) */
			addByte(0xd9); addByte(0xc9); /* work with small part */
			addByte(0xd9); addByte(0xf0); /* 2^small(value) - 1.0 */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xc1); /* add */
			addByte(0xd9); addByte(0xfd); /* s^round(value) * smallerPart */
			addByte(0xdd); addByte(0xd9); /* clear stack */
			break;
		case AST_FUNCTION_SEC: /* sec = 1/cosinus */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xff); /* computes the cosinus */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xf1); /* 1.0 / cos(value) */
			break;
		case AST_FUNCTION_SECH: /* sech = 1/cosh */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)cosh); /* cosh */
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xf1); /* 1.0/cosh */
			break;
		case AST_FUNCTION_SIN: /* sinus */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xfe); /* computes the sinus */
			break;
		case AST_FUNCTION_SINH: /* sinus hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)sinh);
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;
		case AST_FUNCTION_TAN: /* tangens */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xfb); /* computes the sinus and cosinus */
			addByte(0xde); addByte(0xf9); /* sin(value) / cos(value) */
			break;
		case AST_FUNCTION_TANH: /* tangens hyperbolic */
			generate(child(AST,0), data);
			addByte(0x83); addByte(0xec); addByte(0x08); /* SUB ESP 8 (parameter) */
			addByte(0xdd); addByte(0x1c); addByte(0x24); /* load parameter 1 */
			callFunction((unsigned int)tanh);
			addByte(0x83); addByte(0xc4); addByte(0x08); /* ADD ESP 8 (parameter) */
			break;

/* logical cases depends from 0.0 and 1.0 as initial values */
		case AST_LOGICAL_AND:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result */
				generate(child(AST,i), data);
				addByte(0xdc); addByte(0x0d); popAddress(); /* reload old result and mult */
				}
			break;
		case AST_LOGICAL_NOT:
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe1); /* 1.0 - value */
			break;
		case AST_LOGICAL_OR: /* !(AND !values) */
			generate(child(AST,0), data);
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe1); /* 1.0 - value */
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result */
				generate(child(AST,i), data);
				addByte(0xd9); addByte(0xe8); /* 1.0 */
				addByte(0xde); addByte(0xe1); /* 1.0 - value */
				addByte(0xdc); addByte(0x0d); popAddress(); /* reload old result and mult */
				}
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xde); addByte(0xe1); /* 1.0 - value */
			break;
		case AST_LOGICAL_XOR:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result */
				generate(child(AST,i), data);
				addByte(0xdd); addByte(0x05); popAddress(); /* load value 1 */
				addByte(0xde); addByte(0xe9); /* sub value1 - value0 */
				addByte(0xd9); addByte(0xe1); /* abs */
				}
			break;
		case AST_RELATIONAL_EQ:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result to stack*/
				generate(child(AST,i), data);
				}
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and value */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 0.0 and value */
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				addByte(0xdd); addByte(0x05); popAddress(); /* load old value from stack */
				addByte(0xdf); addByte(0xf1); /* compare two values and pop one*/
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				addByte(0xdb); addByte(0xc9); /* if not equal store 0.0 in old result */
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				}
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete old value */
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete 0.0 */
			break;
		case AST_RELATIONAL_GEQ:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result to stack*/
				generate(child(AST,i), data);
				}
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and value */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 0.0 and value */
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				addByte(0xdd); addByte(0x05); popAddress(); /* load old value from stack */
				addByte(0xd9); addByte(0xc9); /* exchange values */
				addByte(0xdf); addByte(0xf1); /* compare two values and pop one*/
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				addByte(0xda); addByte(0xc1); /* if not geq store 0.0 in old result */
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				}
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete old value */
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete 0.0 */
			break;
		case AST_RELATIONAL_GT:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result to stack*/
				generate(child(AST,i), data);
				}
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and value */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 0.0 and value */
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				addByte(0xdd); addByte(0x05); popAddress(); /* load old value from stack */
				addByte(0xd9); addByte(0xc9); /* exchange values */
				addByte(0xdf); addByte(0xf1); /* compare two values and pop one*/
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				addByte(0xda); addByte(0xd1); /* if not gt store 0.0 in old result */
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				}
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete old value */
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete 0.0 */
			break;
		case AST_RELATIONAL_LEQ:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result to stack*/
				generate(child(AST,i), data);
				}
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and value */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 0.0 and value */
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				addByte(0xdd); addByte(0x05); popAddress(); /* load old value from stack */
				addByte(0xd9); addByte(0xc9); /* exchange values */
				addByte(0xdf); addByte(0xf1); /* compare two values and pop one*/
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				addByte(0xdb); addByte(0xd1); /* if not leq store 0.0 in old result */
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				}
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete old value */
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete 0.0 */
			break;
		case AST_RELATIONAL_LT:
			generate(child(AST,0), data);
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(); /* save old result to stack*/
				generate(child(AST,i), data);
				}
			addByte(0xd9); addByte(0xe8); /* 1.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 1.0 and value */
			addByte(0xd9); addByte(0xee); /* 0.0 */
			addByte(0xd9); addByte(0xc9); /* exchange 0.0 and value */
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				addByte(0xdd); addByte(0x05); popAddress(); /* load old value from stack */
				addByte(0xd9); addByte(0xc9); /* exchange values */
				addByte(0xdf); addByte(0xf1); /* compare two values and pop one*/
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				addByte(0xdb); addByte(0xc1); /* if not lt store 0.0 in old result */
				addByte(0xd9); addByte(0xca); /* exchange value and old result */
				}
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete old value */
			addByte(0xdd); addByte(0xc0);
			addByte(0xd9); addByte(0xf7); /* delete 0.0 */
			break;

		default:
			printf("ERROR: Wrong AST_type\n");
			addByte(0xd9); addByte(0xee); /* 0.0 */
			break;
		}
	}

/* generates the basic elements of the function - CALL THIS FUNCTION TO GENERATE THE FUNCTION */
void generateFunction(ASTNode_t *AST, cvodeData_t *data) {
	int i;
	initCode();
	code->codePosition = 0;
	code->FPUstackPosition = 0;
	
	addByte(0x55); /* PUSH EBP */
	addByte(0x89); addByte(0xe5); /* MOV EBP, ESP */
	generate(AST, data);
	addByte(0x5d); /* POP EBP */
	addByte(0xc3); /* RETN */
	}
