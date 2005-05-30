/*
  Last changed Time-stamp: <2005-05-26 11:25:44 raim>
  $Id: odeConstruct.h,v 1.1 2005/05/30 19:49:13 raimc Exp $
*/
#ifndef _ODECONSTRUCT_H_
#define _ODECONSTRUCT_H_

#include "cvodedata.h"

CvodeData 
constructODEs (Model_t *m);
double
Model_getValueById(Model_t *m, const char *id);

Model_t*
Model_reduceToOdes(Model_t *m);
ASTNode_t *
Species_odeFromReactions(Species_t *s, Model_t *m);

#endif

/* End of file */
