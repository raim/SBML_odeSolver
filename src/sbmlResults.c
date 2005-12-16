/*
  Last changed Time-stamp: <2005-12-16 18:30:31 raim>
  $Id: sbmlResults.c,v 1.16 2005/12/16 17:35:55 raimc Exp $
*/
/* 
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. The software and
 * documentation provided hereunder is on an "as is" basis, and the
 * authors have no obligations to provide maintenance, support,
 * updates, enhancements or modifications.  In no event shall the
 * authors be liable to any party for direct, indirect, special,
 * incidental or consequential damages, including lost profits, arising
 * out of the use of this software and its documentation, even if the
 * authors have been advised of the possibility of such damage.  See
 * the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * The original code contained here was initially developed by:
 *
 *     Rainer Machne
 *
 * Contributor(s):
 *     
 */
/*! \defgroup sbmlResults SBML Results Interface
    \ingroup odeSolver
    
    \brief This module contains interfaces to the results structures
    returned by the high level interfaces to SOSlib

*/
/*@{*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/sbmlResults.h"
#include "sbmlsolver/solverError.h"


static timeCourseArray_t *TimeCourseArray_create(int names, int timepoints);
static void TimeCourseArray_free(timeCourseArray_t *);
static timeCourse_t *TimeCourse_create(int timepoints);
static void TimeCourse_free(timeCourse_t *);
static timeCourse_t *TimeCourseArray_getTimeCourse(const char *, timeCourseArray_t *);
static void TimeCourseArray_dump(timeCourseArray_t *, timeCourse_t *);


/*** results as returned by _odeSolver ***/

/* The function
   SBMLResults SBMLResults_create(Model_t *m, int timepoints)
   allocates memory for simulation results (time courses) mapped back
   on SBML structures (i.e. species, and non-constant compartments
   and parameters. It takes CvodeData as an argument, that has to
   contain CVODE integraton results.
*/

SBMLResults_t *SBMLResults_create(Model_t *m, int timepoints)
{
  int i, num_reactions, num_species, num_compartments, num_parameters;
  Species_t *s;
  Compartment_t *c;
  Parameter_t *p;
  Reaction_t *r;
  SBMLResults_t *results;
  timeCourse_t *tc;

  ASSIGN_NEW_MEMORY(results, struct _SBMLResults, NULL);
  
  /* Allocating the time array  */
  results->time =  TimeCourse_create(timepoints);
  ASSIGN_NEW_MEMORY_BLOCK(results->time->name, 5, char, NULL);
  sprintf(results->time->name, "time");
  
  /* Allocating arrays for all SBML species */
  num_species = Model_getNumSpecies(m);
  results->species = TimeCourseArray_create(num_species, timepoints);

  
  /* Writing species names */
  for ( i=0; i<Model_getNumSpecies(m); i++) {
    s = Model_getSpecies(m, i);
    tc = results->species->tc[i];
    ASSIGN_NEW_MEMORY_BLOCK(tc->name, strlen(Species_getId(s))+1, char, NULL);
    sprintf(tc->name, "%s", Species_getId(s));
  }

  /* Allocating arrays for all variable SBML compartments */
  num_compartments = 0;
  for ( i=0; i<Model_getNumCompartments(m); i++ ) 
    if ( ! Compartment_getConstant(Model_getCompartment(m, i)) )
      num_compartments++;
  
  results->compartments = TimeCourseArray_create(num_compartments, timepoints);
  /* Writing variable compartment names */
  for ( i=0; i<Model_getNumCompartments(m); i++) {
    c = Model_getCompartment(m, i);
    if ( ! Compartment_getConstant(c) ) {
      tc = results->compartments->tc[i];
      ASSIGN_NEW_MEMORY_BLOCK(tc->name, strlen(Compartment_getId(c))+1,
			      char, NULL);
      sprintf(tc->name, Compartment_getId(c));
    }
  }

  /* Allocating arrays for all variable SBML parameters */
  num_parameters = 0;
  for ( i=0; i<Model_getNumParameters(m); i++ ) 
    if ( ! Parameter_getConstant(Model_getParameter(m, i)) )
      num_parameters++;
  
  results->parameters = TimeCourseArray_create(num_parameters, timepoints);
  /* Writing variable parameter names */
  for ( i=0; i<Model_getNumParameters(m); i++) {
    p = Model_getParameter(m, i);
    if ( ! Parameter_getConstant(p) ) {
      tc = results->parameters->tc[i];
      ASSIGN_NEW_MEMORY_BLOCK(tc->name, strlen(Parameter_getId(p))+1,
			      char, NULL);
      sprintf(tc->name, Parameter_getId(p));
    }
  }  

  /* Allocating arrays for all variable SBML reactions */
  num_reactions = Model_getNumReactions(m);
  results->fluxes = TimeCourseArray_create(num_reactions, timepoints);
  /* Writing reaction names */
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    tc = results->fluxes->tc[i];
    ASSIGN_NEW_MEMORY_BLOCK(tc->name, strlen(Reaction_getId(r))+1, char, NULL);
    sprintf(tc->name, Reaction_getId(r));
  }

  return results;
}


