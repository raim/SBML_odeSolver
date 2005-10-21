/*
  Last changed Time-stamp: <2005-10-21 09:51:47 raim>
  $Id: cvodedata.h,v 1.11 2005/10/21 08:55:20 raimc Exp $
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
  ASTNode_t ***jacob;
  /* was the model the jacobian constructed ? */
  int jacobian;

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

/** get values from cvodeResults */
SBML_ODESOLVER_API double CvodeResults_getTime(cvodeResults_t *, int);
SBML_ODESOLVER_API double CvodeResults_getValue(cvodeResults_t *results,
						variableIndex_t *vi, int n);
SBML_ODESOLVER_API int CvodeResults_getNout(cvodeResults_t *);




/* internal functions used by integratorInstance.c */

cvodeData_t *CvodeData_create(odeModel_t *);

int CvodeData_initialize(cvodeData_t *, cvodeSettings_t *, odeModel_t *);

void CvodeData_free(cvodeData_t *);

cvodeResults_t *CvodeResults_create(cvodeData_t *, int);

void CvodeResults_free(cvodeResults_t *, int);
#endif

/* End of file */
