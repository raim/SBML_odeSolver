/*
  Last changed Time-stamp: <2005-10-17 17:32:32 raim>
  $Id: definedTimeSeries.c,v 1.2 2005/10/17 16:08:37 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>

int
main (int argc, char *argv[]){
  int i, j;
  char model[256];
  double printstep = 6;
  double *timepoints;
  
  SBMLDocument_t *d;
  SBMLReader_t *sr;
  SBMLResults_t *results;
  cvodeSettings_t *set;

   
  sscanf(argv[1], "%s", model);

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);
  
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_createDefaults();
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1000);

  /* generating predefined output times */
  timepoints = (double *)calloc(printstep, sizeof(double));
  timepoints[0] = 0.5;
  for ( i=1; i<printstep; i++ ) {
    timepoints[i] = i*i;
  }
  CvodeSettings_setTimeSeries(set, timepoints, printstep);
  /* the array was copied and can now be freed */
  free(timepoints);
  
  /* calling the SBML ODE Solver, and retrieving SBMLResults */  
  results = SBML_odeSolver(d, set);
  
  CvodeSettings_free(set);
  SBMLDocument_free(d);
  
  if ( results == NULL ) {
    printf("Integration not sucessful!\n");
    return (EXIT_FAILURE);  
  }

  /* print all species  */
  printf("Printing Species time courses\n");
  printf("time ");
  for ( j=0; j<results->species->num_val; j++) {
    printf("%s ", results->species->names[j]);
  }
  printf("\n");
  for ( i=0; i<results->timepoints; i++ ) {
    printf("%g ", results->time[i]);
    for ( j=0; j<results->species->num_val; j++) {
      printf("%g ", results->species->values[i][j]);
    }
    printf("\n");
  }

  /* now we can also free the result structure */
  SBMLResults_free(results);

  return (EXIT_SUCCESS);  
}

/* End of file */
