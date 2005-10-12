/*
  Last changed Time-stamp: <2005-10-12 21:00:51 raim>
  $Id: odeConstruct.h,v 1.7 2005/10/12 19:49:23 raimc Exp $
*/
#ifndef _ODECONSTRUCT_H_
#define _ODECONSTRUCT_H_

#include "cvodedata.h"

SBML_ODESOLVER_API double Model_getValueById(Model_t *m, const char *id);
SBML_ODESOLVER_API int Model_setValue(Model_t *m, const char *id, const char *rid, double value);
SBML_ODESOLVER_API Model_t* Model_reduceToOdes(Model_t *m);
SBML_ODESOLVER_API ASTNode_t *Species_odeFromReactions(Species_t *s, Model_t *m);

#endif

/* End of file */
