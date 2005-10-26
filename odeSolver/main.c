/*
  Last changed Time-stamp: <2005-10-26 15:31:27 raim>
  $Id: main.c,v 1.1 2005/10/26 14:27:42 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>
#include "commandLine.h"


int
main (int argc, char *argv[]) {  
  return odeSolver (argc, argv);
}

/* End of file */
