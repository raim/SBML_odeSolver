/*
  Last changed Time-stamp: <2005-05-26 11:25:44 raim>
  $Id: odeConstruct.h,v 1.3 2005/07/26 15:43:35 afinney Exp $
*/
#ifndef _ODECONSTRUCT_H_
#define _ODECONSTRUCT_H_

#include "cvodedata.h"

double
Model_getValueById(Model_t *m, const char *id);

Model_t*
Model_reduceToOdes(Model_t *m, int simplify);
ASTNode_t *
Species_odeFromReactions(Species_t *s, Model_t *m);
void
ODEs_constructJacobian(odeModel_t *, int determinant);
#endif

/* End of file */
