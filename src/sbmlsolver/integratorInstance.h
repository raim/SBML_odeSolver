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

  /* integrator state information */
  struct cvodeSolver
  {
    realtype reltol, t, tout, atol1, rtol1, t0, t1, tmult;
    N_Vector y, abstol;
    void *cvode_mem;
    int iout, nout;    
  };

  struct integratorInstance
  {
    /* passed to integratorInstance */
    odeModel_t *om;
    cvodeSettings_t *opt;
    /* created with integratorInstance from om and opt */
    cvodeData_t *data;
    cvodeSolver_t *cv;
    cvodeResults_t *results; 
  };


  SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *, cvodeSettings_t *);
  SBML_ODESOLVER_API int IntegratorInstance_set(integratorInstance_t *, cvodeSettings_t *);
  SBML_ODESOLVER_API int IntegratorInstance_reset(integratorInstance_t *); 

  SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *);
  SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *, variableIndex_t *);
  SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *, variableIndex_t *, double);
  SBML_ODESOLVER_API int IntegratorInstance_timeCourseCompleted(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_copyVariableState(integratorInstance_t *target, integratorInstance_t *source);
  SBML_ODESOLVER_API int IntegratorInstance_integrateOneStep(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_integrate(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *);
  SBML_ODESOLVER_API int IntegratorInstance_handleError(integratorInstance_t *);
  SBML_ODESOLVER_API cvodeResults_t *IntegratorInstance_createResults(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_printStatistics(integratorInstance_t *);

  /* internal functions that are not part of the API (yet?) */
  int IntegratorInstance_createODESolverStructures(integratorInstance_t *);
  void IntegratorInstance_freeODESolverStructures(cvodeSolver_t *);
  int IntegratorInstance_initializeSolver(integratorInstance_t *, cvodeData_t *, cvodeSettings_t *, odeModel_t *);
  void IntegratorInstance_setNextTimeStep(integratorInstance_t *, double);


  
#ifdef __cplusplus
}
#endif

#endif
