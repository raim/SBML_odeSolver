/*
  Last changed Time-stamp: <2005-08-02 17:29:02 raim>
  $Id: odeSolver.h,v 1.7 2005/08/02 15:47:59 raimc Exp $
*/
#ifndef _ODESOLVER_H_
#define _ODESOLVER_H_

/* libSBML header files */
#include<sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/options.h"
#include "sbmlsolver/util.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/batchIntegrator.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/drawGraph.h"
#include "sbmlsolver/interactive.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/sbmlResults.h"
#include "sbmlsolver/exportdefs.h"

/* Settings for batch integration with parameter variation */
typedef struct _VarySettings {
  char *id;         /* SBML ID of the species, compartment or parameter
		       to be varied */
  char *rid;        /* SBML Reaction ID, if a local parameter is to be
		       varied */ 
  double start;     /* start value for parameter variation */
  double end;       /* end value for parameter variation */
  int steps;        /* number of steps for parameter variation */
} VarySettings;

VarySettings vary;


SBMLResults_t *
Model_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set);
SBMLResults_t **
Model_odeSolverBatch(SBMLDocument_t *d, cvodeSettings_t *set,
		     VarySettings vary);
SBMLResults_t ***
Model_odeSolverBatch2 (SBMLDocument_t *d, cvodeSettings_t *set,
		      VarySettings vary1, VarySettings vary2);
SBMLResults_t *
Results_fromCvode(cvodeData_t *data);
int
Model_setValue(Model_t *m, const char *id, const char *rid, double value);
int updateModel(cvodeData_t *data, int nout);
#endif

/* End of file */
