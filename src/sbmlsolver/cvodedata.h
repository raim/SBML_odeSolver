/*
  Last changed Time-stamp: <2005-10-27 23:57:32 raim>
  $Id: cvodedata.h,v 1.17 2005/10/28 09:04:12 afinney Exp $
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

#include <stdio.h>
#include <sbml/SBMLTypes.h>

#include "sbmlsolver/cvodedatatype.h"
#include "sbmlsolver/odemodeldatatype.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/odeModel.h"


struct odeModel
{
  Model_t *m;
  Model_t *simple;

  /* All names, i.e. ODE variables, assigned parameters, and constant
     parameters */
  char **names; 

  /* number of ODEs (neq), assigned parameters (nass), and constant
     parameters (nconst) */
  int neq;
  int nass;
  int nconst;  

  /* Assigned variables: stores species, compartments and parameters,
     that are set by an assignment rule */
  ASTNode_t **assignment;

  /* The main data: number, names, and equation ASTs
     of the ODEs. The value array is used to write and read the
     current value of the ODE variables during simulation. */
  ASTNode_t **ode; 

  /* The jacobian matrix (d[X]/dt)/d[Y] of the ODE system */
  /* neq x neq */
  ASTNode_t ***jacob;
  /* was the model the jacobian constructed ? */
  int jacobian;

  /* forward sensitivity analysis */
  /* neq x num_param */
  int num_param;    /* number of parameters for sens. analysis */
  int *index_param; /* indexes of parameters in char **names, char *value*/
  ASTNode_t ***jacob_param; /*  (d[Y]/dt)/dP  */
  int sensitivity;  /* just a flag, might not be required (in options) */

  /* adjoint */

};

/* CvodeSettings Set; */

/* Stores CVODE specific integration results, and is part of
   the CvodeData structure (see below) */

struct cvodeResults {
  int nout;        /* counter for calculated time steps
		      (without initial conditions), this number
		      can be lower then the same variable in CvodeData,
		      in case the integration is prematurely stopped. */
  double *time;    /* time steps */

  int nvalues;     /* number of variables for which results exist */
  /* the following arrays represent the time series of all variables
     and parameters of the model */
  double **value;

  /* number of parameters for sens. analysis */
  int num_param;
  /* sensitivities: d[Y(t)]/dP */
  double ***sensitivity;

} ;

/* Contains all data needed for CVODE integration, i.e.
   the ODEs, initial values, variable and parameter IDs and
   names, events, and integration settings and output files,
   but also
   the SBML model from which they have been derived,
   and the SBML version of the ODE model.
 */

struct cvodeData {

  odeModel_t *model;
  

  /* The value array is used to write and read the
     current values of all variables and parameters of the
     system (of which there are `nvalues') */
  int nvalues;
  double *value;

  /* stores the current time of the integration */
  float currenttime;

  /* current sensitivities: d[Y(t)]/dP */
  double **sensitivity;
  
  /* cvode settings: start- and end times, timesteps, number of timesteps,
     error tolerances, max. number of steps, etc... */
  cvodeSettings_t *opt;
  
  /* trigger flags: check if triggers were active or not
     at the previous time step */
  int *trigger;  
  int steadystate; 

  /* Results: time series of integration are stored in this
     structure (see above) */
  cvodeResults_t *results;

  /* The flag `run' remembers if already tried with or without use
     of generated Jacobian matrix or internal approximation,
     or with lower error tolerance. It is used to restart the integrator
     with changed settings upon failure. */
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
  SBML_ODESOLVER_API int CvodeData_initialize(cvodeData_t *, cvodeSettings_t *, odeModel_t *);
  SBML_ODESOLVER_API cvodeData_t *CvodeData_create(odeModel_t *);

#ifdef __cplusplus
}
#endif


/* internal functions used by integratorInstance.c */
cvodeResults_t *CvodeResults_create(cvodeData_t *, int);
void CvodeResults_free(cvodeResults_t *);


#endif

/* End of file */
