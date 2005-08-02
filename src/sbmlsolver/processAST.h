/*
  Last changed Time-stamp: <2005-08-01 16:50:56 raim>
  $Id: processAST.h,v 1.3 2005/08/02 13:20:32 raimc Exp $
*/
#ifndef _PROCESSAST_H_
#define _PROCESSAST_H_

/* libSBML header files */
#include <sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/cvodedata.h"

#define SQR(x) ((x)*(x))
#define SQRT(x) pow((x),(.5))
/* Helper Macros to get the second or the third child
   of an Abstract Syntax Tree */
#define child(x,y)  ASTNode_getChild(x,y)
#define child2(x,y,z)  ASTNode_getChild(ASTNode_getChild(x,y),z)
#define child3(x,y,z,w) ASTNode_getChild(ASTNode_getChild(ASTNode_getChild(x,y),z),w)

ASTNode_t *
copyAST(const ASTNode_t *f);
ASTNode_t *
indexAST(const ASTNode_t *f, int nvalues, char ** names);
double
evaluateAST(ASTNode_t *n, cvodeData_t *data);
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
