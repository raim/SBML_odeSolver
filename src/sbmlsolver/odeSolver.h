/*
  Last changed Time-stamp: <2005-10-26 15:36:44 raim>
  $Id: odeSolver.h,v 1.16 2005/10/26 14:27:42 raimc Exp $
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
#include "sbmlsolver/drawGraph.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/sbmlResults.h"
#include "sbmlsolver/exportdefs.h"


/* Settings for batch integration with parameter variation */
typedef struct varySettings varySettings_t;

struct varySettings {
  int nrdesignpoints; /*defines how many design points are set*/
  int nrparams;       /* defines the number of parameters to be varied */
  int cnt_params;     /* counts the number of parameters to be varied */
  char **id;          /* array of SBML ID of the species, compartment
			 or parameter to be varied */
  char **rid;         /* SBML Reaction ID, if a local parameter is to be
			 varied */
  double **params;    /* two dimensional array with the parmaters */
};

SBML_ODESOLVER_API SBMLResults_t *SBML_odeSolver(SBMLDocument_t *, cvodeSettings_t *);
SBML_ODESOLVER_API SBMLResultsMatrix_t *SBML_odeSolverBatch(SBMLDocument_t *, cvodeSettings_t *, varySettings_t *);
SBML_ODESOLVER_API SBMLResults_t *Model_odeSolver(Model_t *, cvodeSettings_t *);
SBML_ODESOLVER_API SBMLResultsMatrix_t *Model_odeSolverBatch(Model_t *, cvodeSettings_t *, varySettings_t *);
SBML_ODESOLVER_API SBMLResults_t *SBMLResults_fromIntegrator(Model_t *, integratorInstance_t *);

/* settings for parameter variation batch runs */
SBML_ODESOLVER_API varySettings_t *VarySettings_allocate(int nrparams, int nrdesignpoints);
SBML_ODESOLVER_API int VarySettings_addParameter(varySettings_t *, char *id, char *rid, double start, double end);
SBML_ODESOLVER_API int VarySettings_setName(varySettings_t *, int i, char *id, char *rid);
SBML_ODESOLVER_API void VarySettings_setValue(varySettings_t *, int i, int j, double value);

SBML_ODESOLVER_API double VarySettings_getValue(varySettings_t *, int i, int j);
SBML_ODESOLVER_API const char *VarySettings_getName(varySettings_t *, int i);
SBML_ODESOLVER_API const char *VarySettings_getReactionName(varySettings_t *vs, int i);

SBML_ODESOLVER_API void VarySettings_dump(varySettings_t *);
SBML_ODESOLVER_API void VarySettings_free();

/* not part of official API */
int VarySettings_addParameterSeries(varySettings_t *, char *id, char *rid, double *designpoints);
int VarySettings_addParameterSet(varySettings_t *,
				 double **designpoints, char **id, char **rid);

#endif

/* End of file */
