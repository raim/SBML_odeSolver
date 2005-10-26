/*
  Last changed Time-stamp: <2005-10-26 12:48:15 raim>
  $Id: cvodeSolver.h,v 1.1 2005/10/26 12:36:50 raimc Exp $
*/
#ifndef _CVODESOLVER_H_
#define _CVODESOLVER_H_

/* Header Files for CVODE */
#include "cvode.h"

#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/integratorInstance.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* CVODE SOLVER */
  SBML_ODESOLVER_API int IntegratorInstance_cvodeOneStep(integratorInstance_t *);
  SBML_ODESOLVER_API void IntegratorInstance_printCVODEStatistics(integratorInstance_t *, FILE *f);

  /* internal functions that are not part of the API (yet?) */
  int IntegratorInstance_createCVODESolverStructures(integratorInstance_t *);
  void IntegratorInstance_freeCVODESolverStructures(cvodeSolver_t *);

#ifdef __cplusplus
}
#endif

#endif

/* End of file */
