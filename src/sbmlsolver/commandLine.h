/*
  Last changed Time-stamp: <2005-08-02 17:18:21 raim>
  $Id: commandLine.h,v 1.1 2005/08/02 15:47:59 raimc Exp $
*/
#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#include <sbml/SBMLTypes.h>
#include "sbmlsolver/exportdefs.h"

SBMLDocument_t* parseModel (char *file);
SBML_ODESOLVER_API int odeSolver (int argc, char *argv[]);

#endif

/* End of file */
