/*
  Last changed Time-stamp: <2005-05-30 14:52:21 raim>
  $Id: odeSolver.h,v 1.1 2005/05/30 19:49:12 raimc Exp $
*/
#ifndef _ODESOLVER_H_
#define _ODESOLVER_H_

#include<sbml/SBMLTypes.h>
#include "options.h"
#include "util.h"
#include "options.h"
#include "sbml.h"
#include "odeConstruct.h"
#include "modelSimplify.h"
#include "odeIntegrate.h"
#include "batchIntegrator.h"
#include "cvodedata.h"
#include "printModel.h"
#include "drawGraph.h"
#include "interactive.h"
#include "processAST.h"
#include "sbmlResults.h"

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
