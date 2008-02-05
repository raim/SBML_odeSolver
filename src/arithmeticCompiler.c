/*
  Last changed Time-stamp: <2008-02-05 16:02:23 raim>
  $Id: arithmeticCompiler.c,v 1.5 2008/02/05 16:19:58 raimc Exp $
*/
/* 
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. The software and
 * documentation provided hereunder is on an "as is" basis, and the
 * authors have no obligations to provide maintenance, support,
 * updates, enhancements or modifications.  In no event shall the
 * authors be liable to any party for direct, indirect, special,
 * incidental or consequential damages, including lost profits, arising
 * out of the use of this software and its documentation, even if the
 * authors have been advised of the possibility of such damage.  See
 * the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * The original code contained here was initially developed by:
 *     Matthias Rosensteiner
 *     Markus Loeberbauer
 *
 * Contributor(s):
 *     Stefan Müller
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "sbmlsolver/arithmeticCompiler.h"
#include "sbmlsolver/processAST.h"

/* locally defined in processAST.c */
static double (*UsrDefFunc)(char*, int, double*) = NULL;

static int analyse (directCode_t *c, ASTNode_t *AST);
static int analyseFPU (ASTNode_t *AST);

/* quasi assembler, helps for better readability */
#define ass_F2XM1			addByte(c,0xd9); addByte(c,0xf0);
#define ass_FABS			addByte(c,0xd9); addByte(c,0xe1);
#define ass_FADD_st(i)		addByte(c,0xd8); addByte(c,0xc0+i);
#define ass_FADDP_st(i)		addByte(c,0xde); addByte(c,0xc0+i);
#define ass_FADD_mem		addByte(c,0xdc); addByte(c,0x05);
#define ass_FCHS			addByte(c,0xd9); addByte(c,0xe0);
#define ass_FCMOVB_st(i)	addByte(c,0xda); addByte(c,0xc0+i);
#define ass_FCMOVBE_st(i)	addByte(c,0xda); addByte(c,0xd0+i);
#define ass_FCMOVNB_st(i)	addByte(c,0xdb); addByte(c,0xc0+i);
#define ass_FCMOVNBE_st(i)	addByte(c,0xdb); addByte(c,0xd0+i);
#define ass_FCMOVNE_st(i)	addByte(c,0xdb); addByte(c,0xc8+i);
#define ass_FCOMIP_st(i)	addByte(c,0xdf); addByte(c,0xf0+i);
#define ass_FCOS			addByte(c,0xd9); addByte(c,0xff);
#define ass_FDIVP_st(i)		addByte(c,0xde); addByte(c,0xf8+i);
#define ass_FDIVRP_st(i)	addByte(c,0xde); addByte(c,0xf0+i);
#define ass_FDIVR_mem		addByte(c,0xdc); addByte(c,0x3d);
#define ass_FFREE_st(i)		addByte(c,0xdd); addByte(c,0xc0+i);
#define ass_FINCSTP			addByte(c,0xd9); addByte(c,0xf7);
#define ass_FLD_mem			addByte(c,0xdd); addByte(c,0x05);
#define ass_FLD_st(i)		addByte(c,0xd9); addByte(c,0xc0+i);
#define ass_FLD1			addByte(c,0xd9); addByte(c,0xe8);
#define ass_FLDL2E			addByte(c,0xd9); addByte(c,0xea);
#define ass_FLDPI			addByte(c,0xd9); addByte(c,0xeb);
#define ass_FLDLG2			addByte(c,0xd9); addByte(c,0xec);
#define ass_FLDLN2			addByte(c,0xd9); addByte(c,0xed);
#define ass_FLDZ			addByte(c,0xd9); addByte(c,0xee);
#define ass_FMUL_st(i)		addByte(c,0xd8); addByte(c,0xc8+i);
#define ass_FMULP_st(i)		addByte(c,0xde); addByte(c,0xc8+i);
#define ass_FMUL_mem		addByte(c,0xdc); addByte(c,0x0d);
#define ass_FPATAN			addByte(c,0xd9); addByte(c,0xf3);
#define ass_FPTAN			addByte(c,0xd9); addByte(c,0xf2);
#define ass_FRNDINT			addByte(c,0xd9); addByte(c,0xfc);
#define ass_FSCALE			addByte(c,0xd9); addByte(c,0xfd);
#define ass_FSIN			addByte(c,0xd9); addByte(c,0xfe);
#define ass_FSINCOS			addByte(c,0xd9); addByte(c,0xfb);
#define ass_FSQRT			addByte(c,0xd9); addByte(c,0xfa);
#define ass_FSTP_st(i)		addByte(c,0xdd); addByte(c,0xd8+i);
#define ass_FSTP_mem		addByte(c,0xdd); addByte(c,0x1d);
#define ass_FSUB_st(i)		addByte(c,0xdc); addByte(c,0xe8+i);
#define ass_FSUBP_st(i)		addByte(c,0xde); addByte(c,0xe8+i);
#define ass_FSUBR_st(i)		addByte(c,0xd8); addByte(c,0xe8+i);
#define ass_FSUBRP_st(i)	addByte(c,0xde); addByte(c,0xe0+i);
#define ass_FSUBR_mem		addByte(c,0xdc); addByte(c,0x2d);
#define ass_FXCH_st(i)		addByte(c,0xd9); addByte(c,0xc8+i);
#define ass_FYL2X			addByte(c,0xd9); addByte(c,0xf1);

