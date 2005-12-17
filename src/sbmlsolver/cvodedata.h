/*
  Last changed Time-stamp: <2005-12-17 00:15:16 raim>
  $Id: cvodedata.h,v 1.26 2005/12/17 13:40:59 raimc Exp $
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
 *     Rainer Machne
 *
 * Contributor(s):
 *     Andrew Finney
 *     
 */

#ifndef _CVODEDATA_H_
#define _CVODEDATA_H_

/* Header Files for CVODE: required only for realtype *p  */
#include "nvector_serial.h"

#include <stdio.h>
#include <sbml/SBMLTypes.h>

#include "sbmlsolver/cvodedatatype.h"
#include "sbmlsolver/odemodeldatatype.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/odeModel.h"


/** Stores CVODE specific integration results, data correspond
    to the respective values in cvodeData */
struct cvodeResults {
  int nout;        /**< counter for calculated time steps (without
		        initial conditions), this number can be lower
		        then the same variable in cvodeData, in case
		        the integration is prematurely stopped. */
  double *time;    /**< the time steps */

  /** number of variables for which results exist */
  int nvalues;     
  /** the following arrays represent the time series of all variables
      and parameters of the model */
  double **value;

  /* number of variables x(t) for which sensitivities are calculated */
  int neq;
  /** number of parameters p for sens. analysis */
  int nsens;
  /** time course of sensitivities dx(t)/dp */
  double ***sensitivity;

} ;

/** Contains all data needed for CVODE integration, i.e.
   the ODEs, initial values, variable and parameter IDs and
   names, events, and integration settings and output files,
   but also
   the SBML model from which they have been derived,
   and the SBML version of the ODE model.
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
  double **sensitivity; /**< current values of sensitivities d[Y(t)]/dP */
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

  /** number of runs with the one integratorInstance */
  int run;

} ;

#ifdef __cplusplus
extern "C" {
#endif
  /* create data for formula evaluation */
  SBML_ODESOLVER_API cvodeData_t *CvodeData_create(odeModel_t *);
  SBML_ODESOLVER_API void CvodeData_initializeValues(cvodeData_t *);
  SBML_ODESOLVER_API void CvodeData_free(cvodeData_t *);
  /* get values from cvodeResults */
  SBML_ODESOLVER_API double CvodeResults_getTime(cvodeResults_t *, int);
  SBML_ODESOLVER_API double CvodeResults_getValue(cvodeResults_t *, variableIndex_t *, int);
  SBML_ODESOLVER_API int CvodeResults_getNout(cvodeResults_t *);
  SBML_ODESOLVER_API void CvodeData_free(cvodeData_t *);
  SBML_ODESOLVER_API cvodeData_t *CvodeData_create(odeModel_t *);
  SBML_ODESOLVER_API double CvodeResults_getSensitivityByNum(cvodeResults_t *,  int value, int parameter, int timestep);
  SBML_ODESOLVER_API double CvodeResults_getSensitivity(cvodeResults_t *,  variableIndex_t *y,  variableIndex_t *p, int timestep);
  SBML_ODESOLVER_API void CvodeResults_free(cvodeResults_t *);
#ifdef __cplusplus
}
#endif


/* internal functions used by integratorInstance.c */
int CvodeData_initialize(cvodeData_t *, cvodeSettings_t *, odeModel_t *);
cvodeResults_t *CvodeResults_create(cvodeData_t *, int);
int CvodeResults_allocateSens(cvodeResults_t *, int neq, int nsens, int nout);


#endif

/* End of file */
