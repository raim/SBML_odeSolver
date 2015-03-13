/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Last changed Time-stamp: <2005-11-17 13:55:39 raim>
  $Id: drawGraph.h,v 1.7 2009/03/27 15:55:03 fbergmann Exp $
*/
#ifndef SBMLSOLVER_DRAWGRAPH_H_
#define SBMLSOLVER_DRAWGRAPH_H_

#include <sbml/SBMLTypes.h>
#include <sbmlsolver/cvodeData.h>
#include <sbmlsolver/exportdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

  /* Drawing the model with GraphViz */
  SBML_ODESOLVER_API int drawModel(Model_t *, char *, char*);
  SBML_ODESOLVER_API int drawJacoby(cvodeData_t *, char *, char*);
  SBML_ODESOLVER_API int drawSensitivity(cvodeData_t *, char *, char*, double);
  
#ifdef __cplusplus
}
#endif

#endif

/* End of file */