/* variables for the code generator */
/*codeSize: maximal code length */
/* codePosition: actual writing code->codePosition in the code array */
/* FPUstackSize: maximal number of external saved values from the FPU stack */
/* FPUstackPosition: actual code->codePosition in the FPU extended stack */
/* storageSize: maximal number of constants that can be stored */
/* storagePosition: actual number of constants stored */
/* prog: array for the program code */
/* FPUstack: extention for the internal FPU stack */
/* storage: array for storing constants in the arithmetic expression */
/* evaluate: name of the generated function - CALL THIS FUNCTION TO EVALUATE THE ARITHMETIC EXPRESSION */

/* initializes the basic parameters and allocated the memory */
void initCode (directCode_t *code, ASTNode_t *AST) {

	int length;

	code->codeSize = 5; /* bytes for basic function construction */
	code->codePosition = 0;
	code->storageSize = 0;
	code->storagePosition = 0;
	length = analyse(code, AST);
	if(length <= 8)
		length = 0;
		else
		length -= 8;
	code->FPUstackSize = length;
	code->FPUstackPosition = 0;
	
	code->prog = (unsigned char *)malloc(sizeof(unsigned char)*code->codeSize);
	code->storage = (double *)malloc(sizeof(double)*code->storageSize);
	code->FPUstack = (double *)malloc(sizeof(double)*code->FPUstackSize);

	code->evaluate = (double (*)())code->prog;
	}

/* adds a byte to the code, extends the memory, if necessary */
void addByte (directCode_t *code, unsigned char byte) {

	if(code->codePosition >= code->codeSize)
		printf("programm is too long\n");
	
	code->prog[code->codePosition++] = byte;
	}

/* adds an code word of the code->codeSize of an integer (addresses or integers) to the code */
void addInt (directCode_t *c, int number) {
	unsigned int num;
	num = number;
	addByte(c,num%256); num /= 256;
	addByte(c,num%256); num /= 256;
	addByte(c,num%256); num /= 256;
	addByte(c,num%256);
	}

/* adds an adress to the code */
void addAddress (directCode_t *c, long long addy) {
	int i, addressLength = sizeof(void (*)());
	
	for(i = 0 ; i < addressLength ; i++) {
		addByte(c,addy%256);
		addy /= 256;
		}
	}



/* adds the parameter to the CPU stack, computes the necessary jump parameters for a function call and adds a call to the code */
void callMathFunction (directCode_t *c, long long fun) {
	addByte(c,0x83); addByte(c,0xec); addByte(c,0x08); /* SUB ESP 8 (parameter) */
	addByte(c,0xdd); addByte(c,0x1c); addByte(c,0x24); /* load parameter 1 */
	fun -= ((long long)c->prog + c->codePosition + sizeof(void (*)()) + 1);
	addByte(c, 0xe8); addAddress(c, fun); /* CALL */
	addByte(c,0x83); addByte(c,0xc4); addByte(c,0x08); /* ADD ESP 8 (parameter) */
	}

/* computes the necessary jump parameters for a function call and adds a call to the code */
void callFunction (directCode_t *c, long long fun) {
	fun -= ((long long)c->prog + c->codePosition + sizeof(void (*)()) + 1);
	addByte(c, 0xe8); addAddress(c, fun); /* CALL */
	}

/* adds an element of the FPU stack to the external stack */
void pushStorage (directCode_t *c) {
	if(c->FPUstackPosition >= c->FPUstackSize)
		printf("code->FPUstack overflow\n");
	ass_FSTP_mem
	addAddress(c, (long long)&c->FPUstack[c->FPUstackPosition++]); /* code->FPUstack place */
	}

/* pops an element from the external stack into the FPU stack */
void popAddress (directCode_t *code) {
	if(code->FPUstackPosition <= 0)
		printf("code->FPUstack underflow\n");
	/*	addByte(c,0xdd); addByte(c,0x05);  *//* FLD */
	addAddress(code, (long long)&code->FPUstack[--code->FPUstackPosition]); /* code->FPUstack code->codePosition */
	}

