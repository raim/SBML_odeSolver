/*
  Last changed Time-stamp: <2005-06-08 11:09:04 raim>
  $Id: odeSolver.h,v 1.4 2005/06/08 15:28:42 afinney Exp $
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

SBML_ODESOLVER_API int
odeSolver (int argc, char *argv[]);
SBMLResults
Model_odeSolver(SBMLDocument_t *d, CvodeSettings set);
SBMLResults *
Model_odeSolverBatch(SBMLDocument_t *d, CvodeSettings settings,
		     VarySettings vary);
SBMLResults **
Model_odeSolverBatch2 (SBMLDocument_t *d, CvodeSettings settings,
		      VarySettings vary1, VarySettings vary2);
SBMLResults
Results_fromCvode(CvodeData data);
int
Model_setValue(Model_t *m, const char *id, const char *rid, double value);

#endif

/* End of file */
