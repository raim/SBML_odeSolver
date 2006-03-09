#ifndef _CVODEDATASTRUCT_H_
#define _CVODEDATASTRUCT_H_

#include "sbmlsolver/cvodedatatype.h"
#include "sbmlsolver/odemodeldatatype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ASTNode ASTNode_t;
typedef struct cvodeSettings cvodeSettings_t ;

/** Contains the data needed for AST formula evaluation and odeModel
    integration and usually corresponds to an odeModel

    cvodeData is used for storing current variable, sensitivity and time
    values. It also holds the internal version of ODEs for optimized for
    a specific integration run and current (on or off) states of event
    triggers and of steady state approximation and the number of integration
    runs with this instance of cvodeData.

    API Interface functions to retrieve values directly from cvodeData,
    instead of via IntegratorInstance_getVariableValue()) will be available
    in one of the next releases!
*/
struct cvodeData {

  odeModel_t *model;
  
  /* ODEs f(x,p,t) = dx/dt and values x. The ODEs are usually
     optimized versions of the same array in odeModel */
  int neq;         /**< number of ODEs */
  ASTNode_t **ode; /**< optimized ODEs as used for integration */

  
  int nvalues; /**< total number of values (variables x(t) + parameters p) */
  /** The value array is used to write and read the
     current values of all variables x(t) and parameters p of the
     system (of which there are `nvalues') */  
  double *value; 

  /** the current time of the integration */
  float currenttime;

  /* current sensitivities: dx(t)/dp */
  int nsens;            /**< number of sensitivities */
  double **sensitivity; /**< current values of sensitivities dx(t)/dp */
  /** current values of parameters in sensitivity analysis, required
      and filled only if no r.h.s function fS is available */
  realtype *p;
  
  /** cvode settings: start- and end times, timesteps, number of timesteps,
     error tolerances, maximum number of steps, etc... */
  cvodeSettings_t *opt;
  
  /** trigger flags: check if triggers were active or not
     at the previous time step */
  int *trigger;
  /** steady state flag: check if steady state was found */
  int steadystate; 

  /** Results: time series of integration are stored in this
     structure (see above) */
  cvodeResults_t *results;

  /** number of (forward) runs with the one integratorInstance */
  int run;

  

  /* Adjoint specific  */ 
  double *adjvalue;  /** The value array is used to write and read the
			 current values of all adjoint variables \psi(t) (of which there are `neq') */  
 
  /** number of adjoint runs with the one integratorInstance */
  int adjrun;
} ;

#ifdef __cplusplus
}
#endif

#endif