/* saves a constant in the code->storage an adds the address to the code */
void addConstant (directCode_t *code, double value) {
	if(code->storagePosition >= code->storageSize)
		printf("code->storage overflow\n");
	code->storage[code->storagePosition] = value;
	addAddress(code, (long long)&code->storage[code->storagePosition++]);
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

double getAST_Name_Time(cvodeData_t *data) {
	return data->currenttime;
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

/* generates the code from the abstract syntax tree */
void generate (directCode_t *c, ASTNode_t *AST) {
	
	int i, childnum;
	long long save;
	double st;
	ASTNodeType_t type;
	type = ASTNode_getType(AST);
	childnum = ASTNode_getNumChildren(AST);
	
	switch(type) {
		case AST_INTEGER:
			st = (double) ASTNode_getInteger(AST);
			ass_FLD_mem
			addConstant(c,st);
			break;
		case AST_REAL: /* load double */
		case AST_REAL_E:
		case AST_RATIONAL:
			st = ASTNode_getReal(AST);
			ass_FLD_mem
			addConstant(c,st);
			break;
		case AST_NAME:
			addByte(c,0x8b); addByte(c,0x45); addByte(c,0x08); /* MOV EAX ESP+8 */
			addByte(c,0x50); /* PUSH data - in real: PUSH EAX */
			addByte(c,0x68); addAddress(c,(long long)AST); /* PUSH AST */
			callFunction(c,(long long)getAST_Name);
			addByte(c,0x83); addByte(c,0xc4); addByte(c,0x08); /* ADD ESP 8 (parameter)  // delete parameter from the stack */
			break;
		case AST_FUNCTION_DELAY: /* not implemented */
			ass_FLDZ
			break;
		case AST_NAME_TIME:
			addByte(c,0x8b); addByte(c,0x45); addByte(c,0x08); /* MOV EAX ESP+8 */
			addByte(c,0x50); /* PUSH data - in real: PUSH EAX */
			callFunction(c,(long long)getAST_Name_Time);
			addByte(c,0x83); addByte(c,0xc4); addByte(c,0x04); /* ADD ESP 4 (parameter)  // delete parameter from the stack */
			break;

		case AST_CONSTANT_E:
			ass_FLDL2E
			ass_FLD1
			ass_FSUBP_st(1)
			ass_F2XM1
			ass_FLD1
			ass_FADDP_st(1)
			ass_FLD1
			ass_FADD_st(0)
			ass_FMULP_st(1)
			break;
		case AST_CONSTANT_FALSE: /* 0.0 */
			ass_FLDZ
			break;
		case AST_CONSTANT_PI: /* pi */
			ass_FLDPI
			break;
		case AST_CONSTANT_TRUE: /* 1.0 */
			ass_FLD1
			break;

		case AST_PLUS: /* add values */
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				if(analyseFPU(child(AST,i)) >= 8) {
					pushStorage(c); /* save old result */
					generate(c,child(AST,i));
					ass_FADD_mem popAddress(c); /* reload old result and add */
					} else {
					generate(c,child(AST,i));
					ass_FADDP_st(1)
					}
				}
			break;
		case AST_MINUS: /* sub values */
		    if ( childnum<2 ) {
				generate(c,child(AST,0));
				ass_FCHS
				} else {
				generate(c,child(AST,0));
				if(analyseFPU(child(AST,1)) >= 8) {
					pushStorage(c); /* save old result */
					generate(c,child(AST,1));
					ass_FSUBR_mem popAddress(c); /* reload old result and subtract */
					} else {
					generate(c,child(AST,1));
					ass_FSUBP_st(1)
					}
				}
			break;
		case AST_TIMES: /* mult values */
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				if(analyseFPU(child(AST,i)) >= 8) {
					pushStorage(c); /* save old result */
					generate(c,child(AST,i));
					ass_FMUL_mem popAddress(c); /* reload old result and mult */
					} else {
					generate(c,child(AST,i));
					ass_FMULP_st(1)
					}
				}
			break;
		case AST_DIVIDE: /* divide values */
			generate(c,child(AST,0));
			if(analyseFPU(child(AST,1)) >= 8) {
				pushStorage(c); /* save old result */
				generate(c,child(AST,1));
				ass_FDIVR_mem popAddress(c); /* reload old result and divide */
				} else {
				generate(c,child(AST,1));
				ass_FDIVP_st(1)
				}
			break;
		case AST_POWER: /* calculates value 1 to the power of value 2 */
		case AST_FUNCTION_POWER:
			generate(c,child(AST,0));
			if(analyseFPU(child(AST,1)) >= 8) {
				pushStorage(c); /* save old result */
				generate(c,child(AST,1));
				ass_FLD1
				ass_FLD_mem popAddress(c); /* load value 1 */
				} else {
				generate(c,child(AST,1));
				ass_FXCH_st(1)
				ass_FLD1
				ass_FXCH_st(1)
				}
			ass_FYL2X
			ass_FMULP_st(1)
			ass_FLD_st(0)
			ass_FRNDINT
			ass_FSUB_st(1)
			ass_FXCH_st(1)
			ass_F2XM1
			ass_FLD1
			ass_FADDP_st(1)
			ass_FSCALE
			ass_FSTP_st(1)
			break;
		case AST_LAMBDA: /* not implemented */
			ass_FLDZ
			break;

		case AST_FUNCTION: /* user defined function */
			if (UsrDefFunc == NULL) { /* no function defined */
				ass_FLDZ
				}
				else
				{
				save = (long long)&c->FPUstack[c->FPUstackPosition];
				for (i = 0 ; i < childnum ; i++) { /* compute parameters and store them on the external stack */
					generate(c,child(AST,i));
					ass_FSTP_mem
					addInt(c,(long long)&c->FPUstack[c->FPUstackPosition++]); /* code->FPUstack place */
					}
				addByte(c,0x68); addAddress(c,save); /* PUSH array pointer */
				addByte(c,0x68); addInt(c,childnum); /* PUSH childnum */
				addByte(c,0x68); addAddress(c,(long long)(char *)ASTNode_getName(AST)); /* PUSH name */
				callFunction(c,(long long)UsrDefFunc);
				addByte(c,0x83); addByte(c,0xc4); addByte(c,0x0c); /* ADD ESP 12 (parameter) */
				c->FPUstackPosition -= childnum;
				}
			break;
		case AST_FUNCTION_ABS: /* absolut value */
			generate(c,child(AST,0));
			ass_FABS
			break;
		case AST_FUNCTION_ARCCOS: /* arccos = pi/2.0 - arcsin  = pi/2.0 - arctan(value / sqrt(1.0-value^2)) */
			generate(c,child(AST,0));
			ass_FLD_st(0)
			ass_FMUL_st(0)
			ass_FLD1
			ass_FSUBRP_st(1)
			ass_FSQRT
			ass_FPATAN
			ass_FLDPI
			ass_FLD1
			ass_FADD_st(0)
			ass_FDIVP_st(1)
			ass_FSUBRP_st(1)
			break;
		case AST_FUNCTION_ARCCOSH: /* arccos hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)aCosh); /* aCosh */
			break;
		case AST_FUNCTION_ARCCOT: /* arccot = pi/2.0 - arctan */
			generate(c,child(AST,0));
			ass_FLD1
			ass_FPATAN
			ass_FLDPI
			ass_FLD1
			ass_FADD_st(0)
			ass_FDIVP_st(1)
			ass_FSUBRP_st(1)
			break;
		case AST_FUNCTION_ARCCOTH: /* arccot hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)aCoth); /* aCoth */
			break;
		case AST_FUNCTION_ARCCSC: /* arccsc = arcsin(1.0/value) */
			generate(c,child(AST,0));
			ass_FMUL_st(0)
			ass_FLD1
			ass_FSUBP_st(1)
			ass_FSQRT
			ass_FLD1
			ass_FXCH_st(1)
			ass_FPATAN
			break;
		case AST_FUNCTION_ARCCSCH: /* arccsc hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)aCsch); /* aCsch */
			break;
		case AST_FUNCTION_ARCSEC: /* arccsc = arccos(1.0/value) =  pi/2.0 - arcsin(1.0/value) */
			generate(c,child(AST,0));
			ass_FMUL_st(0)
			ass_FLD1
			ass_FSUBP_st(1)
			ass_FSQRT
			ass_FLD1
			ass_FXCH_st(1)
			ass_FPATAN
			ass_FLDPI
			ass_FLD1
			ass_FADD_st(0)
			ass_FDIVP_st(1)
			ass_FSUBRP_st(1)
			break;
		case AST_FUNCTION_ARCSECH: /* arcsec hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)aSech); /* aSech */
			break;
		case AST_FUNCTION_ARCSIN: /* arcsin */
			generate(c,child(AST,0));
			ass_FLD_st(0)
			ass_FMUL_st(0)
			ass_FLD1
			ass_FSUBRP_st(1)
			ass_FSQRT
			ass_FPATAN
			break;
		case AST_FUNCTION_ARCSINH: /* arcsin hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)aSinh); /* asinh */
			break;
		case AST_FUNCTION_ARCTAN: /* arctan */
			generate(c,child(AST,0));
			ass_FLD1
			ass_FPATAN
			break;
		case AST_FUNCTION_ARCTANH: /* arctan hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)aTanh); /* atanh */
			break;
		case AST_FUNCTION_CEILING: /* rounds value to next integer */
			generate(c,child(AST,0));
			ass_FLD_st(0)
			ass_FRNDINT
			ass_FXCH_st(1)
			ass_FCOMIP_st(1)
			ass_FLD1
			ass_FADD_st(1)
			ass_FCMOVBE_st(1)
			ass_FSTP_st(1)
			break;
		case AST_FUNCTION_COS: /* cosinus */
			generate(c,child(AST,0));
			ass_FCOS
			break;
		case AST_FUNCTION_COSH: /* cosinus hyperbilic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)cosh); /* cosh */
			break;
		case AST_FUNCTION_COT: /* cotangens = 1/tan */
			generate(c,child(AST,0));
			ass_FPTAN
			ass_FDIVRP_st(1)
			break;
		case AST_FUNCTION_COTH: /* coth = 1/tanh */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)tanh); /* tanh */
			ass_FLD1
			ass_FDIVRP_st(1)
			break;
		case AST_FUNCTION_CSC: /* cosec = 1/sinus */
			generate(c,child(AST,0));
			ass_FSIN
			ass_FLD1
			ass_FDIVRP_st(1)
			break;
		case AST_FUNCTION_CSCH: /* csch = 1/sinh */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)sinh); /* sinh */
			ass_FLD1
			ass_FDIVRP_st(1)
			break;
		case AST_FUNCTION_EXP: /* exp( value ) */
			generate(c,child(AST,0));
			ass_FLDL2E
			ass_FMULP_st(1)
			ass_FLD_st(0)
			ass_FRNDINT
			ass_FSUB_st(1)
			ass_FXCH_st(1)
			ass_F2XM1
			ass_FLD1
			ass_FADDP_st(1)
			ass_FSCALE
			ass_FSTP_st(1)
			break;
		case AST_FUNCTION_FACTORIAL:
			generate(c,child(AST,0));
			callMathFunction(c,(long long)factorial); /* factorial */
			break;
		case AST_FUNCTION_FLOOR: /* rounds value to next integer */
			generate(c,child(AST,0));
			ass_FLD_st(0)
			ass_FRNDINT
			ass_FXCH_st(1)
			ass_FCOMIP_st(1)
			ass_FLD1
			ass_FSUBR_st(1)
			ass_FCMOVNB_st(1)
			ass_FSTP_st(1)
			break;
		case AST_FUNCTION_LN: /* log_e */
			generate(c,child(AST,0));
			ass_FLDLN2
			ass_FXCH_st(1)
			ass_FYL2X
			break;
		case AST_FUNCTION_LOG: /* log_10 */
			generate(c,child(AST,0));
			ass_FLDLG2
			ass_FXCH_st(1)
			ass_FYL2X
			break;
		case AST_FUNCTION_PIECEWISE: /* USEFUL? NOT CORRECTLY IMPLEMENTED! */
			ass_FLDZ
			pushStorage(c); /* save result */
		    for ( i = 0 ; i < childnum-1 ; i = i + 2) {
				generate(c,child(AST,i+1));
				pushStorage(c); /* save old result */
				generate(c,child(AST,i));
				ass_FLD_mem popAddress(c); /* load value 1 */
				ass_FLDZ
				ass_FCOMIP_st(1)
				ass_FFREE_st(0)
				ass_FINCSTP
				ass_FLD_mem popAddress(c); /* load result */
				ass_FCMOVNE_st(1)
				pushStorage(c); /* save result */
				ass_FFREE_st(0)
				ass_FINCSTP
				}
			ass_FLD_mem popAddress(c); /* load result */
			break;
		/* AST_FUNCTION_POWER see AST_POWER */
		case AST_FUNCTION_ROOT:
			generate(c,child(AST,1));
			if(analyseFPU(child(AST,0)) >= 8) {
				pushStorage(c); /* save old result */
				generate(c,child(AST,0));
				ass_FLD1
				ass_FLD_mem popAddress(c); /* load value 1 */
				} else {
				generate(c,child(AST,0));
				ass_FXCH_st(1)
				ass_FLD1
				ass_FXCH_st(1)
				}
			ass_FYL2X
			ass_FDIVRP_st(1)
			ass_FLD_st(0)
			ass_FRNDINT
			ass_FSUB_st(1)
			ass_FXCH_st(1)
			ass_F2XM1
			ass_FLD1
			ass_FADDP_st(1)
			ass_FSCALE
			ass_FSTP_st(1)
			break;
		case AST_FUNCTION_SEC: /* sec = 1/cosinus */
			generate(c,child(AST,0));
			ass_FCOS
			ass_FLD1
			ass_FDIVRP_st(1)
			break;
		case AST_FUNCTION_SECH: /* sech = 1/cosh */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)cosh); /* cosh */
			ass_FLD1
			ass_FDIVRP_st(1)
			break;
		case AST_FUNCTION_SIN: /* sinus */
			generate(c,child(AST,0));
			ass_FSIN
			break;
		case AST_FUNCTION_SINH: /* sinus hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)sinh);
			break;
		case AST_FUNCTION_TAN: /* tangens */
			generate(c,child(AST,0));
			ass_FPTAN
			ass_FFREE_st(0)
			ass_FINCSTP
			break;

			case AST_FUNCTION_TANH: /* tangens hyperbolic */
			generate(c,child(AST,0));
			callMathFunction(c,(long long)tanh);
			break;

