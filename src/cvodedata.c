/*
  Last changed Time-stamp: <2005-08-02 00:17:01 raim>
  $Id: cvodedata.c,v 1.10 2005/08/02 13:20:28 raimc Exp $
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

/*
  The functions
  "cvodeData_t * CvodeData_create(int neq, int nconst, int nass, int nevents)"
  and
  "static void CvodeData_free(cvodeData_t * data)"
  allocate and free the memory for cvodeData_t *
  needed by the CVODE integrator functions, respectively. 
*/


cvodeData_t *
CvodeData_create(int nvalues, int nevents){

  cvodeData_t * data;

  ASSIGN_NEW_MEMORY(data, struct _CvodeData, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->trigger, nevents, int, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->value, nvalues, double, NULL);
  
  return data ;
}

static void CvodeData_freeStructuresExcludingModel(cvodeData_t * data)
{
  int i;

  if(data == NULL){
    return;
  }

  /* free CVODE results if filled */
  if(data->results != NULL){
    for(i=0;i<data->nvalues;i++){
      free(data->results->value[i]);
    }
    free(data->results->time);
    free(data->results->value);
    free(data->results);	      
  }

  free(data->value);

  /* free event trigger flags */
  free(data->trigger);

  /* free cvodeSettings_t */
  /* if ( data->opt != NULL ) */
   /*  free(data->opt); */


}

void
CvodeData_free(cvodeData_t * data){
    CvodeData_freeStructuresExcludingModel(data);
    ODEModel_free(data->model);
    free(data);
}

void
CvodeData_freeExcludingModel(cvodeData_t * data)
{
    CvodeData_freeStructuresExcludingModel(data);
    free(data);
}

/* The function
   "cvodeResults_t *CvodeResults_create(cvodeData_t *) creates
   the structure that contains the integration results produced
   by CVODE.
*/

cvodeResults_t *
CvodeResults_create(cvodeData_t * data){

  int i;
  cvodeResults_t *results;

  if (!data->opt->StoreResults)
    return 0 ;

  ASSIGN_NEW_MEMORY(results, struct _CvodeResults, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(results->time, data->nout+1, double, NULL)
  
  /* The 2-D array `value' contains the time courses, that are
     calculated by ODEs (SBML species, or compartments and parameters
     defined by rate rules.
  */
  ASSIGN_NEW_MEMORY_BLOCK(results->value, data->nvalues, double *, NULL)

  for ( i=0; i<data->nvalues; ++i )
    ASSIGN_NEW_MEMORY_BLOCK(results->value[i], data->nout+1, double, NULL)
  
  return results;  
}

cvodeData_t * 
constructODEs(Model_t *m, int simplify)
{     
    cvodeData_t * result;
    odeModel_t *om =  ODEModel_createFromModelAndOptions(m, simplify);

    SolverError_dump(); /* write out all everything including warnings */
    RETURN_ON_ERRORS_WITH(NULL);
    SolverError_clear(); /* get rid of any lingering warnings */

    result = CvodeData_createFromODEModel(om);
    SolverError_dump();
    RETURN_ON_FATALS_WITH(NULL);

    return result ;
}

/**
  This function initializes and fills the cvodeData_t * for CVODE:
  neq: the number of time-dependent variables and ODEs
  ode[neq]: the ODEs
  species[neq]: the names of the  time-dependent variables,
                as they appear in the ODEs (kinetic laws, rate rules).
  value[neq]:   the initial values of the time-dependent variables

  When finished with ODE construction successfully, it (optionally)
  attempts to construct the Jacobian matrix by calling the function
  ODEs_constructJacobian.
*/

/** I.1: Initialize Data for Integration
    Use the new simplified model to initialize and
    fill the data structure cvodeData_t *, that can then
    be passed to CVODE for integration      
*/

cvodeData_t *
CvodeData_createFromODEModel(odeModel_t *m)
{
  int i, neq, nconst, nass, nvalues, nevents;
  Model_t *ode = m->simple;
  Parameter_t *p;
  Species_t *s;
  Compartment_t *c;
  cvodeData_t *data;

  neq    = m->neq;
  nconst = m->nconst;
  nass   = m->nass;
  nevents = Model_getNumEvents(m->m);
  nvalues = neq + nconst + nass;
  
  data = CvodeData_create(nvalues, nevents);

  RETURN_ON_FATALS_WITH(NULL);

  data->nvalues = nvalues;
  data->model = m ;

  /*
    filling cvodeData_t  structure with data from
    the ODE model
  */

  neq  = 0;
  nass = 0;

  for ( i=0; i<nvalues; i++ ) {
    if ( (s = Model_getSpeciesById(ode, m->names[i])) )
      data->value[i] = Species_getInitialConcentration(s);
    else if ( (c = Model_getCompartmentById(ode, m->names[i])) )
      data->value[i] = Compartment_getSize(c);
    else if ((p = Model_getParameterById(ode, m->names[i])) )
      data->value[i] = Parameter_getValue(p);
  }

  /* data->run is set to 0, will be used for trying a rerun
     of integration withour or with generated Jacobian,
     or with lower error tolerance,
     when first integration with or without generated Jacobian failed
     with CVODE flag -6/CONV_FAILURE
   */
  data->run = 0;

  return data;
}

/* End of file */
