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

typedef struct integratorInstance integratorInstance_t ;

/* integrator state information */
struct integratorInstance
{
  realtype reltol, t, tout, atol1, rtol1, t0, t1, tmult;
  N_Vector y, abstol;
  void *cvode_mem;
  int iout, nout;
  cvodeResults_t *results; 
  cvodeData_t *data;
};
  
SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *, cvodeSettings_t *);
SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *);
SBML_ODESOLVER_API int IntegratorInstance_integrateOneStep(integratorInstance_t *);
SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *);
SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *, variableIndex_t *);
SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *, variableIndex_t *, double value);
SBML_ODESOLVER_API int IntegratorInstance_timeCourseCompleted(integratorInstance_t *);
SBML_ODESOLVER_API void IntegratorInstance_copyVariableState(integratorInstance_t *target, integratorInstance_t *source);

/* internal functions that are not part of the API (yet?) */
int IntegratorInstance_handleError(integratorInstance_t *engine);
integratorInstance_t *IntegratorInstance_createFromCvodeData(cvodeData_t *data);
void IntegratorInstance_freeExcludingCvodeData(integratorInstance_t *engine);
void IntegratorInstance_printStatistics(integratorInstance_t *engine);
  
#ifdef __cplusplus
}
#endif

#endif
