/*
  Last changed Time-stamp: <2005-10-27 12:42:46 raim>
  $Id: drawGraph.h,v 1.5 2005/10/27 12:36:13 raimc Exp $
*/
#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_

#include "sbmlsolver/exportdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* Drawing the model with GraphViz */
  SBML_ODESOLVER_API int drawModel(Model_t *, char *, char*);
  SBML_ODESOLVER_API int drawJacoby(cvodeData_t *, char *, char*);
  
#ifdef __cplusplus
}
#endif

#endif

/* End of file */
