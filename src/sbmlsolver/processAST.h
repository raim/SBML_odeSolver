/*
  Last changed Time-stamp: <2005-10-12 21:18:09 raim>
  $Id: processAST.h,v 1.4 2005/10/12 19:49:23 raimc Exp $
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

SBML_ODESOLVER_API double evaluateAST(ASTNode_t *n, cvodeData_t *data);
SBML_ODESOLVER_API ASTNode_t *differentiateAST(ASTNode_t *f, char*x);
SBML_ODESOLVER_API ASTNode_t *AST_simplify(ASTNode_t *f);
SBML_ODESOLVER_API void setUserDefinedFunction(double(*udf)(char*, int, double*));

ASTNode_t *copyAST(const ASTNode_t *f);
ASTNode_t *indexAST(const ASTNode_t *f, int nvalues, char ** names);
ASTNode_t *determinantNAST(ASTNode_t ***A, int N);  
ASTNode_t *simplifyAST(ASTNode_t *f);


#endif

/* End of file */
