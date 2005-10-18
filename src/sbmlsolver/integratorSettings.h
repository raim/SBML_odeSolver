
#include "sbmlsolver/exportdefs.h"

#ifndef _CVODESETTINGS_H_
#define _CVODESETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

  /* structures */
  typedef struct cvodeSettings cvodeSettings_t;
  typedef struct timeSettings timeSettings_t;

  /* timeSettings: start- and end times, timesteps, number of timesteps */
  struct timeSettings {
    double t0;
    double tmult;
    double tend;
    int nout;
  } ;

  /* Settings for CVODE Integration */
  struct cvodeSettings {
    double Time;          /* Time to which model is integrated or if
			     'Indefinitely' its the step size */
    int PrintStep;        /* Number of output steps from 0 to 'Time'
			     ignored if 'Indefinitely' */
    double *TimePoints;   /* Optional array of designed time-course.
			     If passed by the calling application,
			     Time will be ignored and overruled by
			     TimePoints[Printstep+1], otherwise TimePoints
			     will be calculated from Time and PrintSteps */
    int Indefinitely;     /* run without a defined end time, Time field
			     contains step duration, ignore PrintStep
			     field*/
    double Error;         /* absolute tolerance in Cvode integration */
    double RError;        /* relative tolerance in Cvode integration */
    double Mxstep;        /* maximum step number for CVode integration */
    
    int HaltOnEvent;      /* Stops integration upon an event */
    int SteadyState;      /* Stops integration upon a steady state */
    int UseJacobian;      /* Toggle use of Jacobian ASTs or approximation */
    int StoreResults;     /* Store time course history */
  } ;

  /* functions */
  /* ??might be implented?? */
  timeSettings_t *TimeSettings_create(double t0, double tend, int nout);
  void TimeSettings_free(timeSettings_t *time);
  cvodeSettings_t *CvodeSettings_createFromTimeSettings(timeSettings_t *time);

  /* create and free, get and set cvodeSettings */
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_createFromTimeSeries(double *timeseries, int n);
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_createDefaults();
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_create(double EndTime, int PrintStep, double *TimePoints, double Error, double RError, double Mxstep, int UseJacobian, int Indefinitely, int HaltOnEvent, int SteadyState, int StoreResults);
  SBML_ODESOLVER_API void CvodeSettings_free(cvodeSettings_t *set);
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_clone(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_setTime(cvodeSettings_t *set, double EndTime, int PrintStep);
  SBML_ODESOLVER_API int CvodeSettings_setTimeStep(cvodeSettings_t *set, int, double);
  SBML_ODESOLVER_API void CvodeSettings_setSwitches(cvodeSettings_t *set, int UseJacobian, int Indefinitely, int HaltOnEvent, int SteadyState, int StoreResults);
  SBML_ODESOLVER_API void CvodeSettings_setErrors(cvodeSettings_t *set, double Error, double RError, double Mxstep);
  SBML_ODESOLVER_API void CvodeSettings_setError(cvodeSettings_t *set, double Error);
  SBML_ODESOLVER_API void CvodeSettings_setRError(cvodeSettings_t *set, double RError);
  SBML_ODESOLVER_API void CvodeSettings_setMxstep(cvodeSettings_t *set, int Mxstep);
  SBML_ODESOLVER_API void CvodeSettings_setJacobian(cvodeSettings_t *set, int);
  SBML_ODESOLVER_API void CvodeSettings_setIndefinitely(cvodeSettings_t *set, int);
  SBML_ODESOLVER_API void CvodeSettings_setHaltOnEvent(cvodeSettings_t *set, int);
  SBML_ODESOLVER_API void CvodeSettings_setSteadyState(cvodeSettings_t *set, int);
  SBML_ODESOLVER_API void CvodeSettings_setStoreResults(cvodeSettings_t *set, int);
  
  SBML_ODESOLVER_API double CvodeSettings_getEndTime(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_getPrintsteps(cvodeSettings_t *set);
  SBML_ODESOLVER_API double CvodeSettings_getTimeStep(cvodeSettings_t *set);
  SBML_ODESOLVER_API double CvodeSettings_getTime(cvodeSettings_t *set, int);  
  SBML_ODESOLVER_API double CvodeSettings_getError(cvodeSettings_t *set);
  SBML_ODESOLVER_API double CvodeSettings_getRError(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_getMxstep(cvodeSettings_t *set);

  SBML_ODESOLVER_API int CvodeSettings_getJacobian(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_getIndefinitely(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_getHaltOnEvent(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_getSteadyState(cvodeSettings_t *set);
  SBML_ODESOLVER_API int CvodeSettings_getStoreResults(cvodeSettings_t *set);

  int CvodeSettings_setTimeSeries(cvodeSettings_t *set, double *TimePoints, int PrintStep);

  
#ifdef __cplusplus
}
#endif

#endif /* _CVODESETTINGS_H_ */

