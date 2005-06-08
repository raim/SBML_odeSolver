/*
  Last changed Time-stamp: <2005-06-08 11:29:08 raim>
  $Id: batch_integrate.c,v 1.3 2005/06/08 09:35:44 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sbmlsolver/odeSolver.h>

static void
printResults(SBMLResults results);

int
main (int argc, char *argv[]){
  int i;
  char model[256];
  char parameter[256];
  char reaction[256];
  double start, end, steps, value;
  double time = 0.0;
  double printstep = 1.0;
  SBMLDocument_t *d;
  SBMLReader_t *sr;
  SBMLResults *results;
  CvodeSettings set;
  VarySettings vary;
  
  /** initializing options, that are set via commandline
      arguments in the stand-alone version of the odeSolver.
      Please see file options.h for possible options.
      Option handling will be reorganized soon, so that this
      step will not be needed anymore!!
  */
  initializeOptions();
  /* Opt.PrintMessage = 1; */
  
  sscanf(argv[1], "%s", model);
  sscanf(argv[2], "%lf", &time);
  sscanf(argv[3], "%lf", &printstep);
  sscanf(argv[4], "%lf", &start);
  sscanf(argv[5], "%lf", &end);
  sscanf(argv[6], "%lf", &steps);
  strcpy(parameter, argv[7]);
  if ( argc > 8 ) {
    strcpy(reaction, argv[8]);
  }
  else{
    strcpy(reaction,"");
  }
  
  printf("Varying parameter %s (reaction %s) from %f to %f in %f steps\n",
	 parameter, reaction, start, end, steps);
  
  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);  

  /* Setting SBML ODE Solver integration parameters */
  set.Time = time;
  set.PrintStep = printstep;
  
  set.Error = 1e-18;
  set.RError = 1e-14;
  set.Mxstep = 10000;

  set.PrintOnTheFly = 0;  
  set.PrintMessage = 0;
  set.HaltOnEvent = 0;
  set.SteadyState = 0;
  set.UseJacobian = 1;

  /* Setting SBML Ode Solver batch integration parameters */
  vary.start = start;
  vary.end = end;
  vary.steps = steps;
  vary.id = parameter;
  vary.rid = reaction;
  
  /* calling the SBML ODE Solver, and retrieving SBMLResults */
  results = Model_odeSolverBatch(d, set, vary);  
  SBMLDocument_free(d);

  if ( results == NULL ) {
    printf("Parameter variation not succesfull!\n");
    return(0);
  }
  

  value = start;
  for ( i=0; i<(steps+1); i++ ) {
    printf("RESULTS FOR RUN # %d, with (reaction: %s) %s = %f:\n",
	   i+1, reaction, parameter, value);
    printResults(results[i]);
    SBMLResults_free(results[i]);
    value = value + ((end-start)/steps);
  }
  free (results);
	  

  return (EXIT_SUCCESS);  
}

static void
printResults(SBMLResults results) {

  int i, j;
  
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

  printf("\n");
}

/* End of file */
