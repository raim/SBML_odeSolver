/*
  Last changed Time-stamp: <2005-10-26 17:00:22 raim>
  $Id: odeModel.h,v 1.10 2005/10/26 15:01:02 raimc Exp $ 
*/

#ifndef _ODEMODEL_H_
#define _ODEMODEL_H_

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/odemodeldatatype.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct variableIndex variableIndex_t;

  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromFile(char *sbmlFileName, int jacobian);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromSBML2(SBMLDocument_t *d, int jacobian);
  SBML_ODESOLVER_API odeModel_t *ODEModel_create(Model_t *model, int jacobian);
  SBML_ODESOLVER_API void ODEModel_free(odeModel_t *);
  
  SBML_ODESOLVER_API int ODEModel_hasVariable(odeModel_t *, const char *symbol);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndexByNum(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getOdeVariableIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getAssignedVariableIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getConstantIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndex(odeModel_t *, const char *symbol);
  SBML_ODESOLVER_API const char *ODEModel_getVariableName(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getOde(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getAssignment(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *);

  SBML_ODESOLVER_API int ODEModel_getNeq(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumAssignments(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumConstants(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumValues(odeModel_t *);
  SBML_ODESOLVER_API const Model_t *ODEModel_getModel(odeModel_t *);

  SBML_ODESOLVER_API void ODEModel_dumpNames(odeModel_t *);
  
  void ODEModel_constructJacobian(odeModel_t *);
  
  SBML_ODESOLVER_API ASTNode_t *ODEModel_constructDeterminant(odeModel_t *om);
 
#ifdef __cplusplus
}
#endif

#endif
