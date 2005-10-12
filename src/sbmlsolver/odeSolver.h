/*
  Last changed Time-stamp: <2005-10-12 20:53:21 raim>
  $Id: odeSolver.h,v 1.10 2005/10/12 18:55:01 raimc Exp $
*/
#ifndef _ODESOLVER_H_
#define _ODESOLVER_H_

/* libSBML header files */
#include<sbml/SBMLTypes.h>

/* own header files */

#include "sbmlsolver/util.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/modelSimplify.h"
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
  int nrdesignpoints; /*defines how many design points are set*/
  int nrparams;
  char **id;          /* array of SBML ID of the species, compartment
			 or parameter to be varied */
  double **params;    /* two dimensional array with the parmaters */
};


SBMLResults_t *
Model_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set);
SBMLResults_t ***
Model_odeSolverBatch(SBMLDocument_t *d, cvodeSettings_t *set,
		     varySettings_t *vary);
SBMLResults_t ***
Model_odeSolverBatch2 (SBMLDocument_t *d, cvodeSettings_t *set,
		      VarySettings vary1, VarySettings vary2);
SBMLResults_t *
SBMLResults_fromIntegrator(Model_t *m, integratorInstance_t *ii);

/* settings for parameter variation batch runs */
varySettings_t *VarySettings_create(int nrparams, int nrdesignpoints);
int VarySettings_addParameterSeries(varySettings_t *vs, char *id, char *rid,
				    double *designpoints);
int VarySettings_addParameter(varySettings_t *, char *id, char *rid,
			      double start, double end);
int VarySettings_setParameterName(varySettings_t *vs, int i,
				  char *id, char *rid);
void VarySettings_dump(varySettings_t *);
void VarySettings_free();


int
Model_setValue(Model_t *m, const char *id, const char *rid, double value);
int updateModel(cvodeData_t *data, int nout);
#endif

/* End of file */
