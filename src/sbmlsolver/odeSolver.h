/*
  Last changed Time-stamp: <2005-05-31 12:32:48 raim>
  $Id: odeSolver.h,v 1.1 2005/05/31 13:54:01 raimc Exp $
*/
#ifndef _ODESOLVER_H_
#define _ODESOLVER_H_

/* libSBML header files */
#include<sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/options.h"
#include "sbmlsolver/util.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/batchIntegrator.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/drawGraph.h"
#include "sbmlsolver/interactive.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/sbmlResults.h"

/* Settings for batch integration with parameter variation */
typedef struct _VarySettings {
  char *id;
  double start;
  double end;
  int steps;
} VarySettings;

VarySettings vary;

int
odeSolver (int argc, char *argv[]);
SBMLResults
Model_odeSolver(SBMLDocument_t *d, CvodeSettings set);
SBMLResults *
Model_odeSolverBatch(SBMLDocument_t *d,
		     CvodeSettings settings, VarySettings vary);
SBMLResults
Results_fromCvode(CvodeData data);
int
Model_setValue(Model_t *m, const char *id, double value);

#endif

/* End of file */
