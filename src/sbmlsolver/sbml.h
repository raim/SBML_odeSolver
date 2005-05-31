/*
  Last changed Time-stamp: <2004-09-30 16:50:25 raim>
  $Id: sbml.h,v 1.1 2005/05/31 13:54:01 raimc Exp $
*/
#ifndef _SBML_H_
#define _SBML_H_

SBMLDocument_t *
convertModel (SBMLDocument_t *d1);
SBMLReader_t*
newSBMLReader (void);
SBMLDocument_t*
parseModel (char *file);

#endif

/* End of file */
