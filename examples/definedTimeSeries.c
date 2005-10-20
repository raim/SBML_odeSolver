/*
  Last changed Time-stamp: <2005-10-20 15:50:45 raim>
  $Id: definedTimeSeries.c,v 1.6 2005/10/20 15:36:24 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>

int
main (int argc, char *argv[]){
  int i, j;
  char model[256];
  
  SBMLDocument_t *d;
  SBMLReader_t *sr;
  SBMLResults_t *results;
  timeCourse_t *tc;
  cvodeSettings_t *set;

  double printstep = 6;
  double endtime = 25;
   
  sscanf(argv[1], "%s", model);

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);
  
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1000);
  CvodeSettings_setTime(set, endtime, printstep);
 
  /* writing predefined output times */
  CvodeSettings_setTimeStep(set, 1, 0.5);
  for ( i=2; i<=printstep; i++ ) 
    CvodeSettings_setTimeStep(set, i, (i-1)*(i-1));

 /* printing integration settings */
  CvodeSettings_dump(set);
  
  /* calling the SBML ODE Solver, and retrieving SBMLResults */  
  results = SBML_odeSolver(d, set);
  
  CvodeSettings_free(set);
  SBMLDocument_free(d);
  
  if ( results == NULL ) {
    printf("Integration not sucessful!\n");
    return (EXIT_FAILURE);  
  }

  /* printing results only for species*/
  SBMLResults_dumpSpecies(results);

  /* now we can also free the result structure */
  SBMLResults_free(results);

  return (EXIT_SUCCESS);  
}

/* End of file */