/* logical cases depends from 0.0 and 1.0 as initial values */
		case AST_LOGICAL_AND:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result */
				generate(c,child(AST,i));
				ass_FMUL_mem popAddress(c); /* reload old result and mult */
				}
			break;
		case AST_LOGICAL_NOT:
			generate(c,child(AST,0));
			ass_FLD1
			ass_FSUBRP_st(1)
			break;
		case AST_LOGICAL_OR: /* !(AND !values) */
			generate(c,child(AST,0));
			ass_FLD1
			ass_FSUBRP_st(1)
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result */
				generate(c,child(AST,i));
				ass_FLD1
				ass_FSUBRP_st(1)
				ass_FMUL_mem popAddress(c); /* reload old result and mult */
				}
			ass_FLD1
			ass_FSUBRP_st(1)
			break;
		case AST_LOGICAL_XOR:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result */
				generate(c,child(AST,i));
				ass_FLD_mem popAddress(c); /* load value 1 */
				ass_FSUBP_st(1)
				ass_FABS
				}
			break;
		case AST_RELATIONAL_EQ:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result to stack*/
				generate(c,child(AST,i));
				}
			ass_FLD1
			ass_FXCH_st(1)
			ass_FLDZ
			ass_FXCH_st(1)
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				ass_FLD_mem popAddress(c); /* load old value from stack */
				ass_FCOMIP_st(1)
				ass_FXCH_st(2)
				ass_FCMOVNE_st(1)
				ass_FXCH_st(2)
				}
			ass_FFREE_st(0)
			ass_FINCSTP
			ass_FFREE_st(0)
			ass_FINCSTP
			break;
		case AST_RELATIONAL_GEQ:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result to stack*/
				generate(c,child(AST,i));
				}
			ass_FLD1
			ass_FXCH_st(1)
			ass_FLDZ
			ass_FXCH_st(1)
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				ass_FLD_mem popAddress(c); /* load old value from stack */
				ass_FXCH_st(1)
				ass_FCOMIP_st(1)
				ass_FXCH_st(2)
				ass_FCMOVB_st(1)
				ass_FXCH_st(2)
				}
			ass_FFREE_st(0)
			ass_FINCSTP
			ass_FFREE_st(0)
			ass_FINCSTP
			break;
		case AST_RELATIONAL_GT:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result to stack*/
				generate(c,child(AST,i));
				}
			ass_FLD1
			ass_FXCH_st(1)
			ass_FLDZ
			ass_FXCH_st(1)
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				ass_FLD_mem popAddress(c); /* load old value from stack */
				ass_FXCH_st(1)
				ass_FCOMIP_st(1)
				ass_FXCH_st(2)
				ass_FCMOVBE_st(1)
				ass_FXCH_st(2)
				}
			ass_FFREE_st(0)
			ass_FINCSTP
			ass_FFREE_st(0)
			ass_FINCSTP
			break;
		case AST_RELATIONAL_LEQ:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result to stack*/
				generate(c,child(AST,i));
				}
			ass_FLD1
			ass_FXCH_st(1)
			ass_FLDZ
			ass_FXCH_st(1)
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				ass_FLD_mem popAddress(c); /* load old value from stack */
				ass_FXCH_st(1)
				ass_FCOMIP_st(1)
				ass_FXCH_st(2)
				ass_FCMOVNBE_st(1)
				ass_FXCH_st(2)
				}
			ass_FFREE_st(0)
			ass_FINCSTP
			ass_FFREE_st(0)
			ass_FINCSTP
			break;
		case AST_RELATIONAL_LT:
			generate(c,child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				pushStorage(c); /* save old result to stack*/
				generate(c,child(AST,i));
				}
			ass_FLD1
			ass_FXCH_st(1)
			ass_FLDZ
			ass_FXCH_st(1)
		    for ( i = 1 ; i < childnum ; i++) { /* reverse order of values */
				ass_FLD_mem popAddress(c); /* load old value from stack */
				ass_FXCH_st(1)
				ass_FCOMIP_st(1)
				ass_FXCH_st(2)
				ass_FCMOVNB_st(1)
				ass_FXCH_st(2)
				}
			ass_FFREE_st(0)
			ass_FINCSTP
			ass_FFREE_st(0)
			ass_FINCSTP
			break;

		default:
			printf("ERROR: Wrong AST_type\n");
			ass_FLDZ
			break;
		}
	}

