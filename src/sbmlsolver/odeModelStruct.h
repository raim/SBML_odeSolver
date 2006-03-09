#ifndef _ODEMODELSTRUCT_H_
#define _ODEMODELSTRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SBMLDocument SBMLDocument_t;
typedef struct Model Model_t;
typedef struct ASTNode ASTNode_t ;
typedef struct compiled_code compiled_code_t;
typedef struct ts time_series_t ;
typedef struct cvodeData cvodeData_t ;
typedef void (*CVRhsFn)(realtype t, N_Vector y, N_Vector ydot, void *f_data);
typedef void (*CVDenseJacFn)(long int N, DenseMat J, realtype t,
                             N_Vector y, N_Vector fy, void *jac_data,
                             N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
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
  CVRhsFn compiledCVODERhsFunction; /**< CVODE rhs function created by compiling code generated from model */
  CVDenseJacFn compiledCVODEJacobianFunction; /**< CVODE jacobian function created by compiling code generated from model */
  EventFn compiledEventFunction;
  compiled_code_t *compiledCVODEFunctionCode; /**< compiled code containing compiled functions */

  /* adjoint */
  /* Given a parameter to observation map F(p),
     computes the adjoint operator applied to the vector v, F'*(p)v.
     v is given by a symbolic expression involving x and observation data. */

  int n_adj_sens;          /**< number of parameters for adj. sens. analysis */
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
