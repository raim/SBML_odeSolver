/*
  Last changed Time-stamp: <2007-09-25 17:20:17 raim>
  $Id: odeModel.h,v 1.41 2008/01/28 19:25:27 stefan_tbi Exp $ 
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

#ifndef _ODEMODEL_H_
#define _ODEMODEL_H_

typedef struct odeModel odeModel_t;
typedef struct odeSense odeSense_t;
typedef struct objFunc objFunc_t;
typedef int (*EventFn)(void *, int *); /* RM: replaced cvodeData_t
					    pointer with void pointer
					    because of dependency
					    problems */

#include <cvodes/cvodes.h>
#include <cvodes/cvodes_dense.h>
#include <sbml/SBMLTypes.h>

#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/interpol.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/compiler.h"
#include "sbmlsolver/arithmeticCompiler.h"
#include "sbmlsolver/variableIndex.h"

  /** The internal ODE Model as constructed in odeModel.c from an SBML
      input file, that only contains rate rules (constructed from
      reaction network in odeConstruct.c)
  */
  struct odeModel
  {
    SBMLDocument_t *d; /**< not-NULL only if the odeModel was directly
			  created from file */
    Model_t *m;        /**< the input SBML reaction network */
    Model_t *simple;   /**< the derived SBML with rate rules */
    double *values;    /**< input initial conditions and parameter values
			  (alternative to SBML!) */

    /** All names, i.e. ODE variables, assigned parameters, and constant
	parameters */
    char **names; 

    int neq;    /**< number of ODEs */
    int nalg;   /**< number of algebraic rules */ 
    int nass;   /**< number of assigned variables (nass) */
    int nconst; /**< number of constant parameters */

    /** piecewise expressions as well as events can lead to problems
        and will result in CVODES running in CV_NORMAL_TSTOP mode
        which avoids that the solver internally integrates beyond the
        next requested timestep */
    int nevents;     /**< number of model events */
    int npiecewise;  /**< number of piecewise expression in equations */

    /** Assigned variables: stores species, compartments and parameters,
	that are set by an assignment rule */
    ASTNode_t **assignment;
    directCode_t **assignmentcode;

    /** Algebraic Rules (constraints) as used for DAE systems */
    ASTNode_t **algebraic;
    directCode_t **algebraiccode;
  
    /** The Ordinary Differential Equation System (ODE)s: f(x,p,t) = dx/dt */
    ASTNode_t **ode; 
    directCode_t **odecode;

    /** The jacobian matrix df(x)/dx of the ODE system 
	neq x neq */
    ASTNode_t ***jacob;
    directCode_t ***jacobcode;

    /** was the jacobian matrix constructed ? */
    int jacobian;
    /** flag indicating that jacobian matrix construction had failed for
     this model already and does not need to be tried again */
    int jacobianFailed;

    /* COMPILED CODE OBJECTS */
    /** compiled code containing compiled ODE and Jacobian functions */
    compiled_code_t *compiledCVODEFunctionCode; 

    /** CVODE rhs function created by compiling code generated from model */
    CVRhsFn compiledCVODERhsFunction;
    /** CVODE jacobian function created by compiling
	code generated from model */
    CVDenseJacFn compiledCVODEJacobianFunction;

    /** Event function created by compiling code generated from model */
    EventFn compiledEventFunction; 


    /* compilation of adjoint functions */
    /** CVODE adjoint rhs function created by compiling
       code generated from model */
    CVRhsFnB compiledCVODEAdjointRhsFunction;
    /* remember which function is used (compiled or hard-coded) */
    CVRhsFnB current_AdjRHS; 
    /** CVODE adjoint jacobian function created by compiling code
	generated from model */
    CVDenseJacFnB compiledCVODEAdjointJacobianFunction;
    /* remember which function is used (compiled or hard-coded) */
    CVDenseJacFnB current_AdjJAC; 

    /* EVALUATION ORDERING */
    /** timing of assignment rule evaluation */

    List_t *observables ; /**< set of symbols that the user wishes to
			     have computed for output (list contains
			     char *) by default contains all
			     species */  
    int *observablesArray ; /**< set of symbols that the user wishes
			       to have computed for output; indexing
			       corresponds to 'names' */  

    int *assignmentsBeforeODEs; /**< set of assignments that must be
				   evaluated before evaluating ODEs,
				   boolean array indexed as for
				   'assignment' array */
    int *assignmentsBeforeEvents; /**< set of assignments that must be
				     evaluated before evaluating
				     events, boolean array indexed as
				     for 'assignment' array */ 
    int *assignmentsAfterEvents; /**< set of assignments that must be
				    evaluated after evaluating
				    events */

    /* adjoint */
    /* Adjoint: Given a parameter to observation map F(p),
       computes the adjoint operator applied to the vector v, F'*(p)v.
       v is given by a symbolic expression involving x and observation data. */


    int discrete_observation_data;    /**< 0: data observed is of
					   continuous type (i.e., interpolated)
                                           1: data observed is of
					   discrete type  */

    int compute_vector_v;            /*  if evaluateAST is called to
					 computed vector_v  */

    time_series_t *time_series;  /**< time series of observation data
				    or of vector v */

    ASTNode_t **vector_v;     /**< the vector v, expressiing linear
				 objective used in sensitivity solvers */
    ASTNode_t *ObjectiveFunction;  /**< expression for a general (nonlinear)
				      objective function */
 };

  struct odeSense
  {
    odeModel_t *om;    /**< odeModel_t structure from which sensitivity
			  structures where derived */    
    /* forward sensitivity analysis structure, neq x nsens */
    int nsens;               /**< number of parameters and initial conditions
				for sens. analysis, nsens = nsensP + nsensIC */
    int *index_sens;         /**< indices of parameters and init.cond.
				for sens. anal. in the main ID and data
				arrays char **names, char *value in
				odeModel_t and cvodeData_t */
    int nsensP;
    int *index_sensP;        /**< indices of sensitivity parameters in the
				sensitivity matrix (or -1 variables) */
    ASTNode_t ***sens;       /**< sensitivity matrix: df(x)/dp, neq x nsensP */
    int sensitivity;         /**< was the sensitivity matrix constructed ? */

    
    /** compiled code containing compiled sensitivity functions */
    compiled_code_t *compiledCVODESensitivityCode; 

    /** flag that indicates whether compilation is required,
	upon first request or when required parameters have
	changed since last compilation*/
    int recompileSensitivity;

    /** Sensitivity function created by compiling code generated from model */
    CVSensRhs1Fn compiledCVODESenseFunction;
    
    /* compilation of adjoint functions */
    /** CVODE adjoint quadrature function */
    CVQuadRhsFnB compiledCVODEAdjointQuadFunction;
    /* remember which function is used (compiled or hard-coded) */
    CVQuadRhsFnB current_AdjQAD;
  };
  
  struct objFunc
  {
    /* adjoint */
    /* Adjoint: Given a parameter to observation map F(p),
       computes the adjoint operator applied to the vector v, F'*(p)v.
       v is given by a symbolic expression involving x and observation data. */


    int discrete_observation_data;    /**< 0: data observed is of
					   continuous type (i.e., interpolated)
                                           1: data observed is of
					   discrete type  */

    int compute_vector_v;            /*  if evaluateAST is called to
					 computed vector_v  */

    time_series_t *time_series;  /**< time series of observation data
				    or of vector v */

    ASTNode_t **vector_v;     /**< the vector v, expressiing linear
				 objective used in sensitivity solvers */
    ASTNode_t *ObjectiveFunction;  /**< expression for a general (nonlinear)
				      objective function */
 
  };

