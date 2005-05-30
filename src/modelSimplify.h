/*
  Last changed Time-stamp: <2004-11-14 15:36:36 raim>
  $Id: modelSimplify.h,v 1.1 2005/05/30 19:49:12 raimc Exp $
*/
#ifndef _MODEL_H_
#define _MODEL_H_

void
AST_replaceNameByFormula(ASTNode_t *math, const char *name,
			 const ASTNode_t *formula);
void
AST_replaceNameByValue(ASTNode_t *math, const char *name, double x);
void
AST_replaceNameByParameters(ASTNode_t *math, ListOf_t* lp);
void
AST_replaceFunctionDefinition(ASTNode_t *math, const char *name,
			 const ASTNode_t *formula);
void
AST_replaceConstants(Model_t *m, ASTNode_t *math);

#endif

/* End of file */
