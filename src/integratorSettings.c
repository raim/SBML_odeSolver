/*
  Last changed Time-stamp: <2005-10-12 23:01:05 raim>
  $Id: integratorSettings.c,v 1.4 2005/10/12 21:22:45 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/solverError.h"

static cvodeSettings_t *
CvodeSettings_createDefaultsFromTime(double Time, int PrintStep,
				     double *TimePoints);


/**
   Creates a settings structure with default values
*/

cvodeSettings_t *
CvodeSettings_createDefaults()
{
  return CvodeSettings_createDefaultsFromTime(1., 1, NULL);
}


/* creates a settings structure with default Values
   for Errors, MxStep and Swicthes */
static cvodeSettings_t *
CvodeSettings_createDefaultsFromTime(double Time, int PrintStep,
				     double *TimePoints)
{
  return CvodeSettings_create(Time, PrintStep, TimePoints,
			      1e-18, 1e-10, 10000, 1, 0, 0, 0, 1);
}

/** Creates a settings structure from a predefined timeseries
    and fills rest with default values
*/

SBML_ODESOLVER_API cvodeSettings_t *
CvodeSettings_createFromTimeSeries(double *timeseries, int n)
{
  return CvodeSettings_createDefaultsFromTime(timeseries[n], n-1, timeseries);
}

SBML_ODESOLVER_API void
CvodeSettings_DumpToString(cvodeSettings_t *set) {
}


/**
   Creates a settings structure from input values
*/

SBML_ODESOLVER_API cvodeSettings_t *
CvodeSettings_create(double Time, int PrintStep, double *TimeSeries,
		     double Error, double RError, double Mxstep,
		     int UseJacobian, 
		     int Indefinitely, int HaltOnEvent,
		     int SteadyState, int StoreResults) {

  cvodeSettings_t *set;
  ASSIGN_NEW_MEMORY(set, struct cvodeSettings, NULL);

  /* 1. Setting SBML ODE Solver integration parameters */
  CvodeSettings_setErrors(set, Error, RError, Mxstep);
  CvodeSettings_setSwitches(set, UseJacobian, Indefinitely,
			    HaltOnEvent, SteadyState, StoreResults);

  /* 2. Setting Requested Time Series */
  /* Unless indefinite integration, generate a TimePoints array  */
  if  ( !Indefinitely ) {    
    /* copy optional TimePoint array */
    if ( TimeSeries != NULL )
      CvodeSettings_setTimeSeries(set, TimeSeries, PrintStep);
    /* ... or generate default TimePoint array */
    else 
      CvodeSettings_setTime(set, Time, PrintStep);
  }
  return set;
}


/**
   Creates a settings structure and copies all values from input
*/

SBML_ODESOLVER_API cvodeSettings_t *
CvodeSettings_clone(cvodeSettings_t *set) {

  int i;
  cvodeSettings_t *clone;
  ASSIGN_NEW_MEMORY(clone, struct cvodeSettings, NULL);
    
  /* Setting SBML ODE Solver integration parameters */
  CvodeSettings_setErrors(set, set->Error, set->RError, set->Mxstep);
  CvodeSettings_setSwitches(set, set->UseJacobian, set->Indefinitely,
			    set->HaltOnEvent, set->SteadyState,set->
			    StoreResults);
  
  /* Unless indefinite integration is chosen, generate a TimePoints array  */
  if  ( !clone->Indefinitely ) {    
    ASSIGN_NEW_MEMORY_BLOCK(clone->TimePoints,clone->PrintStep+1,double,NULL);
    /* copy TimePoint array */
    for ( i=0; i<=clone->PrintStep; i++ ) {
      clone->TimePoints[i] = set->TimePoints[i];
    }
  }
  return clone;
}


/**
   Sets absolute and relative error tolerances and maximum number of
   internal steps during CVODE integration 
*/

SBML_ODESOLVER_API
void CvodeSettings_setErrors(cvodeSettings_t *set,
			double Error, double RError, double Mxstep) 
{
  CvodeSettings_setError(set, Error);
  CvodeSettings_setRError(set, RError);
  CvodeSettings_setMxstep(set, Mxstep);    
}


/**
   Sets absolute error tolerance 
*/

SBML_ODESOLVER_API 
void CvodeSettings_setError(cvodeSettings_t *set, double Error)
{
  set->Error = Error;
}


/**
   Sets relative error tolerance 
*/

SBML_ODESOLVER_API 
void CvodeSettings_setRError(cvodeSettings_t *set, double RError)
{
  set->RError = RError;
}


/**
   Sets maximum number of internal steps during CVODE integration
*/

SBML_ODESOLVER_API 
void CvodeSettings_setMxstep(cvodeSettings_t *set, int Mxstep)
{
  set->Mxstep = Mxstep;  
}

/**
   Sets integration switches in cvodeSettings
*/

SBML_ODESOLVER_API
void CvodeSettings_setSwitches(cvodeSettings_t *set,
			       int UseJacobian,
			       int Indefinitely, int HaltOnEvent,
			       int SteadyState, int StoreResults)
{  
  set->UseJacobian = UseJacobian;
  set->Indefinitely = Indefinitely;
  set->HaltOnEvent = HaltOnEvent;
  set->SteadyState = SteadyState;
  set->StoreResults = StoreResults;
}


/**
   Sets a predefined timeseries in cvodeSettings.
   Assigns memory for an array of requested time points.
   PrintStep must be the size of the passed array timeseries.
   Returns 1, if sucessful and 0, if not.
*/