/** Returns the timeCourse for the integration time points
*/

SBML_ODESOLVER_API timeCourse_t *SBMLResults_getTime(SBMLResults_t *results)
{
  return results->time;
}


/** Returns the number of time points, including initial time
*/

SBML_ODESOLVER_API int SBMLResults_getNout(SBMLResults_t *res)
{
  return res->time->timepoints;
}


/** Returns the number of model constants for which
    sensitivities have been calculated
*/

SBML_ODESOLVER_API int SBMLResults_getNumSens(SBMLResults_t *res)
{
  return res->nsens;
}


/** Returns the name (SBML ID) of the ith constants for which
    sensitivities have been calculated, where
    0 <= i < SBMLResults_getNumSens
*/

SBML_ODESOLVER_API const char *SBMLResults_getSensParam(SBMLResults_t *res, int i)
{
  return res->param[i];
}


/** Returns the timeCourse for a  variable (non-constant) compartment.
    For a constant compartment NULL is returned.
*/

SBML_ODESOLVER_API timeCourse_t *Compartment_getTimeCourse(Compartment_t *c, SBMLResults_t *results)
{
  return TimeCourseArray_getTimeCourse(Compartment_getId(c),
				       results->compartments);
}


/** Returns the timeCourse for a species, whether constant or not.
*/

SBML_ODESOLVER_API timeCourse_t *Species_getTimeCourse(Species_t *s, SBMLResults_t *results)
{
  return TimeCourseArray_getTimeCourse(Species_getId(s), results->species);
}


/** Returns the timeCourse for a variable (non-constant) parameters.
    For a constant parameter NULL is returned.
*/

SBML_ODESOLVER_API timeCourse_t *Parameter_getTimeCourse(Parameter_t *p, SBMLResults_t *results)
{
  return TimeCourseArray_getTimeCourse(Parameter_getId(p),results->parameters);
}


/** Returns the timeCourse for a species, variable compartment or parameter
    or a reaction flux with the corresponding SBML ID.
*/

SBML_ODESOLVER_API timeCourse_t *SBMLResults_getTimeCourse(SBMLResults_t *results, const char *id)
{
  timeCourse_t *tc;
  tc = TimeCourseArray_getTimeCourse(id, results->species);
  if ( tc != NULL )
    return tc;
  tc = TimeCourseArray_getTimeCourse(id, results->compartments);
  if ( tc != NULL )
    return tc;
  tc = TimeCourseArray_getTimeCourse(id, results->parameters);
  if ( tc != NULL )
    return tc;

  tc = TimeCourseArray_getTimeCourse(id, results->fluxes);
  return tc;
}

static timeCourse_t *TimeCourseArray_getTimeCourse(const char *id, timeCourseArray_t *tcA)
{
  int i;
  timeCourse_t *tc;
  for ( i=0; i<tcA->num_val; i++ ) {
    tc = tcA->tc[i];
    if ( strcmp(id, tc->name) == 0 )
      return tc;
  }
  return NULL;
}


/** Returns the variable name (SBML ID) of a timeCourse
*/

SBML_ODESOLVER_API const char*TimeCourse_getName(timeCourse_t *tc)
{
  return (const char*) tc->name;
}


/** Returns the number of timepoints in a timeCourse
*/

SBML_ODESOLVER_API int TimeCourse_getNumValues(timeCourse_t *tc)
{
  return tc->timepoints;
}


/**  Returns ith value in a timeCourse, where
     0 <= i < TimeCourse_getNumValues
*/

SBML_ODESOLVER_API double TimeCourse_getValue(timeCourse_t *tc, int i)
{
  return tc->values[i];
}

/**  Returns the sensitivity to ith constant at jth time step, where
     0 <= i < SBMLResults_getNumSens, and
     0 <= j < TimeCourse_getNumValues
*/

SBML_ODESOLVER_API double TimeCourse_getSensitivity(timeCourse_t *tc, int i, int j)
{
  return tc->sensitivity[i][j];
}


/** Frees SBMLResults structure
*/

