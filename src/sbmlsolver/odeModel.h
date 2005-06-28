#ifndef _ODEMODEL_H_
#define _ODEMODEL_H_

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/odemodeldatatype.h"

typedef struct variableIndex variableIndex_t;

SBML_ODESOLVER_API odeModel_t *ODEModel_create(char *sbmlFileName);
SBML_ODESOLVER_API odeModel_t *ODEModel_createFromModel(Model_t *model);
SBML_ODESOLVER_API odeModel_t *ODEModel_createFromModelAndOptions(Model_t *model, int simplify, int determinant, const char *parameterNotToBeReplaced);
SBML_ODESOLVER_API void ODEModel_free(odeModel_t *);

SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndex(odeModel_t *, char *symbol);
SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *);

#endif
