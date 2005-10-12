/*
  Last changed Time-stamp: <2005-10-05 13:22:56 raim>
  $Id: odeConstruct.h,v 1.5 2005/10/12 12:52:09 raimc Exp $
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

#endif

/* End of file */
