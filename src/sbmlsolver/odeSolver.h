/*
  Last changed Time-stamp: <2005-10-17 17:29:52 raim>
  $Id: odeSolver.h,v 1.13 2005/10/17 16:07:50 raimc Exp $
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
  int nrparams;       /* defines the number of parameters to be varied */
  int cnt_params;       /* defines the number of parameters to be varied */
  char **id;          /* array of SBML ID of the species, compartment
			 or parameter to be varied */
  double **params;    /* two dimensional array with the parmaters */
};

SBML_ODESOLVER_API SBMLResults_t *SBML_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set);
SBML_ODESOLVER_API SBMLResultsMatrix_t *SBML_odeSolverBatch(SBMLDocument_t *d, cvodeSettings_t *set, varySettings_t *vary);
SBML_ODESOLVER_API SBMLResults_t *Model_odeSolver(Model_t *d, cvodeSettings_t *set);
SBML_ODESOLVER_API SBMLResultsMatrix_t *Model_odeSolverBatch(Model_t *d, cvodeSettings_t *set, varySettings_t *vary);
SBML_ODESOLVER_API SBMLResults_t *SBMLResults_fromIntegrator(Model_t *m, integratorInstance_t *ii);

/* settings for parameter variation batch runs */
SBML_ODESOLVER_API varySettings_t *VarySettings_allocate(int nrparams, int nrdesignpoints);
SBML_ODESOLVER_API int VarySettings_addParameterSeries(varySettings_t *, char *id, char *rid, double *designpoints);
SBML_ODESOLVER_API int VarySettings_addParameter(varySettings_t *, char *id, char *rid, double start, double end);

SBML_ODESOLVER_API int VarySettings_setParameterName(varySettings_t *vs, int i, char *id, char *rid);
SBML_ODESOLVER_API const char *VarySettings_getParameterName(varySettings_t *vs, int i);
SBML_ODESOLVER_API void VarySettings_setValue(varySettings_t *vs, int i, int j, double value);
SBML_ODESOLVER_API double VarySettings_getValue(varySettings_t *vs, int i, int j);

SBML_ODESOLVER_API void VarySettings_dump(varySettings_t *);
SBML_ODESOLVER_API void VarySettings_free();


#endif

/* End of file */
