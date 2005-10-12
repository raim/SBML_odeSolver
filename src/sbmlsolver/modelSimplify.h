/*
  Last changed Time-stamp: <2005-10-12 20:45:07 raim>
  $Id: modelSimplify.h,v 1.2 2005/10/12 18:55:01 raimc Exp $
*/
#ifndef _MODEL_H_
#define _MODEL_H_

SBML_ODESOLVER_API void AST_replaceNameByFormula(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceNameByValue(ASTNode_t *math, const char *name, double x);
SBML_ODESOLVER_API void AST_replaceNameByParameters(ASTNode_t *math, ListOf_t* lp);
SBML_ODESOLVER_API void AST_replaceFunctionDefinition(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceConstants(Model_t *m, ASTNode_t *math);

#endif

/* End of file */
