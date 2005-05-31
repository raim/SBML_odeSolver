/*
  Last changed Time-stamp: <2004-06-29 17:48:46 raim>
  $Id: drawGraph.h,v 1.1 2005/05/31 13:54:01 raimc Exp $
*/
#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_


/* Drawing the model with GraphViz */
int
drawModel(Model_t *m);
int
drawJacoby(CvodeData data);

#endif

/* End of file */
