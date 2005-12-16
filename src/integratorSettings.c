/*
  Last changed Time-stamp: <2005-12-16 02:29:18 raim>
  $Id: integratorSettings.c,v 1.18 2005/12/16 01:30:21 raimc Exp $
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
 *
 */
/*! \defgroup setttings Integrator Settings
    \ingroup integration 
    \brief This module contains all functions to set integration options
    in integratorSettings
    
    With these functions an application can choose integration time,
    methods and options like error tolerances.
*/
/*@{*/

#include <stdio.h>
#include <stdlib.h>

#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/solverError.h"


static int CvodeSettings_setTimeSeries(cvodeSettings_t *, double *, int);


/** Creates a settings structure with default values

DEFAULT INTEGRATION SETTINGS ARE

1) CVODE SPECIFIC SETTINGS:

absolute error tolerance for each output time:   1e-18

relative error tolerance for each output time:   1e-10

max. nr. of steps to reach next output time:     10000

Nonlinear solver method:                         0: BDF

          Maximum Order:                         5

Iteration method:                                0: NEWTON

Sensitivity:                                     0: no

     method:                                     0: simultaneous

2) SOSlib SPECIFIC SETTINGS:

Jacobian matrix: 1: generate Jacobian

Indefinitely:    0: finite integration

Event Handling:  0: keep integrating

Steady States:   0: keep integrating

Store Results:   1: store results (only for finite integration)

3) TIME SETTINGS:

endtime: 1

steps:   10

*/

SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_create()
{
  return CvodeSettings_createWithTime(1., 10);
}


/** Creates a settings structure with default Values
    for Errors, MxStep and Swicthes
*/

SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_createWithTime(double Time, int PrintStep)
{
  return CvodeSettings_createWith(Time, PrintStep,
				  1e-18, 1e-10, 10000, 
				  1, 0, 0, 0, 1, 0, 0);
}


/** Print all cvodeSettings
*/

SBML_ODESOLVER_API void CvodeSettings_dump(cvodeSettings_t *set)
{
  int i;
  printf("\n");
  printf("SOSlib INTEGRATION SETTINGS\n");
  printf("1) CVODE SPECIFIC SETTINGS:\n");
  printf("absolute error tolerance for each output time:   %g\n",
	 set->Error);
  printf("relative error tolerance for each output time:   %g\n",
	 set->RError);
  printf("max. nr. of steps to reach next output time:     %d\n",
	 set->Mxstep);
  printf("Nonlinear solver method:                         %d: %s\n"
	 "          Maximum Order:                         %d\n",
	 set->CvodeMethod, CvodeSettings_getMethod(set), set->MaxOrder);
  printf("Iteration method:                                %d: %s\n",
	 set->IterMethod, CvodeSettings_getIterMethod(set));
  printf("Sensitivity:                                     %s\n",
	 set->Sensitivity ? "1: yes " : "0: no");
  printf("     method:                                     %d: %s\n",
	 set->SensMethod, CvodeSettings_getSensMethod(set));
  printf("2) SOSlib SPECIFIC SETTINGS:\n");
  printf("Jacobian matrix: %s\n", set->UseJacobian ?
	 "1: generate Jacobian" : "0: CVODE's internal approximation");
  printf("Indefinitely:    %s\n", set->Indefinitely ?
	 "1: infinite integration" :
	 "0: finite integration");
  printf("Event Handling:  %s\n", set->HaltOnEvent ?
	 "1: stop integration" :
	 "0: keep integrating");
  printf("Steady States:   %s\n", set->SteadyState ?
	 "1: stop integration" :
	 "0: keep integrating");
  printf("Store Results:   %s\n", set->StoreResults ?
	 "1: store results (only for finite integration)" :
	 "0: don't store results");  
  printf("3) TIME SETTINGS:\n");
  if ( set->Indefinitely )
    printf("Infinite integration with time step %g", set->Time);
  else {
    printf("endtime: %g\n", set->TimePoints[set->PrintStep]);
    printf("steps:   %d", set->PrintStep);
  }
  printf("\n");
  printf("\n");
}


/** Creates a settings structure from input values. WARNING:
    this function's type signature will change with time,
    as new settings will be required for other solvers!
*/

SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_createWith(double Time, int PrintStep, double Error, double RError, int Mxstep, int UseJacobian, int Indefinitely, int HaltOnEvent, int SteadyState, int StoreResults, int Sensitivity, int SensMethod)
{

  cvodeSettings_t *set;
  ASSIGN_NEW_MEMORY(set, struct cvodeSettings, NULL);

  /* 1. Setting SBML ODE Solver integration parameters */
  CvodeSettings_setErrors(set, Error, RError, Mxstep);
  /* set non-linear solver defaults (BDF, Newton, max.order 5*/
  set->CvodeMethod = 0;
  set->IterMethod = 0;
  set->MaxOrder = 5;
  CvodeSettings_setSwitches(set, UseJacobian, Indefinitely,
			    HaltOnEvent, SteadyState, StoreResults,
			    Sensitivity, SensMethod);

  /* 2. Setting Requested Time Series */
  /* Unless indefinite integration, generate a TimePoints array  */
  if  ( !Indefinitely ) {    
    /* ... generate default TimePoint array */
    CvodeSettings_setTime(set, Time, PrintStep);
  }
  return set;
}


/** Creates a settings structure and copies all values from input
*/

SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_clone(cvodeSettings_t *set)
{
  int i;
  cvodeSettings_t *clone;
  ASSIGN_NEW_MEMORY(clone, struct cvodeSettings, NULL);
    
  /* Setting SBML ODE Solver integration parameters */
  CvodeSettings_setErrors(clone, set->Error, set->RError, set->Mxstep);
  CvodeSettings_setSwitches(clone, set->UseJacobian, set->Indefinitely,
			    set->HaltOnEvent, set->SteadyState,
			    set->StoreResults,
			    set->Sensitivity, set->SensMethod);

  CvodeSettings_setMethod(clone, set->CvodeMethod, set->MaxOrder);
  CvodeSettings_setIterMethod(clone, set->IterMethod);
  
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


/** Sets absolute and relative error tolerances and maximum number of
    internal steps during CVODE integration 
*/

SBML_ODESOLVER_API void CvodeSettings_setErrors(cvodeSettings_t *set, double Error, double RError, int Mxstep) 
{
  CvodeSettings_setError(set, Error);
  CvodeSettings_setRError(set, RError);
  CvodeSettings_setMxstep(set, Mxstep);    
}


/** Sets absolute error tolerance 
*/

SBML_ODESOLVER_API void CvodeSettings_setError(cvodeSettings_t *set, double Error)
{
  set->Error = Error;
}


/** Sets relative error tolerance 
*/

SBML_ODESOLVER_API void CvodeSettings_setRError(cvodeSettings_t *set, double RError)
{
  set->RError = RError;
}


/** Sets maximum number of internal steps during CVODE integration
*/

SBML_ODESOLVER_API void CvodeSettings_setMxstep(cvodeSettings_t *set, int Mxstep)
{
  set->Mxstep = Mxstep;  
}


/** Set method non-linear solver methods, and its max.order
    0: BDF 1: Adams-Moulton,
    Default method is BDF
*/

SBML_ODESOLVER_API void CvodeSettings_setMethod(cvodeSettings_t *set, int i, int j)
{
  /* i == 0: default BDF method
     i == 1: Adams-Moulton method */
  if ( 0 <= i < 2 ) {
    set->CvodeMethod = i;
    set->MaxOrder = j;
  }
}


/** Set method for CVODE integration
    0: NEWTON; 1: FUNCTIONAL,
    Default method is NEWTON
*/

SBML_ODESOLVER_API void CvodeSettings_setIterMethod(cvodeSettings_t *set, int i)
{
  /* i == 0: default NEWTON iteration
     i == 1: FUNCTIONAL iteraction */
  if ( 0 <= i < 1 ) set->IterMethod = i;
  else set->IterMethod = 0;
}


/** Sets maximum order of BDF or Adams-Moulton method, respectively
   default: 5
*/

SBML_ODESOLVER_API void CvodeSettings_setMaxOrder(cvodeSettings_t *set, int MaxOrder)
{
  set->MaxOrder = MaxOrder;  
}


/** Sets integration switches in cvodeSettings. WARNING:
    this function's type signature will change with time,
    as new settings will be required for other solvers!
*/

SBML_ODESOLVER_API void CvodeSettings_setSwitches(cvodeSettings_t *set, int UseJacobian, int Indefinitely, int HaltOnEvent, int SteadyState, int StoreResults, int Sensitivity, int SensMethod)
{  
  set->UseJacobian = UseJacobian;
  set->Indefinitely = Indefinitely;
  set->HaltOnEvent = HaltOnEvent;
  set->SteadyState = SteadyState;
  set->StoreResults = StoreResults;
  CvodeSettings_setSensitivity(set, Sensitivity);
  CvodeSettings_setSensMethod(set, SensMethod);
}


/* Sets a predefined timeseries in cvodeSettings. Assigns memory for
   an array of requested time points.  PrintStep must be the size of
   the passed array timeseries. Returns 1, if sucessful and 0, if
   not. */

static int CvodeSettings_setTimeSeries(cvodeSettings_t *set, double *timeseries, int PrintStep)
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


/** Calculates a time point series from Endtime and Printstep and sets
    the time series in cvodeSettings. Returns 1, if sucessful and 0, if
    not.
*/

SBML_ODESOLVER_API int CvodeSettings_setTime(cvodeSettings_t *set, double EndTime, int PrintStep)
{
  int i, j;
  double *timeseries;
  ASSIGN_NEW_MEMORY_BLOCK(timeseries, PrintStep, double, 0);  
  for ( i=1; i<=PrintStep; i++ )
    timeseries[i-1] = i * EndTime/PrintStep;
  j = CvodeSettings_setTimeSeries(set, timeseries, PrintStep);
  free(timeseries);
  return j;
}


/** Sets the ith time step for the integration, where
    0 < i <= PrintStep.
    The first time is always 0 and can not be set.
    Returns 1, if sucessful and 0, if not.
*/

SBML_ODESOLVER_API int CvodeSettings_setTimeStep(cvodeSettings_t *set, int i, double time)
{
  if ( 0 < i <= set->PrintStep ) {
    set->TimePoints[i] = time;
    return 1;
  }
  else
    return 0;
}


/** Sets use of generated Jacobian matrix (i=1) or
    of CVODE's internal approximation (i=0). If construction
    of the Jacobian matrix fails, the internal approximation will
    be used even if i==1.
*/

SBML_ODESOLVER_API void CvodeSettings_setJacobian(cvodeSettings_t *set, int i)
{
  set->UseJacobian = i;
}


/** Sets indefinite integration (i=1). For indefinite integration
    Time will be used as integration step and PrintStep will
    be ignored.
*/

SBML_ODESOLVER_API void CvodeSettings_setIndefinitely(cvodeSettings_t *set, int i)
{ /*??? set->Time ??*/
  set->Indefinitely = i;
}


/** Sets event handling: if i==1, the integration will stop upon
    detection of an event and evaluation of event assignments;
    if i==0 the integration continues after evaluation of event
    assignments. CAUTION: the accuracy of event evaluations depends
    on the chosen printstep values!
*/

SBML_ODESOLVER_API void CvodeSettings_setHaltOnEvent(cvodeSettings_t *set, int i)
{
  set->HaltOnEvent = i;
}


/** Sets steady state handling: if i==1, the integration will stop
    upon an approximate detection of a steady state, which is here
    defined as some threshold value of the mean value and standard
    deviation of current ODE values.
*/

SBML_ODESOLVER_API void CvodeSettings_setSteadyState(cvodeSettings_t *set, int i)
{
  set->SteadyState = i;
}


/** Results will only be stored, if i==1 and if a finite integration
    has been chosen (CvodeSettings_setIndefinitely(settings, 0)). The results
    can be retrieved after integration has been finished. If i==0 or
    infinite integration has been chosen, results can only be retrieved
    during integration via variableIndex interface or dump functions for
    the integratorInstance.
*/

SBML_ODESOLVER_API void CvodeSettings_setStoreResults(cvodeSettings_t *set, int i)
{
  set->StoreResults = i;
}



/** Activate sensitivity analysis with 1; also sets to default
    sensitivity method `simultaneous' (setSensMethod(set, 0);
*/

SBML_ODESOLVER_API void CvodeSettings_setSensitivity(cvodeSettings_t *set, int i)
{
  set->Sensitivity = i;
  CvodeSettings_setSensMethod(set, 0);
}


/** Set method for sensitivity analysis:
    0: simultaneous 1: staggered, 2: staggered1.    
*/


SBML_ODESOLVER_API void CvodeSettings_setSensMethod(cvodeSettings_t *set, int i)
{
  if ( 0 <= i < 3 ) set->SensMethod = i;
  else set->SensMethod = 0;
}


/**** cvodeSettings get methods ****/

/** Returns the last time point of integration or -1, if
    Indefinitely is set to TRUE (1);   
*/

SBML_ODESOLVER_API double CvodeSettings_getEndTime(cvodeSettings_t *set)
{
  if ( !set->Indefinitely )
    return set->Time;
  else
    return -1.;
}


/** Returns the time step of integration; if infinite integration
    has been chosen, this is only the first time step.
*/

SBML_ODESOLVER_API double CvodeSettings_getTimeStep(cvodeSettings_t *set)
{
  if ( !set->Indefinitely )
    return set->TimePoints[1];
  else
    return set->Time;
}


/**  Returns the number of integration steps or -1, if
     infinite integration has been chosen
*/

SBML_ODESOLVER_API int CvodeSettings_getPrintsteps(cvodeSettings_t *set)
{
  if ( !set->Indefinitely )
    return set->PrintStep;
  else
    return -1;
}


/** Returns the time of the ith time step, where
    0 <= i < PrintStep, unless
    infinite integration has been chosen
*/

SBML_ODESOLVER_API double CvodeSettings_getTime(cvodeSettings_t *set, int i)
{
  if ( !set->Indefinitely )
    return set->TimePoints[i];
  else
    return i * set->Time;
}


/**  Returns the absolute error tolerance
*/

SBML_ODESOLVER_API double CvodeSettings_getError(cvodeSettings_t *set)
{
  return set->Error;
}


/** Returns the relative error tolerance
*/

SBML_ODESOLVER_API double CvodeSettings_getRError(cvodeSettings_t *set)
{
  return set->RError;
}


/** Returns the maximum number of internal time steps taken
    by CVODE to reach the next output time (printstep)
*/

SBML_ODESOLVER_API int CvodeSettings_getMxstep(cvodeSettings_t *set)
{
  return set->Mxstep;
}


/** Get non-linear solver method (BDF or ADAMS-MOULTON)
*/

SBML_ODESOLVER_API char *CvodeSettings_getMethod(cvodeSettings_t *set)
{
  char *meth[2];
  meth[0] = "BDF";
  meth[1] = "ADAMS-MOULTON";
  return meth[set->CvodeMethod];
}

/** Get maximum order of non-linear solver method
*/

SBML_ODESOLVER_API int CvodeSettings_getMaxOrder(cvodeSettings_t *set)
{
  return set->MaxOrder;
}

/** Get non-linear solver iteration type (NEWTON or FUNCTIONAL)
*/

SBML_ODESOLVER_API char *CvodeSettings_getIterMethod(cvodeSettings_t *set)
{
  char *meth[2];
  meth[0] = "NEWTON";
  meth[1] = "FUNCTIONAL";
  return meth[set->IterMethod];
}

/** Returns 1, if the automatically generated
    or 0 if CVODE's internal approximation
    of the jacobian matrix will be used by CVODE 
*/

SBML_ODESOLVER_API int CvodeSettings_getJacobian(cvodeSettings_t *set)
{
  return set->UseJacobian;
}


/** Returns 1, if infinite integration has been chosen,
    and 0 otherwise
*/

SBML_ODESOLVER_API int CvodeSettings_getIndefinitely(cvodeSettings_t *set)
{
  return set->Indefinitely;
}


/** Returns 1, if integration should stop upon an event trigger
    and 0 if integration should continue after evaluation of
    event assignments
*/

SBML_ODESOLVER_API int CvodeSettings_getHaltOnEvent(cvodeSettings_t *set)
{
  return set->HaltOnEvent;
}


/** Returns 1, if integration should stop upon detection of a
    steady state, and 0 if integration should continue 
*/

SBML_ODESOLVER_API int CvodeSettings_getSteadyState(cvodeSettings_t *set)
{
  return set->SteadyState;
}


/** Returns 1, if integration results should be stored internally,
    and 0 if not; If set to 0 current values can be retrieved during
    an integration loop, and the values at the end time of integration
    afterwards.
*/

SBML_ODESOLVER_API int CvodeSettings_getStoreResults(cvodeSettings_t *set)
{
  return set->StoreResults;
}


/** Returns 1, if sensitivity analysis is requested and CVODES
    will be used.
*/

SBML_ODESOLVER_API int CvodeSettings_getSensitivity(cvodeSettings_t *set)
{
  return set->Sensitivity;
}


/** Get sensitivity method 
*/

SBML_ODESOLVER_API char *CvodeSettings_getSensMethod(cvodeSettings_t *set)
{
  char *meth[3];
  meth[0] = "simultaneous";
  meth[1] = "staggered";
  meth[2] = "staggered1";
  return meth[set->SensMethod];
}


/** Frees cvodeSettings.
*/

SBML_ODESOLVER_API 
void CvodeSettings_free(cvodeSettings_t *set)
{
  if ( set->TimePoints != NULL )
    free(set->TimePoints);
  free(set);
}

/** @} */

/** Creates a settings structure from a timeSettings structure
    and fills rest with default values
*/

cvodeSettings_t *
CvodeSettings_createFromTimeSettings(timeSettings_t *time)
{
  return CvodeSettings_createWithTime(time->tend, time->nout);
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
