/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/cvodeData.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/solverError.h"

static double aCosh(double x)
{
  return log(x + (sqrt(x - 1) * sqrt(x + 1)));
}

static double aSinh(double x)
{
  return log(x + sqrt((x * x) + 1));
}

static double aTanh(double x)
{
  return (log(1 + x) - log(1-x))/2 ;
}

/* function pointer to user defined function */
static double (*UsrDefFunc)(char*, int, double*) = NULL;

/** 
 * Sets function pointer for  user defined function to udf

 Takes a function pointer as an argument. This function udf is
 called in evaluateAST to interface external data that can be
 supplied by a calling applications. It must return a double value
 for the current time of an integration run.
   
 ARGUMENTS TO THE PASSED FUNCTION:

 char *:   must be the name of a function used in any formula in the
 SBML file, but not defined via an SBML Function
 Definition in the same SBML model

 int:      is the number of arguments the SBML function takes, which is
 also the size of the following double array

 double *: is an array that is filled by evaluateAST to supply the
 external function with current values of the arguments of
 the respective function the SBML model

 POSSIBLE APPLICATIONS:
   
 * Setting a set of udfs could later also be used to enhance
 integration performance by using SBML sboTerms in kinetic laws,
 i.e. for hard-coded kinetic law evaluation!

 * The function might also be useful for introducing stochastic
 effects to the ODE system. An external function could call a random
 number generator.

 Sorry, if above description is confusing, because of two different
 types of `function'. Basically, it allows a hard-coded definition
 of an otherwise undefined function in the SBML file. While this would
 be invalid SBML (!), it allows a model to access external data.
 We will in one of the next releases provide an example of a simple
 interpolation function, that takes only the current time as argument
 and interpolates a value for the current time from an external time
 series data set.
*/

SBML_ODESOLVER_API void setUserDefinedFunction(double(*udf)(char*, int, double*))
{
  UsrDefFunc = udf;
}

/* ------------------------------------------------------------------------ */

/** Evaluates the passed formula n by a simple recursion and returns
    the result as a double value.

    Variable names are searched in passed cvodeData, which is an
    SBML_odeSolver specific data structure, containing current values of
    indexed AST. See files cvodedata.c/h  and odeConstruct.c to learn
    how this structure is built.
  
    Not implemented:
    - Complex numbers and/or checking for domains of trigonometric and root
    functions.
    - Checking for precision and rounding errors.
    - SBML model specific function DELAY 
    - SBML Function Definitions
    - LAMBDA (only used in SBML Function Definitions)
  
    The node type AST_DELAY defaults to 0.
    The SBML DELAY function and unknown functions (SBML user-defined
    functions) use the value of the left child (first argument to
    function) or 0 if the node has no children and produce an errorMessage
    via SOSlibs Error Management System.
*/

