/*
  Last changed Time-stamp: <2005-05-30 12:43:32 raim>
  $Id: cvodedata.h,v 1.1 2005/05/30 19:49:12 raimc Exp $
*/
#ifndef _CVODEDATA_H_
#define _CVODEDATA_H_

/* Settings for CVODE Integration */
typedef struct _CvodeSettings {
  double Time;          /* Time to which model is integrated */
  double PrintStep;     /* Number of output steps from 0 to 'Time' */
  double Error;         /* absolute tolerance in Cvode integration */
  double RError;        /* relative tolerance in Cvode integration */
  double Mxstep;        /* maximum step number for CVode integration */
  int PrintMessage;     /* Print messages */
  int PrintOnTheFly;    /* Print species concentration during integration */
  int HaltOnEvent;      /* Stops integration upon an event */
  int SteadyState;      /* Stops integration upon a steady state */
  int UseJacobian;      /* Toggle use of Jacobian ASTs or approximation */
} CvodeSettings;

CvodeSettings Set;

/* Stores CVODE specific integration results, and is part of
   the CvodeData structure (see below) */

typedef struct _CvodeResults {
  int nout;        /* counter for calculated time steps
		      (without initial conditions), this number
		      can be lower then the same variable in CvodeData,
		      in case the integration is prematurely stopped. */
  double *time;    /* time steps */
  
  /* the following arrays represent the time series of the variables with
     the same names in CvodeData */
  double **value;  /* values defined by ODEs, and calculated by
		      the CVODE integrator */
  double **avalue; /* values defined by assignment equations, calculated during
		      integration, calculated only for printing and higher
		      level interfaces */
  double **pvalue; /* constant parameter values, constant during integration,
		      stored only for printing of constant species and higher
		      level interfaces */
} *CvodeResults;

/* Contains all data needed for CVODE integration, i.e.
   the ODEs, initial values, variable and parameter IDs and
   names, events, and integration settings and output files,
   but also
   the SBML model from which they have been derived,
   and the SBML version of the ODE model.
 */

typedef struct _CvodeData {

  char *filename;
  FILE *outfile; 

  Model_t *m;
  Model_t *simple;
  char *modelName;
  char *modelId;

  /* Constants: stores constant species, and species,
     compartments or parameters, that can be changed by an event */
  int nconst;
  char **parameter;  
  double *pvalue;

  /* Assigned variables: stores species, compartments and parameters,
     that are set by an assignment rule */
  int nass;
  char **ass_parameter;
  double *avalue;
  ASTNode_t **assignment;
 
  /* The main data: number, names, initial values and equation ASTs
     of the ODEs. The value array is used to write and read the
     current value of the ODE variables during simulation. */
  int neq;  
  ASTNode_t **ode; 
  char **species;
  char **speciesname;
  double *value;

  /* The jacobian matrix (d[X]/dt)/d[Y] of the ODE system */
  ASTNode_t ***jacob;
  /* The determinant of the jacobian */
  ASTNode_t *det;
  
  /* stores the current time of the integration */
  float currenttime;

  /* Mean and variance, and standard deviation of ODE rates, used
     for an internal check of steady states. If a steady state was
     is detected the flag steadystate is set to 1. */
  double dy_mean;
  double dy_var;
  double dy_std;
  int steadystate; 
  
  /* cvode settings: start- and end times, timesteps, number of timesteps,
     and a counter, that is used to recalculate the settings upon a restart
     of integration upon detection of an event. */
  float t0;
  float tmult;
  float nout;
  float tout;
  float cnt;
  double Error;         /* absolute tolerance in Cvode integration */
  double RError;        /* relative tolerance in Cvode integration */
  double Mxstep;        /* maximum step number for CVode integration */
  int PrintMessage;     /* Print messages */
  int PrintOnTheFly;    /* Print species concentration during integration */
  int HaltOnEvent;      /* Stops integration upon an event */
  int SteadyState;      /* Stops integration upon a steady state */
  int UseJacobian;      /* Toggle use of Jacobian ASTs or approximation */
  
  /* trigger flags: check if triggers were active or not
     at the previous time step */
  int *trigger;  

  /* Results: time series of integration are stored in this
     structure (see above) */
  CvodeResults results;

  /* Errors are counted during ODE creation. If errors occured, the
     procedure is aborted. */
  int errors;
  
  /* The flag `run' remembers if already tried with or without use
     of generated Jacobian matrix or internal approximation,
     or with lower error tolerance. It is used to restart the integrator
     with changed settings upon failure. */
  int run;
  
} *CvodeData;

CvodeData
CvodeData_create(int neq, int nconst, int nass, int nevents);
void
CvodeData_free(CvodeData data);
CvodeResults
CvodeResults_create(CvodeData data);

#endif

/* End of file */
