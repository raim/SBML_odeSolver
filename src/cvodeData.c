/*
  Last changed Time-stamp: <2007-10-26 18:31:07 raim>
  $Id: cvodeData.c,v 1.27 2007/10/26 17:52:29 raimc Exp $
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
 *     Andrew M. Finney
 */

/*! \defgroup cvodeData Integration Results Interface:  x(t)
  \ingroup integration
    
  \brief This module contains the functions to create input data
  for formula evaluation and retrieve results from integration   

*/
/*@{*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/Model.h>

/* own header files */


#include "sbmlsolver/cvodeData.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/integratorSettings.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/solverError.h"

#include "sbmlsolver/variableIndex.h"



/* private functions */
static void CvodeData_freeStructures(cvodeData_t *);
static cvodeData_t *CvodeData_allocate(int nvalues, int nevents, int neq);
static int CvodeData_allocateSens(cvodeData_t *, int neq, int nsens);
static void CvodeData_freeSensitivities(cvodeData_t *);
static void CvodeResults_freeSensitivities(cvodeResults_t *);
/* static int CvodeData_allocateAdjSens(cvodeData_t *data, int neq); */



/* Internal Integration Data: The functions allocate and free
   cvodeData and cvodeResults required by the CVODE interface functions
   to read values and store results, respectively. */
