/*
  Last changed Time-stamp: <2005-10-12 14:49:26 raim>
  $Id: odeSolver.h,v 1.8 2005/10/12 12:52:09 raimc Exp $
*/
#ifndef _ODESOLVER_H_
#define _ODESOLVER_H_

/* libSBML header files */
#include<sbml/SBMLTypes.h>

/* own header files */

#include "sbmlsolver/util.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/batchIntegrator.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/drawGraph.h"
#include "sbmlsolver/interactive.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/sbmlResults.h"
#include "sbmlsolver/exportdefs.h"

typedef struct _VarySettings {
  char *id;         /* SBML ID of the species, compartment or parameter
                      to be varied */
  char *rid;        /* SBML Reaction ID, if a local parameter is to be
                      varied */ 
  double start;     /* start value for parameter variation */
  double end;       /* end value for parameter variation */
  int steps;        /* number of steps for parameter variation */
} VarySettings;

/* Settings for batch integration with parameter variation */
typedef struct varySettings varySettings_t;

struct varySettings {
  int nrdesingpoints; /*defines how many design points are set*/
  int nrparams;
  char **id;          /* array of SBML ID of the species, compartment
			 or parameter to be varied */
  char **rid;         /* SBML Reaction ID, if a local parameter is to
			 be varied */
  double **params;    /* two dimensional array with the parmaters */
  int charsize;       /* maximal length of character string*/
  
};


SBMLResults_t *
Model_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set);
SBMLResults_t **
Model_odeSolverBatch(SBMLDocument_t *d, cvodeSettings_t *set,
		     VarySettings vary);
SBMLResults_t ***
Model_odeSolverBatch2 (SBMLDocument_t *d, cvodeSettings_t *set,
		      VarySettings vary1, VarySettings vary2);
SBMLResults_t *
SBMLResults_fromIntegrator(Model_t *m, integratorInstance_t *ii);

/* settings for parameter variation batch runs */
varySettings_t *varySettings_create();
int varySettings_addParameter(char *id, char *rid, int steps,
			      double start, double end);
int varySettings_addParameterDesign(char *id, char *rid, double *);
void varySettings_free();


int
Model_setValue(Model_t *m, const char *id, const char *rid, double value);
int updateModel(cvodeData_t *data, int nout);
#endif

/* End of file */
