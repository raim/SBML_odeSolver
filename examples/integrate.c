/*
  Last changed Time-stamp: <2005-10-19 17:29:59 raim>
  $Id: integrate.c,v 1.7 2005/10/19 16:39:43 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>


int
main (int argc, char *argv[]){
  int i, j;
  char model[256];
  double time = 0.0;
  double printstep;
  
  SBMLDocument_t *d;
  SBMLReader_t *sr;
  Model_t *m;
  
  SBMLResults_t *results;
  timeCourse_t *tc;
  cvodeSettings_t *set;

   
  sscanf(argv[1], "%s", model);
  sscanf(argv[2], "%lf", &time);
  sscanf(argv[3], "%lf", &printstep);

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, time, printstep);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1000);
  
  /* calling the SBML ODE Solver which returns SBMLResults */  
  results = SBML_odeSolver(d, set);
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }

  /* now we have the results and can free the inputs */
  CvodeSettings_free(set);
  SBMLDocument_free(d);

  /* print results */
  printf("### RESULTS \n");
  SBMLResults_dump(results);
  
  /* now we can also free the result structure */
  SBMLResults_free(results);

  return (EXIT_SUCCESS);  
}



/* End of file */