static cvodeData_t *CvodeData_allocate(int nvalues, int nevents, int neq)
{
  cvodeData_t * data;

  ASSIGN_NEW_MEMORY(data, struct cvodeData, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->trigger, nevents, int, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(data->value, nvalues, double, NULL);

  data->neq = neq;
  data->opt = NULL;
  
  data->sensitivity = NULL;
  data->p = NULL;
  data->p_orig  = NULL;

  /* Adjoint specific */
  /*!!! should this be moved to adjoint specific initiation? */
  ASSIGN_NEW_MEMORY_BLOCK(data->adjvalue, nvalues, double, NULL);


  return data ;
}

static int CvodeData_allocateSens(cvodeData_t *data, int neq, int nsens)
{
  int i;
  ASSIGN_NEW_MEMORY_BLOCK(data->p, nsens, realtype, 0);
  ASSIGN_NEW_MEMORY_BLOCK(data->p_orig, nsens, realtype, 0);
  ASSIGN_NEW_MEMORY_BLOCK(data->sensitivity, neq, double *, 0);
  for ( i=0; i<neq; i++ )
    ASSIGN_NEW_MEMORY_BLOCK(data->sensitivity[i], nsens, double, 0);

  data->nsens = nsens;
  data->neq = neq;

  return 1;
}

/* Step I.2: */
/** Creates cvodeData from an odeModel and initial values from the
    original SBML model.

    This function is internally used by integratorInstance creation.
    
    It is available as an API function, so users can create this
    structure from an odeModel to evaluate formulae in odeModel
    independent of integratorInstance.
*/
SBML_ODESOLVER_API cvodeData_t *CvodeData_create(odeModel_t *om)
{
  int neq, nconst, nass, nvalues, nevents;
  cvodeData_t *data;

  neq    = om->neq;
  nconst = om->nconst;
  nass   = om->nass;
  if ( om->simple != NULL ) nevents = Model_getNumEvents(om->simple);
  else nevents = 0;
  nvalues = neq + nconst + nass;

  /* allocate memory for current integration data storage */
  data = CvodeData_allocate(nvalues, nevents, neq);
  RETURN_ON_FATALS_WITH(NULL);

  data->nvalues = nvalues;
  data->nevents = nevents;

  /* set pointer to input model */
  data->model = om ;

  /* initialize values, this function call is only required for
     separate creation of data for analytic purposes */
  CvodeData_initializeValues(data);
  
  return data;
}


/** Writes values from the input SBML model into
    the data structure */

SBML_ODESOLVER_API void CvodeData_initializeValues(cvodeData_t *data)
{
  int i;
  Parameter_t *p;
  Species_t *s;
  Compartment_t *c;
  odeModel_t *om = data->model;
  Model_t *ode = om->simple;

  /* First, fill cvodeData_t  structure with data from
     the derived SBML model  */

  for ( i=0; i<data->nvalues; i++ )
  {
    /* 1: distinguish between SBML-based or direct odeModel */
    /* 1a SBML based initialization */
    /*!!! it might be better to write original SBML values also into this values array!!!*/
    if ( om->values == NULL )
    {
      if ( (s = Model_getSpeciesById(ode, om->names[i])) )
      {
	if ( Species_isSetInitialConcentration(s) )
	  data->value[i] = Species_getInitialConcentration(s);
	else if ( Species_isSetInitialAmount(s) )
	  data->value[i] = Species_getInitialAmount(s);
	else if ( i < om->neq || i >= (om->neq+om->nass) ) 
	  SolverError_error(WARNING_ERROR_TYPE,
			    SOLVER_ERROR_REQUESTED_PARAMETER_NOT_FOUND,
			    "No value found for species %s, value " \
			    "remains uninitialized!",
			    om->names[i]);
      }
      else if ( (c = Model_getCompartmentById(ode, om->names[i])) )
      {
	if ( Compartment_isSetSize(c) )
	  data->value[i] = Compartment_getSize(c);
	else if ( i < om->neq || i >= (om->neq+om->nass) ) 
	  SolverError_error(WARNING_ERROR_TYPE,
			    SOLVER_ERROR_REQUESTED_PARAMETER_NOT_FOUND,
			    "No value found for compartment %s, value " \
			    "remains uninitialized!",
			    om->names[i]);      
      }
      else if ( (p = Model_getParameterById(ode, om->names[i])) )
      {
	if ( Parameter_isSetValue(p) )
	  data->value[i] = Parameter_getValue(p);
	else if ( i < om->neq || i >= (om->neq+om->nass) ) 
	  SolverError_error(WARNING_ERROR_TYPE,
			    SOLVER_ERROR_REQUESTED_PARAMETER_NOT_FOUND,
			    "No value found for parameter %s, value " \
			    "remains uninitialized!",
			    om->names[i]);      	
      }
    }
    else if ( om->values != NULL )
    {
      /* 1b direct odeModel creation from ODEs */
      for ( i=0; i<om->neq+om->nass+om->nconst; i++ )
	data->value[i] = om->values[i];
    }
  }
 
  /* initialize assigned parameters */
  for ( i=0; i<om->nass; i++ ) 
    data->value[om->neq+i] = evaluateAST(om->assignment[i],data);

  /* set current time to 0 */
  data->currenttime = 0.0;

  /* Zeroing initial adjoint values */
  if ( data->adjvalue != NULL )
    for ( i=0; i<data->neq; i++ )
      data->adjvalue[i] = 0.0;

}

/** Frees cvodeData
 */

SBML_ODESOLVER_API void CvodeData_free(cvodeData_t * data)
{
  if(data == NULL)
    return;
  CvodeData_freeStructures(data);
  free(data);
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
    at time step timestep via its variableIndex, where
    0 <= timestep < CvodeResults_getNout,
    and the variableIndex can be retrieved from the input odeModel
*/

SBML_ODESOLVER_API double CvodeResults_getValue(cvodeResults_t *results, variableIndex_t *vi, int timestep)
{
  return results->value[vi->index][timestep];
}


/** Returns the ith (0 <= i < nsens) sensitivity of ODE variable y 
    at timestep nr. `timestep, where 0 <= timestep < CvodeResults_getNout.

    Returns 0 if i >= nsens (which also happens if no sensitivity was
    calculated).
*/

SBML_ODESOLVER_API double CvodeResults_getSensitivityByNum(cvodeResults_t *results,  int value, int i, int timestep)
{
  if ( i >= results->nsens ) return 0;
  else return results->sensitivity[value][i][timestep];
}


/** Returns the sensitivity of ODE variable y to parameter or variable s
    at timestep nr. `timestep, where 0 <= timestep < CvodeResults_getNout.

    Returns 0 if no sensitivity has been calculated for s!
*/

SBML_ODESOLVER_API double CvodeResults_getSensitivity(cvodeResults_t *results,  variableIndex_t *y,  variableIndex_t *s, int timestep)
{
  int i;
  /* find sensitivity for s */
  for ( i=0; i<results->nsens && !(results->index_sens[i] == s->index); i++ );
  if ( i == results->nsens ) return 0;
  else return results->sensitivity[y->index][i][timestep];
}


/** Computes the directional sensitivity of ODE variable y to parameter
    direction dp, for all time steps.
    
    Must not be called, if sensitivity wasn't calculated!
*/

SBML_ODESOLVER_API void CvodeResults_computeDirectional(cvodeResults_t *results, double *dp)
{
  int i, j, k;
  for(i=0; i<results->neq; i++)
  {   
    for(j=0; j<results->nout+1; j++)
    {
      results->directional[i][j] = 0;
      for(k=0; k<results->nsens; k++)
	results->directional[i][j] += results->sensitivity[i][k][j] * dp[k];
    } 
  }
}


/** Frees results structure cvodeResults filled by the
    CVODE integrator
*/
SBML_ODESOLVER_API void CvodeResults_free(cvodeResults_t *results)
{
  int i;
  
  /* free CVODE results if filled */
  if(results != NULL){
    for( i=0; i<results->nvalues; i++ ) 
      free(results->value[i]);
    free(results->time);
    free(results->value);

    /* free sensitivities */
    CvodeResults_freeSensitivities(results);
    
    /* Adjoint free  */
    if ( results->adjvalue != NULL )
    {
      for( i=0; i<results->neq; i++ )
	free(results->adjvalue[i]);
      free(results->adjvalue);
    }

    free(results);	      
  }
}

/*! @} */

/* initialize cvodeData from cvodeSettings and odeModel (could be
   separated in to functions to further support modularity and
   independence of data structures */
int
CvodeData_initialize(cvodeData_t *data, cvodeSettings_t *opt, odeModel_t *om)
{

  int i;
  Event_t *e;
  ASTNode_t *trigger;


  /* data now also depends on cvodeSettings */
  data->opt = opt;

  /* if discrete data is used via settings */
  if ( opt->observation_data_type == 1 )
    om->discrete_observation_data=1;
  else
    om->discrete_observation_data=0;

  /* free and re-create/initialize memory for optimized ODEs,
     only if compilation is off */
  if ( data->ode )
  {
    /* free ODEs */
    for ( i=0; i<data->neq; i++ )
      if ( data->ode[i] )
	ASTNode_free(data->ode[i]);
    free(data->ode);
    data->ode = NULL;
  }
  if ( !data->opt->compileFunctions )
    ASSIGN_NEW_MEMORY_BLOCK(data->ode, data->neq, ASTNode_t *, 0);
  
  /* initialize values */
  CvodeData_initializeValues(data);
     
  /* set current time */
  data->currenttime = opt->TimePoints[0];

  /* update assigned parameters, in case they depend on new time */
  for ( i=0; i<om->nass; i++ ) 
    data->value[om->neq+i] = evaluateAST(om->assignment[i], data);

  /*
    Then, check if formulas can be evaluated, and cvodeData_t *
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<om->neq; i++ ) evaluateAST(om->ode[i], data);

  /* evaluate event triggers and set flags with their initial state */
  for ( i=0; i<data->nevents; i++ )
  {
    e = Model_getEvent(om->simple, i);
    trigger = (ASTNode_t *) Trigger_getMath(Event_getTrigger(e));
    data->trigger[i] = evaluateAST(trigger, data);
  }
    
  /* RESULTS: Now we should have all variables, and can allocate the
     results structure, where the time series will be stored ...  */

  /* allow results only for finite integrations */
  /* this is the only place where options structure is changed
     internally! StoreResults is overruled by Indefinitely */
  opt->StoreResults = !opt->Indefinitely && opt->StoreResults;

  /* free former results */
  if ( data->results != NULL )
    CvodeResults_free(data->results);

  /* create new results if required */
  if ( opt->StoreResults )
  {
    data->results = CvodeResults_create(data, opt->PrintStep);
    RETURN_ON_FATALS_WITH(0);
  }

  return 1;
}


/* initialize sensitivity initial values at time 0 */
int CvodeData_initializeSensitivities(cvodeData_t *data,
				      cvodeSettings_t *opt,
				      odeModel_t *om, odeSense_t *os)
{
  int i, j, nsens;

  /* 0. catch default case, no parameters/variables selected */
  if ( opt->sensIDs == NULL ) nsens = om->nconst;
  else nsens = opt->nsens;
  
  /* 1: free former sens. results, if different number is requested */
  if ( data->nsens != nsens &&  data->sensitivity != NULL ) 
    CvodeData_freeSensitivities(data);
  
  /* 2: create cvodeData and odeModel structures */
  if ( data->sensitivity == NULL )
  {
    CvodeData_allocateSens(data, om->neq, nsens);
    RETURN_ON_FATALS_WITH(0);
  }

  /* bind to odeSense model */
  data->os = os;
  
  /* 3: write initial values */
  /* (re)set to initial values 0.0 or 1.0 for parameter and
     variable sensitivities, respectively */
  for ( i=0; i<data->neq; i++ )
    for ( j=0; j<data->nsens; j++ )
      if ( os->index_sensP[j] == -1 && os->index_sens[j] == i )
	data->sensitivity[i][j] = 1.0; /* variable A: dA(0)/dA(0) */
      else
	data->sensitivity[i][j] = 0.0; /* variable or parameter */

  /* map initial sensitivities to optional result structure */
  if  ( data->results != NULL )
 {
    /* results from former runs have already been freed before
      result structure was re-allocated */
    CvodeResults_allocateSens(data->results, om->neq, data->nsens,
			      opt->PrintStep);
    /* write initial values for sensitivity */
    for ( i=0; i<os->nsens; i++ )
    {
      data->results->index_sens[i] = os->index_sens[i];
      for ( j=0; j<data->results->neq; j++ )
	data->results->sensitivity[j][i][0] = data->sensitivity[j][i];
    }
    
    /* Adjoint specific  */
    if  ( opt->DoAdjoint )
    {
      CvodeResults_allocateAdjSens(data->results, om->neq,
				   nsens, opt->PrintStep);
      /* write initial values for adj sensitivity */
      for ( i=0; i<data->results->neq; i++ )
	data->results->adjvalue[i][0] = data->adjvalue[i];
    }
  }

  return 1;  
}


/* frees all sensitivity stuff of cvodeData */
static void CvodeData_freeSensitivities(cvodeData_t * data)
{
  int i;

  /* free forward sensitivity */  
  if ( data->sensitivity != NULL )
  {
    for ( i=0; i<data->neq; i++ )
      free(data->sensitivity[i]);
    free(data->sensitivity);
    data->sensitivity = NULL;
  }
  if ( data->p != NULL ) free(data->p);
  if ( data->p_orig != NULL ) free(data->p_orig);
	   
  data->p = data->p_orig = NULL;
  data->sensitivity = NULL;

  if ( data->results )
    CvodeResults_freeSensitivities(data->results);
}
  
/* frees all internal stuff of cvodeData */
static void CvodeData_freeStructures(cvodeData_t * data)
{
  int i;

  if ( data == NULL ) return;

  /* free sensitivity structure */  
  CvodeData_freeSensitivities(data);

  /* free adjoint sensitivity */
  if ( data->adjvalue != NULL ) free(data->adjvalue );
  
  /* free results structure */
  CvodeResults_free(data->results);

  /* free current values array */
  free(data->value);
  
  /* free event trigger flags */
  free(data->trigger);

  if ( data->ode != NULL )
  {
      for ( i=0; i<data->neq; i++ )
	if ( data->ode[i] != NULL )
	  ASTNode_free(data->ode[i]);
      free(data->ode);
      data->ode = NULL;
  }
}

/********* cvodeResults will be created by integration runs *********/

int CvodeResults_allocateSens(cvodeResults_t *results,
			      int neq, int nsens, int nout)
{
  int i, j;

  ASSIGN_NEW_MEMORY_BLOCK(results->index_sens, nsens, int, 0);  
  ASSIGN_NEW_MEMORY_BLOCK(results->sensitivity, neq, double **, 0);
  for ( i=0; i<neq; i++ )
  {
    ASSIGN_NEW_MEMORY_BLOCK(results->sensitivity[i], nsens, double*, 0);
    for ( j=0; j<nsens; j++ )
      ASSIGN_NEW_MEMORY_BLOCK(results->sensitivity[i][j], nout+1, double, 0);
  }
  
  results->nsens = nsens;
  results->neq = neq;    

  ASSIGN_NEW_MEMORY_BLOCK(results->directional, neq, double *, 0);
  for ( i=0; i<neq; i++ )  
    ASSIGN_NEW_MEMORY_BLOCK(results->directional[i], nout+1, double, 0);

  return 1;
}



int CvodeResults_allocateAdjSens(cvodeResults_t *results,
				 int neq, int nadjsens, int nout)
{
  int i;

  ASSIGN_NEW_MEMORY_BLOCK(results->adjvalue, neq, double *, 0);
  for ( i=0; i<neq; i++ ) 
    ASSIGN_NEW_MEMORY_BLOCK(results->adjvalue[i], nout+1, double, 0); 

  return 1;
}


/* Creates cvodeResults, the structure that stores
   CVODE integration results */
cvodeResults_t *CvodeResults_create(cvodeData_t * data, int nout)
{
  int i;
  cvodeResults_t *results;

  ASSIGN_NEW_MEMORY(results, struct cvodeResults, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(results->time, nout+1, double, NULL);
  
  /* The 2-D array `value' contains the time courses, that are
     calculated by ODEs (SBML species, or compartments and parameters
     defined by rate rules.  */
  ASSIGN_NEW_MEMORY_BLOCK(results->value, data->nvalues, double *, NULL);

  results->nvalues = data->nvalues;    

  for ( i=0; i<data->nvalues; i++ )
    ASSIGN_NEW_MEMORY_BLOCK(results->value[i], nout+1, double, NULL);

  results->sensitivity = NULL;

  results->directional = NULL;

  /* adjoint */
  results->adjvalue = NULL;  

  return results;  
}

/* frees all sensitivity structures of cvodeResults */
static void CvodeResults_freeSensitivities(cvodeResults_t *results)
{
  int i, j;
  if ( results->sensitivity != NULL )
  {
    for ( i=0; i<results->neq; i++ )
    {
      for ( j=0; j<results->nsens; j++ )
	free(results->sensitivity[i][j]);
      free(results->sensitivity[i]);
    }
    free(results->sensitivity);
    free(results->index_sens);
    results->sensitivity = NULL;
    results->index_sens = NULL;
  }
  
  if ( results->directional != NULL )
  {
    for ( i=0; i<results->neq; i++ )
      free(results->directional[i]);
    free(results->directional);
    results->directional = NULL;
  }
}
/* End of file */
