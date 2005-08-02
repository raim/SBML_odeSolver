/*
  Last changed Time-stamp: <2005-08-01 16:48:18 raim>
  $Id: drawGraph.h,v 1.2 2005/08/02 13:20:32 raimc Exp $
*/
#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_


/* Drawing the model with GraphViz */
int
drawModel(Model_t *m);
int
drawJacoby(cvodeData_t *data);

#endif

/* End of file */
