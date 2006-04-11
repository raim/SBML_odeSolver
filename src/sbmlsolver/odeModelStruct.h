#ifndef _ODEMODELSTRUCT_H_
#define _ODEMODELSTRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sbml/SBMLTypes.h"
  
#include "sbmlsolver/compiler.h"
#include "sbmlsolver/interpol.h"
#include "sbmlsolver/cvodedatatype.h"


  /** Macros for accessing ODE variable (VAR), assigned values (VAS) and
      constant parameters (PAR) of the odeModel structure's `names' array.

      _model should be a pointer to an odeModel structure.  _neqIndex
      is an integer from 0 to number of the requested value type
      (VAR: om->neq, VAS: om->nass, PAR: om->nconst, SENS: om->nsens).

  */
#define VAR(_model, _neqIndex) {_model->names[_neqIndex];}
#define VAS(_model, _nassIndex) {_model->names[_model->neq + _nassIndex];} 
#define PAR(_model, _nconstIndex)				\
  {_model->names[_model->neq + _model->nass + _nassIndex];}

#define SENS(_model, _nsensIndex) \
  {_model->names[_model->indexSens[_nsensIndex]]}; 
  
  
  typedef int (*EventFn)(cvodeData_t *, int *);


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

    /** All names, i.e. ODE variables, assigned parameters, and constant
	parameters */
    char **names; 

    int neq;    /**< number of ODEs */
    int nalg;   /**< number of algebraic rules */ 
    int nass;   /**< number of assigned variables (nass) */
    int nconst; /**< number of constant parameters */ 

    /** Assigned variables: stores species, compartments and parameters,
	that are set by an assignment rule */
    ASTNode_t **assignment;

    /** Algebraic Rules (constraints) as used for DAE systems */
    ASTNode_t **algebraic;
  
    /** The Ordinary Differential Equation System (ODE)s: f(x,p,t) = dx/dt */
    ASTNode_t **ode; 

    /** The jacobian matrix df(x)/dx of the ODE system 
	neq x neq */
    ASTNode_t ***jacob;
    /** was the jacobian matrix constructed ? */
    int jacobian;

    /* forward sensitivity analysis structure
       neq x num_param */
    int nsens;               /**< number of parameters for sens. analysis */
    int *index_sens;         /**< indexes of parameters in char **names,
			        char *value*/
    ASTNode_t ***jacob_sens; /**< sensitivity matrix: df(x)/dp  */
    int sensitivity;         /**< was the sensitivity matrix constructed ? */

    /* compilation */
    CVRhsFn compiledCVODERhsFunction; /**< CVODE rhs function created
					 by compiling code generated
					 from model */
    CVDenseJacFn compiledCVODEJacobianFunction; /**< CVODE jacobian
						   function created by
						   compiling code
						   generated from
						   model */
    EventFn compiledEventFunction; /**< Event function created by
				      compiling code generated from
				      model */
    compiled_code_t *compiledCVODEFunctionCode; /**< compiled code
						   containing compiled
						   functions */

    /* assignment Rule Optimization */
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
    /* Given a parameter to observation map F(p),
       computes the adjoint operator applied to the vector v, F'*(p)v.
       v is given by a symbolic expression involving x and observation data. */

    int n_adj_sens;          /**< number of parameters for
				adj. sens. analysis */
    int *index_adj_sens;

    int observation_type;    /**< 0: continuous data observed
                                1: discrete data observed  */

    time_series_t *time_series;  /**< time series of observation data
				    or of vector v */

    ASTNode_t **vector_v;     /**< The vector v*/
 
  };
#ifdef __cplusplus
}
#endif

#endif
