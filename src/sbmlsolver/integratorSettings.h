/*
  Last changed Time-stamp: <2005-11-03 10:52:14 raim>
  $Id: integratorSettings.h,v 1.10 2005/11/03 10:13:51 raimc Exp $ 
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
    int Mxstep;           /* maximum step number for CVode integration */
    int Sensitivity;      /* use CVODES for sensitivity analysis */
    int SensMethod;       /* flag for sensitivity analysis method */
    
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
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_create();
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_createWithTime(double Time, int PrintStep);
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_createWith(double EndTime, int PrintStep, double Error, double RError, int Mxstep, int UseJacobian, int Indefinitely, int HaltOnEvent, int SteadyState, int StoreResults, int Sensitivity, int SensMethod);
  SBML_ODESOLVER_API int CvodeSettings_setTime(cvodeSettings_t *, double EndTime, int PrintStep);
  SBML_ODESOLVER_API int CvodeSettings_setTimeStep(cvodeSettings_t *, int, double);
  SBML_ODESOLVER_API void CvodeSettings_setSwitches(cvodeSettings_t *, int UseJacobian, int Indefinitely, int HaltOnEvent, int SteadyState, int StoreResults, int Sensitivity, int SensMethod);
  SBML_ODESOLVER_API void CvodeSettings_setErrors(cvodeSettings_t *, double Error, double RError, int Mxstep);
  SBML_ODESOLVER_API void CvodeSettings_setError(cvodeSettings_t *, double);
  SBML_ODESOLVER_API void CvodeSettings_setRError(cvodeSettings_t *, double);
  SBML_ODESOLVER_API void CvodeSettings_setMxstep(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setJacobian(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setIndefinitely(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setHaltOnEvent(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setSteadyState(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setStoreResults(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setSensitivity(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setSensMethod(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_dump(cvodeSettings_t *);
  SBML_ODESOLVER_API void CvodeSettings_free(cvodeSettings_t *);
  SBML_ODESOLVER_API cvodeSettings_t *CvodeSettings_clone(cvodeSettings_t *);
  
  SBML_ODESOLVER_API double CvodeSettings_getEndTime(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getPrintsteps(cvodeSettings_t *);
  SBML_ODESOLVER_API double CvodeSettings_getTimeStep(cvodeSettings_t *);
  SBML_ODESOLVER_API double CvodeSettings_getTime(cvodeSettings_t *, int);  
  SBML_ODESOLVER_API double CvodeSettings_getError(cvodeSettings_t *);
  SBML_ODESOLVER_API double CvodeSettings_getRError(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getMxstep(cvodeSettings_t *);

  SBML_ODESOLVER_API int CvodeSettings_getJacobian(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getIndefinitely(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getHaltOnEvent(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getSteadyState(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getStoreResults(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getSensitivity(cvodeSettings_t *);
  SBML_ODESOLVER_API char *CvodeSettings_getSensMethod(cvodeSettings_t *);

  
#ifdef __cplusplus
}
#endif

#endif /* _CVODESETTINGS_H_ */

