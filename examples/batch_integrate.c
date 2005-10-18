/*
  Last changed Time-stamp: <2005-10-18 16:15:10 raim>
  $Id: batch_integrate.c,v 1.9 2005/10/18 14:17:32 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/sbmlsolver/odeSolver.h"
#include "../src/sbmlsolver/options.h"

static void
printResults(SBMLResults_t *results);

int
main (int argc, char *argv[]){
  int i, j;
  char model[256];
  char parameter[256];
  char *reaction;
  double start, end, steps, value;
  double time = 0.0;
  double printstep = 1.0;
  SBMLDocument_t *d;
  SBMLReader_t *sr;

  /* required SOSlib structures, that will be created
     during and must be freed at the end of this program  */
  cvodeSettings_t *set;
  varySettings_t *vs;
  SBMLResultsMatrix_t *resM;
  SBMLResults_t *results;
  
  sscanf(argv[1], "%s", model);
  sscanf(argv[2], "%lf", &time);
  sscanf(argv[3], "%lf", &printstep);
  sscanf(argv[4], "%lf", &start);
  sscanf(argv[5], "%lf", &end);
  sscanf(argv[6], "%lf", &steps);
  strcpy(parameter, argv[7]);
  
  if ( argc > 8 ) {
    ASSIGN_NEW_MEMORY_BLOCK(reaction, strlen(argv[8])+1, char, 0);
    sprintf(reaction, argv[8]);
  }
  else{
    reaction = NULL;
  }
  
  printf("### Varying parameter %s (reaction %s) from %f to %f in %f steps\n",
	 parameter, reaction, start, end, steps);
  
  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);  


  /* Setting SBML ODE Solver integration parameters */
   /* Setting SBML ODE Solver integration parameters with default values */
  set = CvodeSettings_createDefaults();
  /* resetting the values we need */
  CvodeSettings_setTime(set, time, printstep);
  /* CvodeSettings_setErrors(set, 1e-18, 1e-10, 10000); */
  CvodeSettings_setSwitches(set, 1, 0, 1, 1, 1); 
  
  /* Setting SBML Ode Solver batch integration parameters */
  vs = VarySettings_allocate(1, steps+1);
  VarySettings_addParameter(vs, parameter, reaction, start, end);
  VarySettings_dump(vs);

  if ( reaction != NULL )
    free(reaction);

  /* calling the SBML ODE Solver Batch function,
     and retrieving SBMLResults */
  resM = SBML_odeSolverBatch(d, set, vs);

  if ( resM == NULL ) {
    printf("### Parameter variation not succesful!\n");
    return(0);
  }
    /* we don't need these anymore */
  CvodeSettings_free(set);  
  /* printf("%s", writeSBMLToString(d)); */
  SBMLDocument_free(d);
  VarySettings_free(vs);

  results = resM->results[0][0];

  for ( i=0; i<resM->i; i++ ) {
    for ( j=0; j<resM->j; j++ ) {
      results = SBMLResultsMatrix_getResults(resM, i, j);
      printf("### RESULTS Parameter %d, Step %d \n", i+1, j+1);
      printResults(results);
    }
  }

  SBMLResultsMatrix_free(resM);
  return (EXIT_SUCCESS);  
}

static void
printResults(SBMLResults_t *results) {

  int i, j;
  
  /* print all species  */
  printf("### Printing Species time courses\n");
  printf("#time ");
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
    printf("### No variable compartments.\n");

  }
  else {
    printf("### Printing variable Compartment time courses\n");
    printf("#time ");
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
    printf("### No variable parameters.\n");
  }
  else {
    printf("### Printing variable Parameter time courses\n");
    printf("#time ");
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
  printf("\n");
}

/* End of file */
