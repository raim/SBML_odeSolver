/*
  Last changed Time-stamp: <2005-12-15 20:33:39 raim>
  $Id: solverError.h,v 1.13 2005/12/15 19:54:06 raimc Exp $ 
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
 *
 *     Andrew Finney
 *
 * Contributor(s):
 *     Rainer Machne
 */

#ifndef _SOLVERERROR_H_
#define _SOLVERERROR_H_

#include <stdarg.h>
#include <stddef.h>
#include "sbmlsolver/exportdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

  /** error codes.
      codes < 0 reserved for CVODE 
      codes 0 throu 9999 reserved for LibSBML */
typedef enum errorCode
{

    /** 1XXXX - conversion to ode model failures in odeConstruct.c */
    SOLVER_ERROR_ODE_COULD_NOT_BE_CONSTRUCTED_FOR_SPECIES  = 10000,
    SOLVER_ERROR_THE_MODEL_CONTAINS_EVENTS = 10001,
    SOLVER_ERROR_THE_MODEL_CONTAINS_ALGEBRAIC_RULES = 10002,
    SOLVER_ERROR_ODE_MODEL_COULD_NOT_BE_CONSTRUCTED = 10003,
    SOLVER_ERROR_NO_KINETIC_LAW_FOUND_FOR_REACTION = 10004,
    SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED = 10005,
    SOLVER_ERROR_MODEL_NOT_SIMPLIFIED = 10006,
    SOLVER_ERROR_ENTRIES_OF_THE_PARAMETRIC_MATRIX_COULD_NOT_BE_CONSTRUCTED = 10007,

    /** 1xx30 - SBML input model failures in sbml.c */
    SOLVER_ERROR_MAKE_SURE_SCHEMA_IS_ON_PATH = 10030,
    SOLVER_ERROR_CANNOT_PARSE_MODEL = 10031,
    SOLVER_ERROR_DOCUMENTLEVEL_ONE = 100032,
     
    /** 1XX5X - Graph Drawing Errors  in drawGraph.c */
    SOLVER_ERROR_NO_GRAPHVIZ = 10050,
    /** 1X1XX - Wrong Input Settings */
    SOLVER_ERROR_INTEGRATOR_SETTINGS = 10100,
    SOLVER_ERROR_VARY_SETTINGS = 10101,
      
    /** 2XXXX - Integration Failures in integratorInstance.c */
    SOLVER_ERROR_INTEGRATION_NOT_SUCCESSFUL = 20000,
    SOLVER_ERROR_EVENT_TRIGGER_FIRED = 20001,
    SOLVER_ERROR_CVODE_MALLOC_FAILED = 20002,

    /** 2X1XX - AST Processing Failures in processAST.c */
    /** AST evaluation in evaluateAST */
    SOLVER_ERROR_AST_UNKNOWN_NODE_TYPE = 20100,
    SOLVER_ERROR_AST_UNKNOWN_FAILURE = 20101,
    SOLVER_ERROR_AST_EVALUATION_FAILED_MISSING_VALUE = 20102,
    SOLVER_ERROR_AST_EVALUATION_FAILED_DELAY = 20103,
    SOLVER_ERROR_AST_EVALUATION_FAILED_LAMBDA = 20104,
    SOLVER_ERROR_AST_EVALUATION_FAILED_FUNCTION = 20105,
    SOLVER_ERROR_AST_EVALUATION_FAILED_FLOAT_FACTORIAL = 20106,
    /** AST differentiation in differentiateAST */
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_CONSTANT = 20110,
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_OPERATOR = 20111,
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_LAMBDA = 20112,
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_DELAY = 20114,
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_FACTORIAL = 20115,
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_PIECEWISE = 20117,
    SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_LOGICAL_OR_RELATIONAL = 20118,
    
    /** 2X2XX - Result Writing Failures */
    SOLVER_ERROR_CVODE_RESULTS_FAILED = 20201,
    SOLVER_ERROR_SBML_RESULTS_FAILED = 20202,
    
    /** 2X5XX - Integration Messages in integratorInstance.c */
    SOLVER_MESSAGE_RERUN_WITH_OR_WO_JACOBIAN = 20500,
    SOLVER_MESSAGE_STEADYSTATE_FOUND = 20501,
 
    /** 3XXXX - Memory Exhaustion; general */
    SOLVER_ERROR_NO_MORE_MEMORY_AVAILABLE = 30000,

    /** 4XXXX - assorted API errors */
    SOLVER_ERROR_SYMBOL_IS_NOT_IN_MODEL = 40000,
    SOLVER_ERROR_ATTEMPTING_TO_COPY_VARIABLE_STATE_BETWEEN_INSTANCES_OF_DIFFERENT_MODELS = 40001

} errorCode_t;

  /** error types */
typedef enum errorType
{
    FATAL_ERROR_TYPE = 0,
    ERROR_ERROR_TYPE = 1,
    WARNING_ERROR_TYPE = 2,
    NUMBER_OF_ERROR_TYPES = 3
} errorType_t;

#define RETURN_ON_ERRORS_WITH(x) \
{if (SolverError_getNum(ERROR_ERROR_TYPE) || SolverError_getNum(FATAL_ERROR_TYPE)) return (x); }

#define RETURN_ON_FATALS_WITH(x) \
{if (SolverError_getNum(FATAL_ERROR_TYPE)) return (x); }

#define ASSIGN_NEW_MEMORY_BLOCK(_ref, _num, _type, _return) \
{ (_ref) = (_type *)SolverError_calloc(_num, sizeof(_type)); RETURN_ON_FATALS_WITH(_return) }

#define ASSIGN_NEW_MEMORY(_ref, _type, _return) ASSIGN_NEW_MEMORY_BLOCK(_ref, 1, _type, _return)

/* get number of stored errors  of given type */
SBML_ODESOLVER_API int SolverError_getNum(errorType_t); 

/* get a stored error message */
SBML_ODESOLVER_API char * SolverError_getMessage(errorType_t, int errorNum);

/* get error code */
SBML_ODESOLVER_API errorCode_t SolverError_getCode(errorType_t, int errorNum);

/* get error code of last error stored of given type */
SBML_ODESOLVER_API errorCode_t SolverError_getLastCode(errorType_t);

/* empty error store */
SBML_ODESOLVER_API void SolverError_clear();

/* create an error */
SBML_ODESOLVER_API void SolverError_error(errorType_t, errorCode_t, char *format, ...);

/* exit the program if errors or fatals have been created. */
SBML_ODESOLVER_API void SolverError_haltOnErrors();

/* write all errors and warnings to standard error */
SBML_ODESOLVER_API void SolverError_dump();

/* write all errors and warnings to a string (owned by caller unless SolverError_isMemoryExhausted()) */
SBML_ODESOLVER_API char *SolverError_dumpToString();

/* free string returned by SolverError_dumpToString */
SBML_ODESOLVER_API void SolverError_freeDumpString(char *);

/* write all errors and warnings to standard error and then empty error store*/
SBML_ODESOLVER_API void SolverError_dumpAndClearErrors();

/* allocated memory and sets error if it fails */
SBML_ODESOLVER_API void *SolverError_calloc(size_t num, size_t size);

/* returns 1 if memory has been exhausted 0 otherwise */
SBML_ODESOLVER_API int SolverError_isMemoryExhausted();

#ifdef __cplusplus
}
#endif

#endif 
/* _SOLVERERROR_H_ */

