/*
  Last changed Time-stamp: <2005-10-17 16:45:22 raim>
  $Id: sbmlResults.c,v 1.6 2005/10/17 16:07:50 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/sbmlResults.h"
#include "sbmlsolver/solverError.h"


static timeCourse_t *TimeCourseArray_create(int names, int timepoints);
static void TimeCourseArray_free(timeCourse_t *tc);
static timeCourse_t *TimeCourse_create(int names, int timepoints);
static void TimeCourse_free(timeCourse_t *tc);

/* The function
   SBMLResults SBMLResults_create(Model_t *m, int timepoints)
   allocates memory for simulation results (time courses) mapped back
   on SBML structures (i.e. species, and non-constant compartments
   and parameters. It takes CvodeData as an argument, that has to
   contain CVODE integraton results.
*/

SBMLResults_t *
SBMLResults_create(Model_t *m, int timepoints){

  int i, num_reactions, num_species, num_compartments, num_parameters;
  Species_t *s;
  Compartment_t *c;
  Parameter_t *p;
  Reaction_t *r;
  SBMLResults_t *results;

  ASSIGN_NEW_MEMORY(results, struct _SBMLResults, NULL);
  results->timepoints = timepoints;
  
  /* Allocating the time array:
     number of output times, plus 1 for the initial values. */
  ASSIGN_NEW_MEMORY_BLOCK(results->time, timepoints, double, NULL);
  
  /* Allocating arrays for all SBML species */
  num_species = Model_getNumSpecies(m);
  results->species = TimeCourse_create(num_species, timepoints);

  
  /* Writing species names */
  for ( i=0; i<Model_getNumSpecies(m); i++) {
    s = Model_getSpecies(m, i);
    ASSIGN_NEW_MEMORY_BLOCK(results->species->names[i],
			    strlen(Species_getId(s))+1, char, NULL);
    sprintf(results->species->names[i], "%s", Species_getId(s));
  }

  /* Allocating arrays for all variable SBML compartments */
  num_compartments = 0;
  for ( i=0; i<Model_getNumCompartments(m); i++ ) 
    if ( ! Compartment_getConstant(Model_getCompartment(m, i)) )
      num_compartments++;
  
  results->compartments = TimeCourse_create(num_compartments, timepoints);
  /* Writing variable compartment names */
  for ( i=0; i<Model_getNumCompartments(m); i++) {
    c = Model_getCompartment(m, i);
    if ( ! Compartment_getConstant(c) ) {
      ASSIGN_NEW_MEMORY_BLOCK(results->compartments->names[i],
			      strlen(Compartment_getId(c))+1, char, NULL);
      sprintf(results->compartments->names[i], Compartment_getId(c));
    }
  }

  /* Allocating arrays for all variable SBML parameters */
  num_parameters = 0;
  for ( i=0; i<Model_getNumParameters(m); i++ ) 
    if ( ! Parameter_getConstant(Model_getParameter(m, i)) )
      num_parameters++;
  results->parameters = TimeCourse_create(num_parameters, timepoints);
  /* Writing variable parameter names */
  for ( i=0; i<Model_getNumParameters(m); i++) {
    p = Model_getParameter(m, i);
    if ( ! Parameter_getConstant(p) ) {
      ASSIGN_NEW_MEMORY_BLOCK(results->parameters->names[i],
			      strlen(Parameter_getId(p))+1, char, NULL);
      sprintf(results->parameters->names[i], Parameter_getId(p));
    }
  }  

  /* Allocating arrays for all variable SBML reactions */
  num_reactions = Model_getNumReactions(m);
  results->fluxes = TimeCourse_create(num_reactions, timepoints);
  /* Writing reaction names */
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    ASSIGN_NEW_MEMORY_BLOCK(results->fluxes->names[i],
			      strlen(Reaction_getId(r))+1, char, NULL);
    sprintf(results->fluxes->names[i], Reaction_getId(r));
  }

  return results;
}


SBML_ODESOLVER_API SBMLResultsMatrix_t *SBMLResultsMatrix_allocate(int nrparams, int nrdesignpoints)
{
  int i;
  SBMLResultsMatrix_t *resM;
  ASSIGN_NEW_MEMORY(resM, struct _SBMLResultsMatrix, NULL);
  ASSIGN_NEW_MEMORY(resM->results, struct _SBMLResults **, NULL);
  resM -> i = nrparams;
  resM -> j = nrdesignpoints;
  for ( i=0; i<nrparams; i++ ) {
    ASSIGN_NEW_MEMORY_BLOCK(resM->results[i], nrdesignpoints,
			    struct _SBMLResults *, NULL);
  }
  return(resM);
}

SBML_ODESOLVER_API SBMLResults_t *SBMLResultsMatrix_getResults(SBMLResultsMatrix_t *resM, int i, int j)
{
  return resM->results[i][j];  
}


SBML_ODESOLVER_API void SBMLResultsMatrix_free(SBMLResultsMatrix_t *resM)
{
  int i, j;  
  for ( i=0; i<resM->i; i++ ) {
    for ( j=0; j<resM->j; j++ )
      SBMLResults_free(resM->results[i][j]);
    free(resM->results[i]);
  }
  free(resM->results);    
  free(resM);    
}


SBML_ODESOLVER_API void SBMLResults_free(SBMLResults_t *results) {

  if ( results != NULL ) {
    if ( results->time != NULL ) {
      free(results->time);
    }
    /* see SBMLResults_create and header file for comments */
    TimeCourse_free(results->species);
    TimeCourse_free(results->compartments);
    TimeCourse_free(results->parameters);
    TimeCourse_free(results->fluxes);

    free(results);
  }
}

static timeCourse_t *
TimeCourse_create(int num_val, int timepoints){

  int i;
  timeCourse_t *tc;

  ASSIGN_NEW_MEMORY(tc, struct timeCourse, NULL);
  tc->num_val = num_val;
  tc->timepoints = timepoints;
  /* timecourse variable names */  
  ASSIGN_NEW_MEMORY_BLOCK(tc->names, num_val, char *, NULL);
  /* timecourses variable matrix */
  ASSIGN_NEW_MEMORY_BLOCK(tc->values, timepoints, double *, NULL);
  /* timecourse arrays */
  for ( i=0; i<tc->timepoints; i++ ) 
    ASSIGN_NEW_MEMORY_BLOCK(tc->values[i], num_val, double, NULL);
    
  return tc;
}

static void
TimeCourse_free(timeCourse_t *tc) {

  int i;

  for ( i=0; i<tc->num_val; i++ )
      free(tc->names[i]);
  free(tc->names);
  
  for ( i=0; i<tc->timepoints; i++ )
    free(tc->values[i]);
  free(tc->values);

  free(tc);
  
}



/* End of file */
