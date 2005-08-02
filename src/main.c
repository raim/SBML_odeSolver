/*
  Last changed Time-stamp: <2005-08-02 17:15:47 raim>
  $Id: main.c,v 1.3 2005/08/02 15:47:59 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "sbmlsolver/commandLine.h"
#include "sbmlsolver/odeSolver.h"

int
main (int argc, char *argv[]) {  
  return odeSolver (argc, argv);
}

/* End of file */
