/*
  Last changed Time-stamp: <2005-10-12 20:43:00 raim>
  $Id: drawGraph.h,v 1.3 2005/10/12 18:55:01 raimc Exp $
*/
#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_


/* Drawing the model with GraphViz */
SBML_ODESOLVER_API int drawModel(Model_t *m);
SBML_ODESOLVER_API int drawJacoby(cvodeData_t *data);

#endif

/* End of file */
