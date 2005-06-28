#ifndef _INTEGRATORINSTANCE_H_
#define _INTEGRATORINSTANCE_H_

#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/cvodeSettings.h"
#include "sbmlsolver/cvodedatatype.h"
#include "sbmlsolver/odeModel.h"

typedef struct integratorInstance integratorInstance_t ;

SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *, CvodeSettings *);
SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *);
SBML_ODESOLVER_API int IntegratorInstance_integrateOneStep(integratorInstance_t *);
SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *);
SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *, variableIndex_t *);
SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *, variableIndex_t *, double value);
SBML_ODESOLVER_API int IntegratorInstance_timeCourseCompleted(integratorInstance_t *);

/* internal functions that are not part of the API (yet?) */
int IntegratorInstance_handleError(integratorInstance_t *engine);
integratorInstance_t *IntegratorInstance_createFromCvodeData(CvodeData data);
void IntegratorInstance_freeExcludingCvodeData(integratorInstance_t *engine);
void IntegratorInstance_printStatistics(integratorInstance_t *engine);
#endif