#ifdef __cplusplus
extern "C" {
#endif

  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromFile(const char *);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromSBML2(SBMLDocument_t *);
  SBML_ODESOLVER_API odeModel_t *ODEModel_create(Model_t *);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromFileWithObservables(const char *, char **);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromSBML2WithObservables(SBMLDocument_t *, char **);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createWithObservables(Model_t *, char **);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromODEs(ASTNode_t **, int neq, int nass, int nconst, char **, double *, Model_t *);
  SBML_ODESOLVER_API void ODEModel_free(odeModel_t *);
  
  SBML_ODESOLVER_API int ODEModel_hasVariable(odeModel_t *, const char *);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndexByNum(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getOdeVariableIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getAssignedVariableIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getConstantIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndex(odeModel_t *, const char *);
  SBML_ODESOLVER_API const char *ODEModel_getVariableName(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getOde(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getAssignment(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API CVRhsFn ODEModel_getCompiledCVODERHSFunction(odeModel_t *);
  SBML_ODESOLVER_API CVDenseJacFn ODEModel_getCompiledCVODEJacobianFunction(odeModel_t *);
  SBML_ODESOLVER_API CVRhsFnB ODEModel_getCompiledCVODEAdjointRHSFunction(odeModel_t *);
  SBML_ODESOLVER_API CVDenseJacFnB ODEModel_getCompiledCVODEAdjointJacobianFunction(odeModel_t *);

  SBML_ODESOLVER_API int VariableIndex_getIndex(variableIndex_t *);
  SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *);
  SBML_ODESOLVER_API int ODEModel_getNeq(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNalg(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumAssignments(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumConstants(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumValues(odeModel_t *);
  SBML_ODESOLVER_API const Model_t *ODEModel_getModel(odeModel_t *);
  SBML_ODESOLVER_API void ODEModel_dumpNames(odeModel_t *);  
  SBML_ODESOLVER_API int ODEModel_constructJacobian(odeModel_t *);
  SBML_ODESOLVER_API void ODEModel_freeJacobian(odeModel_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getJacobianIJEntry(odeModel_t *, int i, int j);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getJacobianEntry(odeModel_t *, variableIndex_t *, variableIndex_t *);
  SBML_ODESOLVER_API ASTNode_t *ODEModel_constructDeterminant(odeModel_t *);
  SBML_ODESOLVER_API odeSense_t *ODEModel_constructSensitivity(odeModel_t *);
  SBML_ODESOLVER_API void ODEModel_compileCVODEFunctions(odeModel_t *);

  /* Sensitivity Model */
  SBML_ODESOLVER_API odeSense_t *ODESense_create(odeModel_t *, cvodeSettings_t *);
  SBML_ODESOLVER_API void ODESense_free(odeSense_t *);
  SBML_ODESOLVER_API variableIndex_t *ODESense_getSensParamIndexByNum(odeSense_t *, int);
SBML_ODESOLVER_API CVQuadRhsFnB ODESense_getCompiledCVODEAdjointQuadFunction(odeSense_t *);
  SBML_ODESOLVER_API CVSensRhs1Fn ODESense_getCompiledCVODESenseFunction(odeSense_t *);
  SBML_ODESOLVER_API int ODESense_getNsens(odeSense_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODESense_getSensIJEntry(odeSense_t *, int i, int j);
  SBML_ODESOLVER_API const ASTNode_t *ODESense_getSensEntry(odeSense_t *, variableIndex_t *, variableIndex_t *);
  SBML_ODESOLVER_API void ODESense_compileCVODESenseFunctions(odeSense_t *);
#ifdef __cplusplus
}
#endif

/* internal functions, not be used by calling applications */  
int ODEModel_getVariableIndexFields(odeModel_t *, const char *SBML_ID);
int ODESense_constructMatrix(odeSense_t *, odeModel_t *);
void ODESense_freeMatrix(odeSense_t *);
void ODESense_freeStructures(odeSense_t *os);

#endif