/* analyses the abstract syntax tree according to code and stack size */
int analyse (directCode_t *c, ASTNode_t *AST) { /* returns the number of places it occupies of the FPU stack */
	
	int i, childnum, save, save1;
	ASTNodeType_t type;
	type = ASTNode_getType(AST);
	childnum = ASTNode_getNumChildren(AST);
	
	switch(type) {
		case AST_INTEGER:
		case AST_REAL:
		case AST_REAL_E:
		case AST_RATIONAL:
			c->storageSize++;
			c->codeSize += 6;
			return 1;
		case AST_NAME:
			c->codeSize += 13 + sizeof(void (*)());
			return 8;
		case AST_FUNCTION_DELAY: /* not implemented */
		case AST_LAMBDA: /* not implemented */
			c->codeSize += 2;
			return 1;
		case AST_NAME_TIME:
			c->codeSize += 8 + sizeof(void (*)());
			return 8;

		case AST_CONSTANT_E:
			c->codeSize += 18;
			return 2;
		case AST_CONSTANT_FALSE: /* 0.0 */
		case AST_CONSTANT_PI: /* pi */
		case AST_CONSTANT_TRUE: /* 1.0 */
			c->codeSize += 2;
			return 1;

		case AST_PLUS: /* add values */
		case AST_TIMES: /* mult values */
			save = analyse(c, child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				save1 = analyse(c, child(AST,i));
				if(save1 >= 8)
					c->codeSize += 12;
					else c->codeSize += 2;
				if(save < save1+1)
					save = save1 + 1;
				}
			return save;
		case AST_MINUS: /* sub values */
		    if ( childnum<2 ) {
				save = analyse(c,child(AST,0));
				c->codeSize += 2;
				} else {
				save = analyse(c,child(AST,0));
				save1 = analyse(c, child(AST,1));
				if(save1 >= 8)
					c->codeSize += 12;
					else c->codeSize += 2;
				if(save < save1+1)
					save = save1 + 1;
				}
			return save;
		case AST_DIVIDE: /* divide values */
			save = analyse(c,child(AST,0));
			save1 = analyse(c, child(AST,1));
			if(save1 >= 8)
				c->codeSize += 12;
				else c->codeSize += 2;
			if(save < save1+1)
				save = save1 + 1;
			return save;
		case AST_POWER: /* calculates value 1 to the power of value 2 */
		case AST_FUNCTION_POWER:
			save = analyse(c,child(AST,0));
			save1 = analyse(c, child(AST,1));
			if(save1 >= 8)
				c->codeSize += 14;
				else c->codeSize += 6;
			if(save < save1+1)
				save = save1 + 1;
			c->codeSize += 22;
			if(save < 3)
				save = 3;
			return save;

		case AST_FUNCTION: /* user defined function */
			if (UsrDefFunc == NULL) { /* no function defined */
				c->codeSize += 2;
				save = 1;
				} else {
				save = 8 + childnum;
				for (i = 0 ; i < childnum ; i++) { /* compute parameters and store them on the external stack */
					save1 = analyse(c,child(AST,i));
					if(save1 + i > save)
						save = save1 + i;
					}
				c->codeSize += 6*childnum;
				c->codeSize += 15 + 3*sizeof(void (*)());
				}
			return save;
		case AST_FUNCTION_ARCSEC: /* arccsc = arccos(1.0/value) =  pi/2.0 - arcsin(1.0/value) */
			c->codeSize += 2;
		case AST_FUNCTION_ARCCOS: /* arccos = pi/2.0 - arcsin  = pi/2.0 - arctan(value / sqrt(1.0-value^2)) */
		case AST_FUNCTION_EXP: /* exp( value ) */
			c->codeSize += 8;
		case AST_FUNCTION_ARCCOT: /* arccot = pi/2.0 - arctan */
			c->codeSize += 2;
		case AST_FUNCTION_ARCSIN: /* arcsin */
			save = analyse(c,child(AST,0));
			if(save < 3)
				save = 3;
			c->codeSize += 12;
			return save;
		case AST_FUNCTION_PIECEWISE: /* USEFUL? NOT CORRECTLY IMPLEMENTED! */
			save = 10;
			c->codeSize += 14;
		    for ( i = 0 ; i < childnum-1 ; i = i + 2) {
				save1 = analyse(c,child(AST,i+1));
				if(save < save1+1)
					save = save1+1;
				save1 = analyse(c,child(AST,i));
				if(save < save1+2)
					save = save1+2;
				c->codeSize += 38;
				}
			return save;
		/* AST_FUNCTION_POWER see AST_POWER */
		case AST_FUNCTION_ROOT:
			save = analyse(c,child(AST,1));
			save1 = analyse(c, child(AST,0));
			if(save1 >= 8)
				c->codeSize += 14;
				else c->codeSize += 6;
			if(save < save1+1)
				save = save1 + 1;
			c->codeSize += 22;
			if(save < 3)
				save = 3;
			return save;
		case AST_FUNCTION_ABS: /* absolut value */
		case AST_FUNCTION_COS: /* cosinus */
		case AST_FUNCTION_SIN: /* sinus */
			save = analyse(c,child(AST,0));
			c->codeSize += 2;
			return save;
		case AST_FUNCTION_CEILING: /* rounds value to next integer */
		case AST_FUNCTION_FLOOR: /* rounds value to next integer */
			c->codeSize += 2;
		case AST_FUNCTION_ARCCSC: /* arccsc = arcsin(1.0/value) = arctan(1.0 / sqrt(value^2 - 1.0)) */
			c->codeSize += 8;
		case AST_FUNCTION_CSC: /* cosec = 1/sinus */
		case AST_FUNCTION_LN: /* log_e */
		case AST_FUNCTION_LOG: /* log_10 */
		case AST_FUNCTION_SEC: /* sec = 1/cosinus */
		case AST_FUNCTION_TAN: /* tangens */
			c->codeSize += 2;
		case AST_FUNCTION_ARCTAN: /* arctan */
		case AST_FUNCTION_COT: /* cotangens = 1/tan */
			save = analyse(c,child(AST,0));
			if(save < 2)
				save = 2;
			c->codeSize += 4;
			return save;
		case AST_FUNCTION_COTH: /* coth = 1/tanh */
		case AST_FUNCTION_CSCH: /* csch = 1/sinh */
		case AST_FUNCTION_SECH: /* sech = 1/cosh */
			c->codeSize += 4;
		case AST_FUNCTION_ARCCOSH: /* arccos hyperbolic */
		case AST_FUNCTION_ARCCOTH: /* arccot hyperbolic */
		case AST_FUNCTION_ARCCSCH: /* arccsc hyperbolic */
		case AST_FUNCTION_ARCSECH: /* arcsec hyperbolic */
		case AST_FUNCTION_ARCSINH: /* arcsin hyperbolic */
		case AST_FUNCTION_ARCTANH: /* arctan hyperbolic */
		case AST_FUNCTION_COSH: /* cosinus hyperbilic */
		case AST_FUNCTION_FACTORIAL:
		case AST_FUNCTION_SINH: /* sinus hyperbolic */
		case AST_FUNCTION_TANH: /* tangens hyperbolic */
			save = analyse(c,child(AST,0));
			if(save < 8)
				save = 8;
			c->codeSize += 10 + sizeof(void (*)());
			return save;

/* logical cases depends from 0.0 and 1.0 as initial values */
		case AST_LOGICAL_XOR:
			c->codeSize += (childnum-1)*4;
		case AST_LOGICAL_AND:
			save = 9;
			save1 = analyse(c,child(AST,0));
			if(save < save1)
				save = save1;
			for ( i = 1 ; i < childnum ; i++) {
				save1 = analyse(c,child(AST,i));
				if(save < save1+1)
					save = save1+1;
				}
			c->codeSize += (childnum-1)*12;
			return save;
		case AST_LOGICAL_NOT:
			save = analyse(c,child(AST,0));
			if(save < 2)
				save = 2;
			c->codeSize += 4;
			return save;
		case AST_LOGICAL_OR: /* !(AND !values) */
			c->codeSize += 8;
		case AST_RELATIONAL_EQ:
			c->codeSize -= (childnum-1)*2;
		case AST_RELATIONAL_GEQ:
		case AST_RELATIONAL_GT:
		case AST_RELATIONAL_LEQ:
		case AST_RELATIONAL_LT:
			save = 7 + childnum;
			save1 = analyse(c,child(AST,0));
			if(save < save1)
				save = save1;
			for ( i = 1 ; i < childnum ; i++) {
				save1 = analyse(c,child(AST,i));
				if(save < save1 + i)
					save = save1 + i;
				}
			c->codeSize += 16;
			c->codeSize += (childnum-1)*22;
			return save;

		default:
			printf("ERROR: Wrong AST_type\n");
			c->codeSize += 2;
			return 1;
		}

	return 1;
	}

