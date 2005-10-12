/*
  Last changed Time-stamp: <2005-10-10 15:34:05 raim>
  $Id: integrate.c,v 1.5 2005/10/12 12:52:45 raimc Exp $
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
  SBMLResults_t *results;
  cvodeSettings_t *set;

   
  sscanf(argv[1], "%s", model);
  sscanf(argv[2], "%lf", &time);
  sscanf(argv[3], "%lf", &printstep);

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);

  

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_createDefaults();
  CvodeSettings_setTime(set, time, printstep);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1000);
  
  /* calling the SBML ODE Solver, and retrieving SBMLResults */  
  results = Model_odeSolver(d, set);
  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }
  
  CvodeSettings_free(set);
  SBMLDocument_free(d);

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
  /* print variable compartments */
  if ( results->compartments->num_val == 0 ) {
    printf("No variable compartments.\n");

  }
  else {
    printf("Printing variable Compartment time courses\n");
    printf("time ");
    for ( j=0; j<results->compartments->num_val; j++) {
      printf("%s ", results->compartments->names[j]);
    }
    printf("\n");
    for ( i=0; i<results->timepoints; i++ ) {
      printf("%g ", results->time[i]);
      for ( j=0; j<results->compartments->num_val; j++) {
	printf("%g ", results->compartments->values[i][j]);
      }
      printf("\n");
    }
  }
  /* print variable parameters */
  if ( results->parameters->num_val == 0 ) {
    printf("No variable parameters.\n");
  }
  else {
    printf("Printing variable Parameter time courses\n");
    printf("time ");
    for ( j=0; j<results->parameters->num_val; j++) {
      printf("%s ", results->parameters->names[j]);
    }
    printf("\n");
    for ( i=0; i<results->timepoints; i++ ) {
      printf("%g ", results->time[i]);
      for ( j=0; j<results->parameters->num_val; j++) {
	printf("%g ", results->parameters->values[i][j]);
      }
      printf("\n");
    }
  }
  /* print fluxes */
  printf("Printing Reaction Fluxes\n");
  printf("time ");
  for ( j=0; j<results->fluxes->num_val; j++) {
    printf("%s ", results->fluxes->names[j]);
  }
  printf("\n");
  for ( i=0; i<results->timepoints; i++ ) {
    printf("%g ", results->time[i]);
    for ( j=0; j<results->fluxes->num_val; j++) {
      printf("%g ", results->fluxes->values[i][j]);
    }
    printf("\n");
  }
  /* now we can also free the result structure */
  SBMLResults_free(results);

  return (EXIT_SUCCESS);  
}

/* End of file */
