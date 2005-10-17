/*
  Last changed Time-stamp: <2005-10-13 14:25:15 raim>
  $Id: drawGraph.h,v 1.4 2005/10/17 16:07:50 raimc Exp $
*/
#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_


/* Drawing the model with GraphViz */
SBML_ODESOLVER_API int drawModel(Model_t *, char *, char*);
SBML_ODESOLVER_API int drawJacoby(cvodeData_t *, char *, char*);

#endif

/* End of file */
