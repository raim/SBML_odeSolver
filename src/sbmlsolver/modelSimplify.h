/*
  Last changed Time-stamp: <2005-10-18 13:28:59 raim>
  $Id: modelSimplify.h,v 1.4 2005/10/18 14:17:32 raimc Exp $
*/
#ifndef _MODEL_H_
#define _MODEL_H_

#include "sbmlsolver/exportdefs.h"

SBML_ODESOLVER_API void AST_replaceNameByFormula(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceNameByName(ASTNode_t *math, const char *name, const char *newname);
SBML_ODESOLVER_API void AST_replaceNameByValue(ASTNode_t *math, const char *name, double x);
SBML_ODESOLVER_API void AST_replaceNameByParameters(ASTNode_t *math, ListOf_t* lp);
SBML_ODESOLVER_API void AST_replaceFunctionDefinition(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceConstants(Model_t *m, ASTNode_t *math);

#endif

/* End of file */
