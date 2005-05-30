/*
  Last changed Time-stamp: <2005-03-07 15:27:54 raim>
  $Id: processAST.h,v 1.1 2005/05/30 19:49:12 raimc Exp $
*/
#ifndef _PROCESSAST_H_
#define _PROCESSAST_H_

#include <sbml/SBMLTypes.h>
#include "cvodedata.h"

#define SQR(x) ((x)*(x))
#define SQRT(x) pow((x),(.5))
/* Helper Macros to get the second or the third child
   of an Abstract Syntax Tree */
#define AST_secondChild(x,y,z)  ASTNode_getChild(ASTNode_getChild(x,y),z)
#define AST_thirdChild(x,y,z,w) ASTNode_getChild(ASTNode_getChild(ASTNode_getChild(x,y),z),w)

ASTNode_t *
copyAST(const ASTNode_t *f);
double
evaluateAST(ASTNode_t *n, CvodeData data);
ASTNode_t *
differentiateAST(ASTNode_t *f, char*x);
ASTNode_t *
determinantNAST(ASTNode_t ***A, int N);  
ASTNode_t *
AST_simplify(ASTNode_t *f);
ASTNode_t *
simplifyAST(ASTNode_t *f);
void
setUserDefinedFunction(double(*udf)(char*, int, double*));

#endif

/* End of file */
