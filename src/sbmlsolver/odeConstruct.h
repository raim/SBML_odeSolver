/*
  Last changed Time-stamp: <2005-10-12 20:42:14 raim>
  $Id: odeConstruct.h,v 1.6 2005/10/12 18:55:01 raimc Exp $
*/
#ifndef _ODECONSTRUCT_H_
#define _ODECONSTRUCT_H_

#include "cvodedata.h"

SBML_ODESOLVER_API double Model_getValueById(Model_t *m, const char *id);
SBML_ODESOLVER_API Model_t* Model_reduceToOdes(Model_t *m);
SBML_ODESOLVER_API ASTNode_t *Species_odeFromReactions(Species_t *s, Model_t *m);

#endif

/* End of file */
