/*
  Last changed Time-stamp: <2005-10-12 20:43:44 raim>
  $Id: commandLine.h,v 1.3 2005/10/12 18:55:01 raimc Exp $
*/
#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#include <sbml/SBMLTypes.h>
#include "sbmlsolver/exportdefs.h"
#include "sbmlsolver/integratorInstance.h"

SBMLDocument_t* parseModelWithArguments(char *file);
int integrator(integratorInstance_t *engine, int PrintMessage,
	       int PrintOnTheFly, FILE *outfile);

SBML_ODESOLVER_API int odeSolver (int argc, char *argv[]);

#endif

/* End of file */