SBML_ODESOLVER_API double evaluateAST(ASTNode_t *n, cvodeData_t *data)
{
  int i, j, childnum;
  int found, datafound;
  int true;
  time_series_t *ts=data->model->time_series;
  double findtol=1e-5;
 
  ASTNodeType_t type;
  /* ASTNode_t **child; */

  /* value* are for intermediate values. */
  double value1, value2, value3, result;

  if ( n == NULL )
  {
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_AST_UNKNOWN_NODE_TYPE,
		      "evaluateAST: empty Abstract Syntax Tree (AST).");
    return (0);
  }
  if ( ASTNode_isUnknown(n) )
  {
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_AST_UNKNOWN_NODE_TYPE,
		      "evaluateAST: unknown ASTNode type");
  }
  result = 0;

  childnum = ASTNode_getNumChildren(n);
  type = ASTNode_getType(n);
  switch(type)
  {
  case AST_INTEGER:
    result = (double) ASTNode_getInteger(n);      
    break;
  case AST_REAL:
    result = ASTNode_getReal(n);
    break;
  case AST_REAL_E:
    result = ASTNode_getReal(n);
    break;      
  case AST_RATIONAL:
    result = ASTNode_getReal(n) ;
    break;
	
  case AST_NAME:
    /** VARIABLES:

    find the value of the variable in the data->value
    array. SOSlib's extension to libSBML's AST allows to add the
    index of the variable in this array to AST_NAME
    (ASTIndexName). If the ASTNode is not indexed, its array
    index is searched via the data->model->names array, which
    corresponds to the data->value array. For nodes with name
    `Time' or `time' the data->currenttime is returned.  If no
    value is found a fatal error is produced. */
    found = 0;
    if ( ASTNode_isSetIndex(n) )
    {
      if ( ASTNode_isSetData(n) )
      {

        /* if continuous data is observed, obtain interpolated result */  
        if ( (data->model->discrete_observation_data != 1) || (data->model->compute_vector_v != 1) )
	{
	  result = call(ASTNode_getIndex(n),
			data->currenttime, ts);

	}
	else  /* if discrete data is observed, simply obtain value from time_series */
	{
          datafound = 0;
          i = data->TimeSeriesIndex;
        
	    if ( fabs(data->currenttime - ts->time[i]) < findtol )
	    {    
	      result = ts->data[ASTNode_getIndex(n)][i];
              datafound++;
	    }
	
          if ( datafound != 1)
	  {	            
	    SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_EVALUATION_FAILED_DISCRETE_DATA,
			"use of discrete time series data failed; none or several time points matching current time");
	    result = 0;
           /*  break;  */
	  }
          else found = 1;

	}
      }
      else
      {
	/* majority case: just return the
	   according value from data->values
	   from the index stored by SOSlib
	   ASTIndexNameNode sub-class of libSBML's ASTNode */
	result = data->value[ASTNode_getIndex(n)];
  
      }
      
       found++;
    }

    if ( found == 0 )
    {
      for ( j=0; j<data->nvalues; j++ )
      {
	if ( (strcmp(ASTNode_getName(n),data->model->names[j]) == 0) )
	{
	  
	  result = data->value[j];
	  found++;
	}
      }
    }

    if ( found == 0 )
    {
      SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_EVALUATION_FAILED_MISSING_VALUE,
			"No value found for AST_NAME %s . Defaults to Zero "
			"to avoid program crash", ASTNode_getName(n));
      result = 0;
    }
    break;

  case AST_FUNCTION_DELAY:
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_AST_EVALUATION_FAILED_DELAY,
		      "Solving ODEs with Delay is not implemented. "
		      "Defaults to 0 to avoid program crash");
    result = 0.0;
    break;
  case AST_NAME_TIME:
    result = (double) data->currenttime;
    break;
	
  case AST_CONSTANT_E:
    /** exp(1) is used to adjust exponentiale to machine precision */
    result = exp(1.);
    break;
  case AST_CONSTANT_FALSE:
    result = 0.0;
    break;
  case AST_CONSTANT_PI:
    /** pi = 4 * atan 1  is used to adjust Pi to machine precision */
    result = 4.*atan(1.);
    break;
  case AST_CONSTANT_TRUE:
    result = 1.0;
    break;

  case AST_PLUS:
    result = 0.0;
    for ( i=0; i<childnum; i++) 
      result += evaluateAST(child(n,i),data);   
    break;      
  case AST_MINUS:
    if ( childnum<2 )
      result = - (evaluateAST(child(n,0),data));
    else
      result = evaluateAST(child(n,0),data) - evaluateAST(child(n,1),data);
    break;
  case AST_TIMES:
    result = 1.0;
    for ( i=0; i<childnum; i++)
    {
      result *= evaluateAST(child(n,i),data);
      if (result == 0.0) break; /* small optimization by skipping the rest of children */
    }
    break;
  case AST_DIVIDE:
    result = evaluateAST(child(n,0),data) / evaluateAST(child(n,1),data);
    break;
  case AST_POWER:
    result = pow(evaluateAST(child(n,0),data),evaluateAST(child(n,1),data));
    break;
  case AST_LAMBDA:
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_AST_EVALUATION_FAILED_LAMBDA,
		      "Lambda can only be used in SBML function definitions."
		      " Defaults to 0 to avoid program crash");
    result = 0.0;
    break;
    /** FUNCTIONS: */
  case AST_FUNCTION:
    /**  Evaluate external functions, if it was set with
	 setUserDefinedFunction */      
    if ( UsrDefFunc == NULL )
    {
      SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_EVALUATION_FAILED_FUNCTION,
			"The function %s() has not been defined "
			"in the SBML input model or as an externally "
			"supplied function. Defaults to 0 to avoid "
			"program crash",
			ASTNode_getName(n));
      result = 0.0;
    }
    else
    {
      double *func_vals = NULL;
      ASSIGN_NEW_MEMORY_BLOCK(func_vals, childnum+1, double, 0);
      for ( i=0; i<childnum; i++ ) 
	func_vals[i] = evaluateAST(child(n,i), data);      
      result = UsrDefFunc((char *)ASTNode_getName(n), childnum, func_vals);
      free(func_vals);
    }
    break;
  case AST_FUNCTION_ABS:
    result = fabs(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCCOS:
    result = acos(evaluateAST(child(n,0),data)) ;
    break;
  case AST_FUNCTION_ARCCOSH:
    result = aCosh(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCCOT:
    /** arccot x =  arctan (1 / x) */
    result = atan(1./ evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCCOTH:
    /** arccoth x = 1/2 * ln((x+1)/(x-1)) */
    value1 = evaluateAST(child(n,0),data);
    result = log((value1 + 1) / (value1 - 1))/2;
    break;
  case AST_FUNCTION_ARCCSC:
    /** arccsc(x) = Arctan(1 / sqrt((x - 1)(x + 1))) */
    value1 = evaluateAST(child(n,0),data);
    result = atan(1/sqrt((value1-1)*(value1+1)));
    break;
  case AST_FUNCTION_ARCCSCH:
    /** arccsch(x) = ln((1/x) + sqrt((1/(x*x)) + 1)) */
    value1 = evaluateAST(child(n,0),data);
    result = log(1/value1 + sqrt(1/(value1*value1)+1));
    break;
  case AST_FUNCTION_ARCSEC:
    /** arcsec(x) = arccos(1/x) */
    result = acos(1/evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCSECH:
    /* arcsech(x) = arccosh(1/x) */
    result = aCosh( 1. /  evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCSIN:
    result = asin(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCSINH:
    result = aSinh(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCTAN:
    result = atan(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_ARCTANH:
    result = aTanh(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_CEILING:
    result = ceil(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_COS:
    result = cos(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_COSH:
    result = cosh(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_COT:
    /** cot x = 1 / tan x */
    result = (1./tan(evaluateAST(child(n,0),data)));
    break;
  case AST_FUNCTION_COTH:
    /** coth x = cosh x / sinh x */
    result = 1/tanh(evaluateAST(child(n,0),data));
    break;  
  case AST_FUNCTION_CSC:
    /** csc x = 1 / sin x */
    result = (1./sin(evaluateAST(child(n,0),data)));
    break;
  case AST_FUNCTION_CSCH:
    /** csch x = 1 / sinh x  */
    result = (1./sinh(evaluateAST(child(n,0),data)));
    break;
  case AST_FUNCTION_EXP:
    result = exp(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_FACTORIAL:
    {
      int j;
      value1 = evaluateAST(child(n,0),data);
      j = floor(value1);
      if ( value1 != j )
	SolverError_error(FATAL_ERROR_TYPE,
			  SOLVER_ERROR_AST_EVALUATION_FAILED_FLOAT_FACTORIAL,
			  "The factorial is only implemented."
			  "for integer values. The floor value of the "
			  "passed float is used for calculation!");

      for(result=1;j>1;--j)
	result *= j;
    }
    break;
  case AST_FUNCTION_FLOOR:
    result = floor(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_LN:
    result = log(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_LOG:
    /** log(x,y) = log10(y)/log10(x) (where x is the base)  */
    result = log10(evaluateAST(child(n,1),data)) /
      log10(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_PIECEWISE:
    /** Piecewise: */
    found = 0;
    /** Go through n pieces with 2 AST children for each piece, */
    for ( i=0; i<(childnum-1); i=i+2 )
    {
      if ( evaluateAST(child(n, i+1), data) )
      {
	found++;
	result = evaluateAST(child(n, i), data);
      }
    }
    /** odd number of AST children: if no piece was true, otherwise remains */
    /* i should be equal to childnum for even number piecewise AST
       and equal to childnum-1 for odd numbered piecewise ASTs */
    if ( i == childnum-1 && found == 0 )
    {
      found++;
      result = evaluateAST(child(n, i), data);
    }
    if ( found == 0 )
      SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_EVALUATION_FAILED_PIECEWISE,
			"Piecewise function failed; no true piece");
    if ( found > 1 )
      SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_EVALUATION_FAILED_PIECEWISE,
			"Piecewise function failed; several true pieces");
    break;
  case AST_FUNCTION_POWER:
    result = pow(evaluateAST(child(n,0),data),evaluateAST(child(n,1),data));
    break;
  case AST_FUNCTION_ROOT:
    /*!!! ALSO do this in compiled code */
    value1 = evaluateAST(child(n,1),data);
    value2 = evaluateAST(child(n,0),data);
    value3 = floor(value2);
    /* for odd root degrees, negative numbers are OK */
    if ( value2 == value3 ) /* check whether degree is integer */
    {
      if ( (value1 < 0) && ((int)value2 % 2 != 0) )
	result = - pow(fabs(value1), 1./value2);
      else
	result = pow(value1, 1./value2);	
    }
    else
      result = pow(value1, 1./value2);
    break;
  case AST_FUNCTION_SEC:
    /** sec x = 1 / cos x */
    result = 1./cos(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_SECH:
    /** sech x = 1 / cosh x */
    result = 1./cosh(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_SIN:
    result = sin(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_SINH:
    result = sinh(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_TAN:
    result = tan(evaluateAST(child(n,0),data));
    break;
  case AST_FUNCTION_TANH:
    result = tanh(evaluateAST(child(n,0),data));
    break;

  case AST_LOGICAL_AND:
    /** AND: all children must be true */
    true = 0;
    for ( i=0; i<childnum; i++ ) true += evaluateAST(child(n,i),data);
    if ( true == childnum ) result = 1.0;
    else result = 0.0;
    break;
  case AST_LOGICAL_NOT:
    result = (double) (!(evaluateAST(child(n,0),data)));
    break;
  case AST_LOGICAL_OR:
    /** OR: at least one child must be true */
    true = 0;
    for ( i=0; i<childnum; i++ ) true += evaluateAST(child(n,i),data);    
    if ( true > 0 ) result = 1.0;
    else result = 0.0;
    break;
  case AST_LOGICAL_XOR:
    /* n-ary: true if an odd number of children is true */
    true = 0;
    for ( i=0; i<childnum; i++ ) true += evaluateAST(child(n,i),data);    
    if ( true % 2 != 0 ) result = 1.0;
    else result = 0.0;
    break;
    /* !!! check n-ary definitions for relational operators !!! */
  case AST_RELATIONAL_EQ:
    /** n-ary: all children must be equal */
    result = 1.0;
    if (childnum <= 1) break;
    value1 = evaluateAST(child(n,0),data);
    for ( i=1; i<childnum; i++ )
      if ( value1 != evaluateAST(child(n,i),data) )
      {
        result = 0.0;
        break;
      }
    break;
  case AST_RELATIONAL_GEQ:
    /** n-ary: each child must be greater than or equal to the following */
    result = 1.0;
    if (childnum <= 1) break;
    value2 = evaluateAST(child(n,0),data);
    for ( i=1; i<childnum; i++ )
    {
      value1 = value2;
      value2 = evaluateAST(child(n,i),data);
      if (value1 < value2)
      {
        result = 0.0;
        break;
      }
    }
    break;
  case AST_RELATIONAL_GT:
    /** n-ary: each child must be greater than the following */
    result = 1.0;
    if (childnum <= 1) break;
    value2 = evaluateAST(child(n,0),data);
    for ( i=1; i<childnum; i++ )
    {
      value1 = value2;
      value2 = evaluateAST(child(n,i),data);
      if (value1 <= value2)
      {
        result = 0.0;
        break;
      }
    }
    break;
  case AST_RELATIONAL_LEQ:
    /** n-ary: each child must be lower than or equal to the following */
    result = 1.0;
    if (childnum <= 1) break;
    value2 = evaluateAST(child(n,0),data);
    for ( i=1; i<childnum; i++ )
    {
      value1 = value2;
      value2 = evaluateAST(child(n,i),data);
      if (value1 > value2)
      {
        result = 0.0;
        break;
      }
    }
    break;
  case AST_RELATIONAL_LT :
    /* n-ary: each child must be lower than the following */
    result = 1.0;
    if (childnum <= 1) break;
    value2 = evaluateAST(child(n,0),data);
    for ( i=1; i<childnum; i++ )
    {
      value1 = value2;
      value2 = evaluateAST(child(n,i),data);
      if (value1 >= value2)
      {
        result = 0.0;
        break;
      }
    }
    break;
  case AST_RELATIONAL_NEQ:
    /* binary "not equal" */
    result = (evaluateAST(child(n, 0), data) != evaluateAST(child(n, 1), data));
    break;
  default:
    SolverError_error(FATAL_ERROR_TYPE,
                      SOLVER_ERROR_AST_UNKNOWN_NODE_TYPE,
                      "evaluateAST: unknown ASTNode type: %d", type);
    result = 0;
    break;
  }
  
  return result;
}
