/*
  Last changed Time-stamp: <2005-07-28 15:04:28 raim>
  $Id: odeConstruct.h,v 1.4 2005/08/02 13:20:32 raimc Exp $
*/
#ifndef _ODECONSTRUCT_H_
#define _ODECONSTRUCT_H_

#include "cvodedata.h"

double
Model_getValueById(Model_t *m, const char *id);

Model_t*
Model_reduceToOdes(Model_t *m);
ASTNode_t *
Species_odeFromReactions(Species_t *s, Model_t *m);
void
ODEs_constructJacobian(odeModel_t *);
#endif

/* End of file */
