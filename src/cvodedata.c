/*
  Last changed Time-stamp: <2005-10-12 12:05:24 raim>
  $Id: cvodedata.c,v 1.11 2005/10/12 12:52:08 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/Model.h>

/* own header files */
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/util.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/variableIndex.h"


/* private functions */
static void CvodeData_freeStructures(cvodeData_t * data);
static cvodeData_t *CvodeData_allocate(int nvalues, int nevents);


/* Internal Integration Data: The functions allocate and free
  cvodeData and cvodeResults required by the CVODE interface functions
  to read values and store results, respectively. */
static cvodeData_t *CvodeData_allocate(int nvalues, int nevents){

  cvodeData_t * data;

  ASSIGN_NEW_MEMORY(data, struct cvodeData, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->trigger, nevents, int, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->value, nvalues, double, NULL);
  
  return data ;
}

/* I.2: Initialize Data for Integration
   Use the odeModel structure to initialize and
   fill the data structure cvodeData_t *, that can then
   be passed to CVODE for integration */
cvodeData_t *CvodeData_create(odeModel_t *om)
{
  int neq, nconst, nass, nvalues, nevents;
  cvodeData_t *data;

  neq    = om->neq;
  nconst = om->nconst;
  nass   = om->nass;
  nevents = Model_getNumEvents(om->simple);
  nvalues = neq + nconst + nass;

  /* allocate memory for current integration data storage */
  data = CvodeData_allocate(nvalues, nevents);

  RETURN_ON_FATALS_WITH(NULL);

  data->nvalues = nvalues;

  /* set pointer to input model */
  data->model = om ;

  /* set integration run counter to 0,
     used for multiple reruns of integration  */
  data->run = 0;

  return data;
}

/*  Frees cvodeData, as created by IntegratorInstance */
void CvodeData_free(cvodeData_t * data) {
  
  if(data == NULL)
    return;
  
  CvodeData_freeStructures(data);
  free(data);

}


/* frees all internal stuff of cvodeData */
static void CvodeData_freeStructures(cvodeData_t * data) {

  if(data == NULL){
    return;
  }

  /* free results structure */
  CvodeResults_free(data->results, data->nvalues);

  /* free current values array */
  free(data->value);
  
  /* free event trigger flags */
  free(data->trigger);

}


/** Creates cvodeResults, the structure that stores
    CVODE integration results
*/

cvodeResults_t *CvodeResults_create(cvodeData_t * data) {

  int i;
  cvodeResults_t *results;

  ASSIGN_NEW_MEMORY(results, struct cvodeResults, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(results->time, data->nout+1, double, NULL)
  
  /* The 2-D array `value' contains the time courses, that are
     calculated by ODEs (SBML species, or compartments and parameters
     defined by rate rules.
  */
    ASSIGN_NEW_MEMORY_BLOCK(results->value, data->nvalues, double *, NULL);

  results->nvalues = data->nvalues;    

  for ( i=0; i<data->nvalues; ++i )
    ASSIGN_NEW_MEMORY_BLOCK(results->value[i], data->nout+1, double, NULL)
  
  return results;  
}


/** Returns the number of time points for which results exist
*/

SBML_ODESOLVER_API int CvodeResults_getNout(cvodeResults_t *results)     
{
  return results->nout + 1;
}

/** Returns the time point number n, where 0 <= n < CvodeResults_getNout
*/

SBML_ODESOLVER_API double CvodeResults_getTime(cvodeResults_t *results, int n)
{
  return results->time[n];
}


/** Returns the value of a variable or parameter of the odeModel
    at time step n via its variableIndex, where
    0 <= n < CvodeResults_getNout, and the variableIndex can be retrieved
    from the input odeModel
*/

SBML_ODESOLVER_API double CvodeResults_getValue(cvodeResults_t *results,
						variableIndex_t *vi, int n)
{
  return results->value[n][vi->index];
}




/** Frees results structure cvodeResults filled by the
    CVODE integrator
*/
void CvodeResults_free(cvodeResults_t *results, int nvalues) {

  int i;
  /* free CVODE results if filled */
  if(results != NULL){
    for(i=0;i<nvalues;i++){
      free(results->value[i]);
    }
    free(results->time);
    free(results->value);
    free(results);	      
  }
}


/* End of file */
