/*
  Last changed Time-stamp: <2005-10-12 21:31:28 raim>
  $Id: modelSimplify.h,v 1.3 2005/10/12 19:49:23 raimc Exp $
*/
#ifndef _MODEL_H_
#define _MODEL_H_

#include "sbmlsolver/exportdefs.h"

SBML_ODESOLVER_API void AST_replaceNameByFormula(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceNameByValue(ASTNode_t *math, const char *name, double x);
SBML_ODESOLVER_API void AST_replaceNameByParameters(ASTNode_t *math, ListOf_t* lp);
SBML_ODESOLVER_API void AST_replaceFunctionDefinition(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceConstants(Model_t *m, ASTNode_t *math);

#endif

/* End of file */
