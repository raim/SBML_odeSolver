/*
  Last changed Time-stamp: <2005-10-27 19:54:48 raim>
  $Id: cvodedata.c,v 1.18 2005/10/27 17:58:51 raimc Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/Model.h>

/* own header files */
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/integratorSettings.h"
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



/* initialize cvodeData from cvodeSettings and odeModel (could be
   separated in to functions to further support modularity and
   independence of data structures */
int
CvodeData_initialize(cvodeData_t *data, cvodeSettings_t *opt, odeModel_t *om)
{

  int i;
  Parameter_t *p;
  Species_t *s;
  Compartment_t *c;

  Model_t *ode = om->simple;

  /* data now also depends on cvodeSettings */
  data->opt = opt;
     

  /*
    First, fill cvodeData_t  structure with data from
    the ODE model
  */

  for ( i=0; i<data->nvalues; i++ ) {
    if ( (s = Model_getSpeciesById(ode, om->names[i])) )
      data->value[i] = Species_getInitialConcentration(s);
    else if ( (c = Model_getCompartmentById(ode, om->names[i])) )
      data->value[i] = Compartment_getSize(c);
    else if ((p = Model_getParameterById(ode, om->names[i])) )
      data->value[i] = Parameter_getValue(p);
  }

  /* set current time */
  data->currenttime = opt->TimePoints[0];
  
  /*
    Then, check if formulas can be evaluated, and cvodeData_t *
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<om->neq; i++ ) 
    evaluateAST(om->ode[i], data);

  /* initialize assigned parameters */
  for ( i=0; i<om->nass; i++ ) 
    data->value[om->neq+i] = evaluateAST(om->assignment[i],data);

  /* Now we should have all variables, and can allocate the
     results structure, where the time series will be stored ...  */
  /* allow results only for finite integrations */
  opt->StoreResults = !opt->Indefinitely && opt->StoreResults;
  /* free former results */
  if ( data->results != NULL )
      CvodeResults_free(data->results);
  /* create new results if required */
  if ( opt->StoreResults ) {
    data->results = CvodeResults_create(data, opt->PrintStep);
    RETURN_ON_FATALS_WITH(0);
  }
  
  return 1;
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
  CvodeResults_free(data->results);

  /* free current values array */
  free(data->value);
  
  /* free event trigger flags */
  free(data->trigger);

}


/** Creates cvodeResults, the structure that stores
    CVODE integration results
*/

cvodeResults_t *CvodeResults_create(cvodeData_t * data, int nout) {

  int i;
  cvodeResults_t *results;

  ASSIGN_NEW_MEMORY(results, struct cvodeResults, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(results->time, nout+1, double, NULL)
  
  /* The 2-D array `value' contains the time courses, that are
     calculated by ODEs (SBML species, or compartments and parameters
     defined by rate rules.
  */
    ASSIGN_NEW_MEMORY_BLOCK(results->value, data->nvalues, double *, NULL);

  results->nvalues = data->nvalues;    

  for ( i=0; i<data->nvalues; ++i )
    ASSIGN_NEW_MEMORY_BLOCK(results->value[i], nout+1, double, NULL)
  
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
void CvodeResults_free(cvodeResults_t *results) {

  int i;
  /* free CVODE results if filled */
  if(results != NULL){
    for(i=0;i<results->nvalues;i++){
      free(results->value[i]);
    }
    free(results->time);
    free(results->value);
    free(results);	      
  }
}


/* End of file */
