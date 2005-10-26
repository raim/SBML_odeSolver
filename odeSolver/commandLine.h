/*
  Last changed Time-stamp: <2005-10-26 15:07:26 raim>
  $Id: commandLine.h,v 1.1 2005/10/26 14:27:42 raimc Exp $
*/
#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#include <sbml/SBMLTypes.h>

#include "../src/sbmlsolver/exportdefs.h"
#include "../src/sbmlsolver/integratorInstance.h"

SBMLDocument_t* parseModelWithArguments(char *file);
int integrator(integratorInstance_t *engine, int PrintMessage,
	       int PrintOnTheFly, FILE *outfile);

SBML_ODESOLVER_API int odeSolver (int argc, char *argv[]);

#endif

/* End of file */