SBML_ODESOLVER_API 
int CvodeSettings_setTimeSeries(cvodeSettings_t *set,
				 double *timeseries, int PrintStep)
{
  int i;
  free(set->TimePoints);
  ASSIGN_NEW_MEMORY_BLOCK(set->TimePoints, PrintStep+1, double, 0);    
  set->Time = timeseries[PrintStep-1];
  set->PrintStep = PrintStep;
  set->TimePoints[0] = 0.0;
  for ( i=1; i<=PrintStep; i++ ) 
    set->TimePoints[i] = timeseries[i-1];
  
  return 1;
}


/**
   Calculates a time point series from Endtime and Printstep
   and sets the time series in cvodeSettings
   Returns 1, if sucessful and 0, if not.
*/

SBML_ODESOLVER_API 
int CvodeSettings_setTime(cvodeSettings_t *set,
			   double EndTime, int PrintStep)
{
  int i;
  double *timeseries;
  ASSIGN_NEW_MEMORY_BLOCK(timeseries, PrintStep, double, 0);  
  for ( i=1; i<=PrintStep; i++ )
    timeseries[i-1] = i * EndTime/PrintStep;
  CvodeSettings_setTimeSeries(set, timeseries, PrintStep);
  free(timeseries);
  
  return 1;
}


/**
   Returns the last time point of integration or -1, if
   Indefinitely is set to TRUE (1);   
*/

SBML_ODESOLVER_API 
double CvodeSettings_getEndTime(cvodeSettings_t *set)
{
  if ( !set->Indefinitely )
    return set->Time;
  else
    return -1.;
}


/**
   Returns the time step of integration; if a pre-defined
   time series has been set, this is only the first time step
*/

SBML_ODESOLVER_API 
int CvodeSettings_getTimeStep(cvodeSettings_t *set)
{
  if ( !set->Indefinitely )
    return set->TimePoints[1];
  else
    return set->Time;
}


/**
   Returns the number of integration steps or -1,
   Indefinitely is set to TRUE (1)
*/

SBML_ODESOLVER_API 
int CvodeSettings_getPrintsteps(cvodeSettings_t *set)
{
  if ( !set->Indefinitely )
    return set->PrintStep;
  else
    return -1;
}


/**
   Returns the time of the ith time step, i must be smaller or
   equal to PrintStep unless Indefinitely is set to TRUE (1)
*/

SBML_ODESOLVER_API 
double CvodeSettings_getTime(cvodeSettings_t *set, int i)
{
  if ( !set->Indefinitely )
    return set->TimePoints[i];
  else
    return i * set->Time;
}


/**
   Returns the absolute error tolerance
*/

SBML_ODESOLVER_API 
double CvodeSettings_getError(cvodeSettings_t *set)
{
  return set->Error;
}


/**
   Returns the relative error tolerance
*/

SBML_ODESOLVER_API 
double CvodeSettings_getRError(cvodeSettings_t *set)
{
  return set->RError;
}


/**
   Returns the maximum number of internal time steps taken
   by CVODE to reach the next output time (printstep)
*/

SBML_ODESOLVER_API 
int CvodeSettings_getMxstep(cvodeSettings_t *set)
{
  return set->Mxstep;
}


/**
   Returns 1, if the automatically generated 
   or 0 if CVODE's internal approximation
   of the jacobian matrix will be used by CVODE 
*/

SBML_ODESOLVER_API 
int CvodeSettings_useJacobian(cvodeSettings_t *set)
{
  return set->UseJacobian;
}


/**
   Returns 1, if indefinite integration has been chosen,
   and 0 otherwise
*/

SBML_ODESOLVER_API 
int CvodeSettings_integratieIndefinitely(cvodeSettings_t *set)
{
  return set->Indefinitely;
}


/**
   Returns 1, if integration should stop upon an event trigger
   and 0 if integration should continue afert evaluation of
   event assignments
*/

SBML_ODESOLVER_API 
int CvodeSettings_haltOnEvent(cvodeSettings_t *set)
{
  return set->HaltOnEvent;
}


/**
   Returns 1, if integration should stop upon detection of a
   steady state, and 0 if integration should continue 
*/

SBML_ODESOLVER_API 
int CvodeSettings_checkSteadyState(cvodeSettings_t *set)
{
  return set->SteadyState;
}


/**
   Returns 1, if integration results should be stored internally,
   and 0 if not; If set to 0 current values can be retrieved during
   an integration loop, and the values at the end time of integration
   afterwards.
*/

SBML_ODESOLVER_API 
int CvodeSettings_storeResults(cvodeSettings_t *set)
{
  return set->StoreResults;
}


/**
   Frees cvodeSettings.
*/

SBML_ODESOLVER_API 
void CvodeSettings_free(cvodeSettings_t *set)
{
  if ( set->TimePoints != NULL )
    free(set->TimePoints);
  free(set);
}

/**
   Creates a settings structure from a timeSettings structure
   and fills rest with default values
*/

cvodeSettings_t *
CvodeSettings_createFromTimeSettings(timeSettings_t *time)
{
  return CvodeSettings_createDefaultsFromTime(time->tend, time->nout, NULL);
}



/* for when timeSettings might become a separate structure,
 not used at the moment */
timeSettings_t *
TimeSettings_create(double t0, double tend, int nout) {

  timeSettings_t *time;
  
  time = (timeSettings_t *)calloc(1, sizeof(timeSettings_t));

  time->t0 = t0;
  time->tend = tend;
  time->nout = nout;
  time->tmult = (t0-tend) / nout;
  return time;				  

}

void
TimeSettings_free(timeSettings_t *time)
{
  free(time);
}




/* End of file */
