/*
  Last changed Time-stamp: <>
  $Id: integratorSettings.c,v 1.1 2005/08/02 13:21:16 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "sbmlsolver/integratorSettings.h"

timeSettings_t *
TimeSettings_createWith(double t0, double tend, int nout) {

  timeSettings_t *time;
  
  time = (timeSettings_t *)calloc(1, sizeof(timeSettings_t));

  time->t0 = t0;
  time->tend = tend;
  time->nout = nout;
  time->tmult = (t0-tend) / nout;
  return time;				  

}

void
TimeSettings_free(timeSettings_t *time) {
  free(time);
}

cvodeSettings_t *
CvodeSettings_createDefaults() {

  return CvodeSettings_createWith(1., 1, 1e-10, 1e-10, 500,
				  1, 0, 0, 0, 0, 1);
}

cvodeSettings_t *
CvodeSettings_createWith(double EndTime, int PrintStep,
			 double Error, double RError, double Mxstep,
			 int UseJacobian, int EnableVariableChanges,
			 int Indefinitely, int HaltOnEvent,
			 int SteadyState, int StoreResults) {
  
  cvodeSettings_t *set;
  set = (cvodeSettings_t *)calloc(1, sizeof(cvodeSettings_t));
  
  /* Setting SBML ODE Solver integration parameters */
  set->Time = EndTime;
  set->PrintStep = PrintStep;  
  set->Error = Error;
  set->RError = RError;
  set->Mxstep = Mxstep;
  set->HaltOnEvent = HaltOnEvent;
  set->SteadyState = SteadyState;
  set->UseJacobian = UseJacobian;
  set->StoreResults = StoreResults;
  set->Indefinitely = Indefinitely;
  set->EnableVariableChanges = EnableVariableChanges;

  return set;
}

void
CvodeSettings_free(cvodeSettings_t *set) {
  free(set);
}



/* End of file */
