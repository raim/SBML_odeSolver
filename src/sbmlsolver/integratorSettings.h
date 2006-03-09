/*
  Last changed Time-stamp: <2005-12-15 20:38:57 raim>
  $Id: integratorSettings.h,v 1.15 2006/03/09 17:23:50 afinney Exp $ 
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

#include "sbmlsolver/cvodeSettingsStruct.h"

  /* structures */
  typedef struct cvodeSettings cvodeSettings_t;
  typedef struct timeSettings timeSettings_t;

  /** NOT USED CURRENTLY!
      timeSettings: start- and end times, timesteps, number of timesteps */
  struct timeSettings {
    double t0;
    double tmult;
    double tend;
    int nout;
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
  SBML_ODESOLVER_API void CvodeSettings_setCompileFunctions(cvodeSettings_t *, int);

  /* Adjoint setttings */
  SBML_ODESOLVER_API void CvodeSettings_setDoAdj(cvodeSettings_t *);
  SBML_ODESOLVER_API void CvodeSettings_setAdjPhase(cvodeSettings_t *); 
  SBML_ODESOLVER_API void CvodeSettings_setAdjErrors(cvodeSettings_t *, double Error, double RError);
  SBML_ODESOLVER_API void CvodeSettings_setAdjError(cvodeSettings_t *, double);
  SBML_ODESOLVER_API void CvodeSettings_setAdjRError(cvodeSettings_t *, double);
  SBML_ODESOLVER_API void CvodeSettings_setnSaveSteps(cvodeSettings_t *, int);
  SBML_ODESOLVER_API int CvodeSettings_setAdjTime(cvodeSettings_t *, double EndTime, int PrintStep);


  SBML_ODESOLVER_API void CvodeSettings_setMethod(cvodeSettings_t *, int, int);
  SBML_ODESOLVER_API void CvodeSettings_setIterMethod(cvodeSettings_t *, int);
  SBML_ODESOLVER_API void CvodeSettings_setMaxOrder(cvodeSettings_t *, int);
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
  SBML_ODESOLVER_API char *CvodeSettings_getMethod(cvodeSettings_t *);
  SBML_ODESOLVER_API char *CvodeSettings_getIterMethod(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getMaxOrder(cvodeSettings_t *);
  SBML_ODESOLVER_API int CvodeSettings_getCompileFunctions(cvodeSettings_t *);

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