SBML_ODESOLVER_API void SBMLResults_free(SBMLResults_t *results)
{
  TimeCourse_free(results->time);
  TimeCourseArray_free(results->species);
  TimeCourseArray_free(results->compartments);
  TimeCourseArray_free(results->parameters);
  TimeCourseArray_free(results->fluxes);
  free(results);
}

static timeCourseArray_t *TimeCourseArray_create(int num_val, int timepoints)
{
  int i;
  timeCourseArray_t *tcA;

  ASSIGN_NEW_MEMORY(tcA, struct timeCourseArray, NULL);
  tcA->num_val = num_val;
   /* num_val time course structures */  
  ASSIGN_NEW_MEMORY_BLOCK(tcA->tc, num_val, struct timeCourse *, NULL);
  for ( i=0; i<num_val; i++ )
    tcA->tc[i] = TimeCourse_create(timepoints);
  
  return tcA;
}

static void TimeCourseArray_free(timeCourseArray_t *tcA)
{
  int i;
  for ( i=0; i<tcA->num_val; i++ )
    TimeCourse_free(tcA->tc[i]);
  free(tcA->tc);
  free(tcA);  
}

static timeCourse_t *TimeCourse_create(int timepoints)
{
  timeCourse_t *tc;
  ASSIGN_NEW_MEMORY(tc, struct timeCourse, NULL);
  tc->timepoints = timepoints;
  /* timecourses variable matrix */
  ASSIGN_NEW_MEMORY_BLOCK(tc->values, timepoints, double, NULL);  
  return tc;
}

static void TimeCourse_free(timeCourse_t *tc)
{
  free(tc->name);
  free(tc->values);
  free(tc);
}


/* */
static
void TimeCourseArray_dump(timeCourseArray_t *tcA, timeCourse_t *time)
{
  int i, j;
  timeCourse_t *tc;
  
  /* print all species  */
  /* print variable compartments */
  if ( tcA == NULL ) 
    printf("## No Values.\n");
  else if ( tcA->num_val == 0 ) 
    printf("## No Values.\n");
  else {    
    printf("#time ");
    for ( j=0; j<tcA->num_val; j++) {
      tc = tcA->tc[j];
      printf("%s ", tc->name);
    }
    printf("\n");
    for ( i=0; i<time->timepoints; i++ ) {
      printf("%g ", time->values[i]);
      for ( j=0; j<tcA->num_val; j++) {
	tc = tcA->tc[j];
	printf("%g ", tc->values[i]);
      }
      printf("\n");
    }
  }  
}


/** Prints the timeCourses of all SBML species
*/

SBML_ODESOLVER_API void SBMLResults_dumpSpecies(SBMLResults_t *results)
{
  printf("## Printing Species time courses\n");
  TimeCourseArray_dump(results->species, results->time);
}


/** Prints the timeCourses of all variable SBML compartments
*/

SBML_ODESOLVER_API void SBMLResults_dumpCompartments(SBMLResults_t *results)
{
  printf("## Printing Variable Compartment time courses\n");
  TimeCourseArray_dump(results->compartments, results->time);
}


/** Prints the timeCourses of all variable SBML parameters.
*/

SBML_ODESOLVER_API void SBMLResults_dumpParameters(SBMLResults_t *results)
{
  printf("## Printing Variable Parameter time courses\n");
  TimeCourseArray_dump(results->parameters, results->time);
}


/**  Prints the timeCourses of all SBML reaction fluxes
*/

SBML_ODESOLVER_API void SBMLResults_dumpFluxes(SBMLResults_t *results)
{
  printf("## Printing Reaction Flux time courses\n");
  TimeCourseArray_dump(results->fluxes, results->time);
}


/** Prints the timeCourses of all SBML species, of variable
    compartments and parameters, and of reaction fluxes  
*/

SBML_ODESOLVER_API void SBMLResults_dump(SBMLResults_t *results)
{
  printf("### Printing All Results \n");
  SBMLResults_dumpCompartments(results);
  SBMLResults_dumpSpecies(results);
  SBMLResults_dumpParameters(results);
  SBMLResults_dumpFluxes(results);
}



/*** results matrix as returned by _odeSolverBatch parameter variation ***/

/** Returns the SBMLResults for the jth value of the ith parameter from
    a parameter variation batch run SBMLResultMatrix
*/

SBML_ODESOLVER_API SBMLResults_t *SBMLResultsMatrix_getResults(SBMLResultsMatrix_t *resM, int i, int j)
{
  return resM->results[i][j];  
}


/** Frees the SBMLResultMatrix from a parameter variation batch run 
*/


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

SBMLResultsMatrix_t *
SBMLResultsMatrix_allocate(int nrparams, int nrdesignpoints)
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

/*\@}*/

/* End of file */
