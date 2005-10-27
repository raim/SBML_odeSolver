/*
  Last changed Time-stamp: <2005-10-27 19:51:03 raim>
  $Id: integratorInstance.h,v 1.15 2005/10/27 17:58:51 raimc Exp $ 
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
 *     Andrew Finney
 *
 * Contributor(s):
 *     Rainer Machne
 */

#ifndef _INTEGRATORINSTANCE_H_
#define _INTEGRATORINSTANCE_H_

/* Header Files for CVODE */
#include "cvode.h"    

#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/cvodedatatype.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/cvodedata.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct cvodeSolver cvodeSolver_t;
  typedef struct integratorInstance integratorInstance_t ;

  /* CVODE integrator state information */
  struct cvodeSolver
  {
    /* these data are required by the functions common to all solvers */
    double t, tout, t0;
    int iout, nout;
    /* these data are only used by the CVODE solver specific functions */
    realtype reltol, atol1;
    N_Vector y, abstol;
    void *cvode_mem;
  };


  struct integratorInstance
  {
    /* passed to integratorInstance */
    odeModel_t *om;
    cvodeSettings_t *opt;
    /* created with integratorInstance from om and opt */
    cvodeData_t *data;
    /* alternative solver structures */
    cvodeSolver_t *solver;
    /* optional results */
    cvodeResults_t *results; 
  };
  
  /* common to all solvers */
  SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *, cvodeSettings_t *);
  SBML_ODESOLVER_API int IntegratorInstance_set(integratorInstance_t *, cvodeSettings_t *);
  SBML_ODESOLVER_API int IntegratorInstance_reset(integratorInstance_t *);
  SBML_ODESOLVER_API cvodeSettings_t *IntegratorInstance_getSettings(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_copyVariableState(integratorInstance_t *target, integratorInstance_t *source);
  SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *);
  SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *, variableIndex_t *);
  SBML_ODESOLVER_API int IntegratorInstance_setNextTimeStep(integratorInstance_t *, double);
  SBML_ODESOLVER_API void IntegratorInstance_dumpNames(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_dumpData(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_integrate(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_checkTrigger(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_checkSteadyState(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_timeCourseCompleted(integratorInstance_t *);
  SBML_ODESOLVER_API cvodeResults_t *IntegratorInstance_createResults(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_updateModel(integratorInstance_t*);
  SBML_ODESOLVER_API int IntegratorInstance_simpleOneStep(integratorInstance_t *);
  
  /* these functions contain solver specific switches and need to be adapted
     for any new solver, and so does the local
     integratorInstance_initialiyeSolverStructures */    
  SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *, variableIndex_t *, double);
  SBML_ODESOLVER_API int IntegratorInstance_integrateOneStep(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_dumpSolver(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_handleError(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_printStatistics(integratorInstance_t *, FILE *f);
  
  
#ifdef __cplusplus
}
#endif

/* default function for data update, event and steady state handling,
   result storage and loop variables; to be used by solver
   specific ...OneStep functions */
int IntegratorInstance_updateData(integratorInstance_t *);

#endif
