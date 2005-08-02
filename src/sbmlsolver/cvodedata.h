/*
  Last changed Time-stamp: <2005-08-02 01:51:40 raim>
  $Id: cvodedata.h,v 1.7 2005/08/02 13:20:32 raimc Exp $
*/
#ifndef _CVODEDATA_H_
#define _CVODEDATA_H_

#include <stdio.h>
#include <sbml/SBMLTypes.h>

#include "sbmlsolver/cvodedatatype.h"
#include "sbmlsolver/odemodeldatatype.h"
#include "sbmlsolver/integratorSettings.h"

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

struct _CvodeResults {
  int nout;        /* counter for calculated time steps
		      (without initial conditions), this number
		      can be lower then the same variable in CvodeData,
		      in case the integration is prematurely stopped. */
  double *time;    /* time steps */
  
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

struct _CvodeData {

  odeModel_t *model;
  

  /* The value array is used to write and read the
     current values of all variables and parameters of the
     system (of which there are `nvalues') */
  int nvalues;
  double *value;

  /* stores the current time of the integration */
  float currenttime;

  
  /* cvode settings: start- and end times, timesteps, number of timesteps,
     and a counter, that is used to recalculate the settings upon a restart
     of integration upon detection of an event. */
  float t0;
  float tmult;
  float nout;
  float tout;

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

cvodeData_t *
CvodeData_create(int nvalues, int nevents);

cvodeData_t *
CvodeData_createFromODEModel(odeModel_t *m);

cvodeData_t * 
constructODEs(Model_t *m, int simplify);

void
CvodeData_free(cvodeData_t *data);
void
CvodeData_freeExcludingModel(cvodeData_t *data);
cvodeResults_t *
CvodeResults_create(cvodeData_t *data);

#endif

/* End of file */