/* analyses the place needed in the FPU stack - used in operations with more than one parameters (e.g. plus) */
int analyseFPU (ASTNode_t *AST) { /* returns the number of places it occupies of the FPU stack */
	
	int i, childnum, save, save1;
	ASTNodeType_t type;
	type = ASTNode_getType(AST);
	childnum = ASTNode_getNumChildren(AST);
	
	switch(type) {
		case AST_INTEGER:
		case AST_REAL:
		case AST_REAL_E:
		case AST_RATIONAL:
		case AST_FUNCTION_DELAY: /* not implemented */
		case AST_LAMBDA: /* not implemented */
		case AST_CONSTANT_FALSE: /* 0.0 */
		case AST_CONSTANT_PI: /* pi */
		case AST_CONSTANT_TRUE: /* 1.0 */
			return 1;
		case AST_NAME:
		case AST_NAME_TIME:
			return 8;
		case AST_CONSTANT_E:
			return 2;

		case AST_PLUS: /* add values */
		case AST_TIMES: /* mult values */
			save = analyseFPU(child(AST,0));
		    for ( i = 1 ; i < childnum ; i++) {
				save1 = analyseFPU(child(AST,i));
				if(save < save1+1)
					save = save1 + 1;
				}
			return save;
		case AST_MINUS: /* sub values */
		    if ( childnum<2 ) {
				save = analyseFPU(child(AST,0));
				} else {
				save = analyseFPU(child(AST,0));
				save1 = analyseFPU(child(AST,1));
				if(save < save1+1)
					save = save1 + 1;
				}
			return save;
		case AST_DIVIDE: /* divide values */
			save = analyseFPU(child(AST,0));
			save1 = analyseFPU(child(AST,1));
			if(save < save1+1)
				save = save1 + 1;
			return save;
		case AST_POWER: /* calculates value 1 to the power of value 2 */
		case AST_FUNCTION_POWER:
			save = analyseFPU(child(AST,0));
			save1 = analyseFPU(child(AST,1));
			if(save < save1+1)
				save = save1 + 1;
			if(save < 3)
				save = 3;
			return save;

		case AST_FUNCTION: /* user defined function */
			if (UsrDefFunc == NULL) { /* no function defined */
				save = 1;
				} else {
				save = 8 + childnum;
				for (i = 0 ; i < childnum ; i++) { /* compute parameters and store them on the external stack */
					save1 = analyseFPU(child(AST,i));
					if(save1 + i > save)
						save = save1 + i;
					}
				}
			return save;
		case AST_FUNCTION_ARCSEC: /* arccsc = arccos(1.0/value) =  pi/2.0 - arcsin(1.0/value) */
		case AST_FUNCTION_ARCCOS: /* arccos = pi/2.0 - arcsin  = pi/2.0 - arctan(value / sqrt(1.0-value^2)) */
		case AST_FUNCTION_EXP: /* exp( value ) */
		case AST_FUNCTION_ARCCOT: /* arccot = pi/2.0 - arctan */
		case AST_FUNCTION_ARCSIN: /* arcsin */
			save = analyseFPU(child(AST,0));
			if(save < 3)
				save = 3;
			return save;
		case AST_FUNCTION_ABS: /* absolut value */
		case AST_FUNCTION_COS: /* cosinus */
		case AST_FUNCTION_SIN: /* sinus */
			return analyseFPU(child(AST,0));
		case AST_FUNCTION_PIECEWISE: /* USEFUL? NOT CORRECTLY IMPLEMENTED! */
			save = 10;
		    for ( i = 0 ; i < childnum-1 ; i = i + 2) {
				save1 = analyseFPU(child(AST,i+1));
				if(save < save1+1)
					save = save1+1;
				save1 = analyseFPU(child(AST,i));
				if(save < save1+2)
					save = save1+2;
				}
			return save;
		/* AST_FUNCTION_POWER see AST_POWER */
		case AST_FUNCTION_ROOT:
			save = analyseFPU(child(AST,1));
			save1 = analyseFPU(child(AST,0));
			if(save < save1+1)
				save = save1 + 1;
			if(save < 3)
				save = 3;
			return save;
		case AST_FUNCTION_ARCCSC: /* arccsc = arcsin(1.0/value) = arctan(1.0 / sqrt(value^2 - 1.0)) */
		case AST_FUNCTION_CSC: /* cosec = 1/sinus */
		case AST_FUNCTION_LN: /* log_e */
		case AST_FUNCTION_LOG: /* log_10 */
		case AST_FUNCTION_SEC: /* sec = 1/cosinus */
		case AST_FUNCTION_ARCTAN: /* arctan */
		case AST_FUNCTION_CEILING: /* rounds value to next integer */
		case AST_FUNCTION_COT: /* cotangens = 1/tan */
		case AST_FUNCTION_FLOOR: /* rounds value to next lower integer */
		case AST_FUNCTION_TAN: /* tangens */
			save = analyseFPU(child(AST,0));
			if(save < 2)
				save = 2;
			return save;
		case AST_FUNCTION_COTH: /* coth = 1/tanh */
		case AST_FUNCTION_CSCH: /* csch = 1/sinh */
		case AST_FUNCTION_SECH: /* sech = 1/cosh */
		case AST_FUNCTION_ARCCOSH: /* arccos hyperbolic */
		case AST_FUNCTION_ARCCOTH: /* arccot hyperbolic */
		case AST_FUNCTION_ARCCSCH: /* arccsc hyperbolic */
		case AST_FUNCTION_ARCSECH: /* arcsec hyperbolic */
		case AST_FUNCTION_ARCSINH: /* arcsin hyperbolic */
		case AST_FUNCTION_ARCTANH: /* arctan hyperbolic */
		case AST_FUNCTION_COSH: /* cosinus hyperbilic */
		case AST_FUNCTION_FACTORIAL:
		case AST_FUNCTION_SINH: /* sinus hyperbolic */
		case AST_FUNCTION_TANH: /* tangens hyperbolic */
			save = analyseFPU(child(AST,0));
			if(save < 8)
				save = 8;
			return save;

/* logical cases depends from 0.0 and 1.0 as initial values */
		case AST_LOGICAL_XOR:
		case AST_LOGICAL_AND:
			save = 9;
			save1 = analyseFPU(child(AST,0));
			if(save < save1)
				save = save1;
			for ( i = 1 ; i < childnum ; i++) {
				save1 = analyseFPU(child(AST,i));
				if(save < save1+1)
					save = save1+1;
				}
			return save;
		case AST_LOGICAL_NOT:
			save = analyseFPU(child(AST,0));
			if(save < 2)
				save = 2;
			return save;
		case AST_LOGICAL_OR: /* !(AND !values) */
		case AST_RELATIONAL_EQ:
		case AST_RELATIONAL_GEQ:
		case AST_RELATIONAL_GT:
		case AST_RELATIONAL_LEQ:
		case AST_RELATIONAL_LT:
			save = 7 + childnum;
			save1 = analyseFPU(child(AST,0));
			if(save < save1)
				save = save1;
			for ( i = 1 ; i < childnum ; i++) {
				save1 = analyseFPU(child(AST,i));
				if(save < save1 + i)
					save = save1 + i;
				}
			return save;

		default:
			printf("ERROR: Wrong AST_type\n");
			return 1;
		}

	return 1;
	}

/* generates the basic elements of the function - CALL THIS FUNCTION TO GENERATE THE FUNCTION */
void generateFunction(directCode_t *code, ASTNode_t *AST) {

	initCode(code, AST); /* dynamic allocation of necessary memory */

	addByte(code, 0x55); /* PUSH EBP */
	addByte(code, 0x89); addByte(code, 0xe5); /* MOV EBP, ESP */
	generate(code, AST);
	addByte(code, 0x5d); /* POP EBP */
	addByte(code, 0xc3); /* RETN */
	}

/* disallocates the functions arrays */
void destructFunction(directCode_t *code) {

	code->codeSize = 0;
	code->FPUstackSize = 0;
	code->storageSize = 0;
	code->codePosition = 0;
	code->FPUstackPosition = 0;
	code->storagePosition = 0;
	free(code->storage);
	free(code->FPUstack);
	free(code->prog);
	}
