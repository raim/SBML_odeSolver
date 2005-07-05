/*
  Last changed Time-stamp: <2005-06-28 16:27:15 raim>
  $Id: cvodedata.c,v 1.7 2005/07/05 15:30:27 afinney Exp $
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
  "CvodeData CvodeData_create(int neq, int nconst, int nass, int nevents)"
  and
  "static void CvodeData_free(CvodeData data)"
  allocate and free the memory for CvodeData
  needed by the CVODE integrator functions, respectively. 
*/


CvodeData
CvodeData_create(int neq, int nconst, int nass,
		 int nevents, const char *resultsFilename){

  CvodeData data;

  ASSIGN_NEW_MEMORY(data, struct _CvodeData, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->trigger, nevents, int, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->value, neq, double, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->pvalue, nconst, double, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->avalue, nass, double, NULL);

  data->filename = resultsFilename ;

  if (resultsFilename)
      data->outfile = fopen(data->filename, "w");
  else
      data->outfile = stdout ;

  return data ;
}

static void CvodeData_freeStructuresExcludingModel(CvodeData data)
{
  int i;

  if(data == NULL){
    return;
  }

  /* free CVODE results if filled */
  if(data->results != NULL){
    for(i=0;i<data->model->nass;i++){
      free(data->results->avalue[i]);
    }
    free(data->results->avalue);
    for(i=0;i<data->model->nconst;i++){
      free(data->results->pvalue[i]);
    }
    free(data->results->pvalue);
    for(i=0;i<data->model->neq;i++){
      free(data->results->value[i]);
    }
    free(data->results->time);
    free(data->results->value);
    free(data->results);	      
  }

  free(data->value);
  free(data->avalue);   
  free(data->pvalue);

  /* free event trigger flags */
  free(data->trigger);
  
  /* save and close results file */
  if (data->filename) {
    fclose(data->outfile);
    fprintf(stderr, "Saved results to file %s.\n\n", data->filename);
    free((char *) data->filename);
  }
}

void
CvodeData_free(CvodeData data){
    CvodeData_freeStructuresExcludingModel(data);
    ODEModel_free(data->model);
    free(data);
}

void
CvodeData_freeExcludingModel(CvodeData data)
{
    CvodeData_freeStructuresExcludingModel(data);
    free(data);
}

/* The function
   "CvodeResults CvodeResults_create(CvodeData) creates
   the structure that contains the integration results produced
   by CVODE.
*/

CvodeResults
CvodeResults_create(CvodeData data){

  int i;
  CvodeResults results;

  if (!data->storeResults)
    return 0 ;

  ASSIGN_NEW_MEMORY(results, struct _CvodeResults, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(results->time, data->nout+1, double, NULL)
  
  /* The 2-D array `value' contains the time courses, that are
     calculated by ODEs (SBML species, or compartments and parameters
     defined by rate rules.
  */
  ASSIGN_NEW_MEMORY_BLOCK(results->value, data->model->neq, double *, NULL)

  for(i=0;i<data->model->neq;++i)
    ASSIGN_NEW_MEMORY_BLOCK(results->value[i], data->nout+1, double, NULL)
  
  /* The 2-D array `pvalue' contains time courses for all
     constant SBML species. These data are not needed for
     integration, but is convenient to have for output of
     results.
  */
  ASSIGN_NEW_MEMORY_BLOCK(results->pvalue, data->model->nconst, double *, NULL)

  for(i=0;i<data->model->nconst;++i)
    ASSIGN_NEW_MEMORY_BLOCK(results->pvalue[i], data->nout+1, double, NULL)
  
  /* The 2-D array `avalue' contains time courses for all
     SBML species, compartments and parameters that are defined
     by SBML assignment rules. Again, this is not needed for
     integration, but convenient to have for output of results.
   */
  ASSIGN_NEW_MEMORY_BLOCK(results->avalue, data->model->nass, double *, NULL)

  for(i=0;i<data->model->nass;++i)
    ASSIGN_NEW_MEMORY_BLOCK(results->avalue[i], data->nout+1, double, NULL)
  
  return results;  
}

CvodeData 
constructODEsPassingOptions(Model_t *m, const char *resultsFilename,
			    int simplify, int determinant,
			    const char *parameterNotToBeReplaced)
{     
    CvodeData result;
    odeModel_t *om =
      ODEModel_createFromModelAndOptions(m, simplify,
					 determinant,
					 parameterNotToBeReplaced);

    SolverError_dump(); /* write out all everything including warnings */
    RETURN_ON_ERRORS_WITH(NULL);
    SolverError_clear(); /* get rid of any lingering warnings */

    result = CvodeData_createFromODEModel(om, resultsFilename);
    SolverError_dump();
    RETURN_ON_FATALS_WITH(NULL);

    return result ;
}

/**
  This function initializes and fills the CvodeData for CVODE:
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
    fill the data structure CvodeData, that can then
    be passed to CVODE for integration      
*/

CvodeData
CvodeData_createFromODEModel(odeModel_t *m,
			     const char *resultsFilename)
{
  int i, j, neq, nconst, nass, nevents;
  Model_t *ode = m->simple;
  Parameter_t *p;
  Species_t *s;
  Rule_t *rl;
  RateRule_t *rr;
  SBMLTypeCode_t type;  
  CvodeData data;

  neq    = m->neq;
  nconst = m->nconst;
  nass   = m->nass;
  nevents = Model_getNumEvents(m->m);

  data = CvodeData_create(neq, nconst, nass, nevents, resultsFilename);

  RETURN_ON_FATALS_WITH(NULL);

  data->model = m ;
  data->UseJacobian = m->simplified ;

  /*
    filling CvodeData structure with data from
    the ODE model
  */
  nconst = 0;  
  for ( i=0; i<Model_getNumParameters(ode); i++ ) {
    p = Model_getParameter(ode, i);
    if ( Parameter_getConstant(p) ) {     
      data->pvalue[nconst] = Parameter_getValue(p);
      nconst++;
    }
  }

  neq  = 0;
  nass = 0;
  
  for ( j=0; j<Model_getNumRules(ode); j++ ) {
    rl = Model_getRule(ode,j);
    type = SBase_getTypeCode((SBase_t *)rl);
  
    if ( type == SBML_RATE_RULE ) {

      rr = (RateRule_t *)rl;
      s = Model_getSpeciesById(ode, RateRule_getVariable(rr));
      data->value[neq] = Species_getInitialConcentration(s);

      neq++;      
    }
    else if ( type == SBML_ASSIGNMENT_RULE )
    {  
      data->avalue[nass] = evaluateAST(data->model->assignment[nass], data);
      nass++;      
    }
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
