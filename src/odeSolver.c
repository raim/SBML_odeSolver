/*
  Last changed Time-stamp: <2005-10-18 16:28:08 raim>
  $Id: odeSolver.c,v 1.23 2005/10/18 14:31:05 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/odeSolver.h"

static int globalizeParameter(Model_t *, char *id, char *rid);
static int localizeParameter(Model_t *, char *id, char *rid);

/**

*/

SBML_ODESOLVER_API SBMLResults_t *
SBML_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set) {

  SBMLDocument_t *d2 = NULL;
  Model_t *m = NULL;
  SBMLResults_t *results;
  
  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) != 2 ) {
    d2 = convertModel(d);
    m = SBMLDocument_getModel(d2);    
  }
  else {
    m = SBMLDocument_getModel(d);
  }  
  RETURN_ON_FATALS_WITH(NULL);

  results = Model_odeSolver(m, set);
  
  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }
  
  return results;
  
}


/** 

*/

SBML_ODESOLVER_API SBMLResultsMatrix_t *
SBML_odeSolverBatch(SBMLDocument_t *d, cvodeSettings_t *set,
		    varySettings_t *vs) 
{

  SBMLDocument_t *d2 = NULL;
  Model_t *m = NULL;
  SBMLResultsMatrix_t *resM;
  
  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) != 2 ) {
    d2 = convertModel(d);
    m = SBMLDocument_getModel(d2);    
  }
  else {
    m = SBMLDocument_getModel(d);
  }  
  RETURN_ON_FATALS_WITH(NULL);
  
  resM = Model_odeSolverBatch(m, set, vs);
  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }
  
  return resM;
    
}


/**

*/

SBML_ODESOLVER_API SBMLResults_t *
Model_odeSolver(Model_t *m, cvodeSettings_t *set) {
  
  odeModel_t *om;
  integratorInstance_t *ii; 
  SBMLResults_t *results;
  
  /** At first, ODEModel_create, attempts to construct a simplified
     SBML model with reactions replaced by ODEs. SBML RateRules,
     AssignmentRules,AlgebraicRules and Events are copied to the
     simplified model. AlgebraicRules or missing mathematical
     expressions produce fatal errors and appropriate messages.  All
     function definitions are replaced by their values or expressions
     respectively in all remaining formulas (ie. rules and events). If
     that conversion was successful, an internal model structure
     (odeModel) is created, that contains indexed versions of all
     formulae (AFM's AST_INDEX) where the index of a former AST_NAME
     corresponds to its position in a value array (double *), that is
     used to store current values and to evaluate AST formulae during
     integration.
  */

  om = ODEModel_create(m, set->UseJacobian);      
  RETURN_ON_FATALS_WITH(NULL);
  /**
     Second, an integratorInstance is created from the odeModel
     and the passed cvodeSettings. If that worked out ...
  */
  
  ii = IntegratorInstance_create(om, set);
  RETURN_ON_FATALS_WITH(NULL);

  /** .... the integrator loop can be started,
      that invoking CVODE to move one time step and store.
      The function will also handle events and
      check for steady states.
  */
  while (!IntegratorInstance_timeCourseCompleted(ii)) {
    if (!IntegratorInstance_integrateOneStep(ii))
      IntegratorInstance_handleError(ii);
  }  
  RETURN_ON_FATALS_WITH(NULL);

  /* map cvode results to SBML compartments,
     species and parameters  */
  results = SBMLResults_fromIntegrator(m, ii);

  /* free integration data */
  IntegratorInstance_free(ii);
  /* free odeModel */
  ODEModel_free(om);
  
  /* ... well done. */
  return(results);
}


/** 

*/

SBML_ODESOLVER_API SBMLResultsMatrix_t *
Model_odeSolverBatch (Model_t *m, cvodeSettings_t *set,
		      varySettings_t *vs) {


  int i, j;
  double value, increment;
 
  odeModel_t *om;
  integratorInstance_t *ii;
  variableIndex_t *vi;
  SBMLResultsMatrix_t *resM;

  char *local_param;


  resM = SBMLResultsMatrix_allocate(vs->nrparams, vs->nrdesignpoints);
 
  /* At first, globalize all local (kineticLaw) parameters to be varied */
  for ( i=0; i<vs->nrparams; i++ ) 
    if ( vs->rid[i] != NULL ) 
      globalizeParameter(m, vs->id[i], vs->rid[i]);     
    
 
  /* At first, ODEModel_create, attempts to construct a simplified
     SBML model with reactions replaced by ODEs.
     See comments in Model_odeSolver for details.  */
  om = ODEModel_create(m, set->UseJacobian);      
  RETURN_ON_FATALS_WITH(NULL);
 
  /* an integratorInstance is created from the odeModel and the passed
     cvodeSettings. If that worked out ...  */  
  ii = IntegratorInstance_create(om, set);
  RETURN_ON_FATALS_WITH(NULL);
      
  /* now, work through the passed parameters in varySettings */
  for ( i=0; i<vs->nrparams; i++ ) {

    /* get the index for parameter i */
    if ( vs->rid[i] != NULL ) {
      ASSIGN_NEW_MEMORY_BLOCK(local_param,
			      strlen(vs->id[i]) + strlen(vs->rid[i]) + 4,
			      char , 0);
      sprintf(local_param, "r_%s_%s", vs->rid[i], vs->id[i]);
      vi = ODEModel_getVariableIndex(om, local_param);
      free(local_param);
    }
    else
      vi = ODEModel_getVariableIndex(om, vs->id[i]);
    
    if ( vi == NULL )
      return NULL;
  
    /* then, work through all values for this parameter */
    for ( j=0; j<vs->nrdesignpoints; j++ ) {

      /* Set the value!*/
      IntegratorInstance_setVariableValue(ii, vi, vs->params[i][j]);


      /** .... the integrator loop can be started, that invoking
	  CVODE to move one time step and store.  The function will
	  also handle events and check for steady states.  */      
      while (!IntegratorInstance_timeCourseCompleted(ii)) {
	if (!IntegratorInstance_integrateOneStep(ii))
	  IntegratorInstance_handleError(ii);
      }
    
      RETURN_ON_FATALS_WITH(NULL);
    
      /* map cvode results to SBML compartments,
	 species and parameters  */
      resM->results[i][j] = SBMLResults_fromIntegrator(m, ii);
      IntegratorInstance_reset(ii);
    }
  }
  /* localize parameters again, unfortunately the new
     globalized parameter cannot be freed currently  */
  for ( i=0; i<vs->nrparams; i++ ) 
    if ( vs->rid[i] != NULL ) 
      localizeParameter(m, vs->id[i], vs->rid[i]);     

  /* free variableIndex, used for setting values */
  VariableIndex_free(vi);
  /* free integration data */
  IntegratorInstance_free(ii);
  /* free odeModel */
  ODEModel_free(om);
  /* ... well done. */
  return(resM);

}

static int globalizeParameter(Model_t *m, char *id, char *rid) {

  int i, found;
  Reaction_t *r;
  KineticLaw_t *kl;
  Parameter_t *p, *p_global;
  ASTNode_t *math;
  char *newname;
 
  r = Model_getReactionById (m, (const char *) rid);
  
  if ( r == NULL )
    return 0;
  
  kl = Reaction_getKineticLaw(r);
  math = (ASTNode_t *)KineticLaw_getMath(kl);

  ASSIGN_NEW_MEMORY_BLOCK(newname, strlen(id) + strlen(rid) + 4, char , 0);
  sprintf(newname, "r_%s_%s", rid, id);
  AST_replaceNameByName(math, (const char *) id,  (const char *) newname);

  found = 0;
  
  for ( i=0; i<KineticLaw_getNumParameters(kl); i++ ) {
    p = KineticLaw_getParameter(kl, i);
    if ( strcmp(Parameter_getId(p), id) == 0 ) {
      p_global = Model_createParameter(m);
      Parameter_setConstant(p_global, 1);
      Parameter_setId(p_global, newname);
      if (Parameter_isSetValue(p)) 
	Parameter_setValue(p_global, Parameter_getValue(p));
      if(Parameter_isSetUnits(p)) 
	Parameter_setUnits(p_global, Parameter_getUnits(p));
      if (Parameter_isSetName(p)) 
	Parameter_setName(p_global, Parameter_getName(p));
      found = 1;
    }
  }
  free(newname);
  return found;
}

static int localizeParameter(Model_t *m, char *id, char *rid) {

  int i, found;
  Reaction_t *r;
  KineticLaw_t *kl;
  Parameter_t *p, *p_global;
  ASTNode_t *math;
  char *newname;
  
  r = Model_getReactionById (m, (const char *) rid);
  
  if ( r == NULL )
    return 0;
  
  kl = Reaction_getKineticLaw(r);
  math = (ASTNode_t *)KineticLaw_getMath(kl);  
  ASSIGN_NEW_MEMORY_BLOCK(newname, strlen(id) + strlen(rid) + 4, char , 0);
  sprintf(newname, "r_%s_%s", rid, id);
  AST_replaceNameByName(math, (const char *) newname, (const char *) id);

  /* unfortunately the parameter cannot be freed currently */
  if ((p = Model_getParameterById(m, newname)) != NULL) {
    Parameter_setId(p, "notUsedAnymore_asdf");
  }
  else
    return 0;

  free(newname);
  return 1;

}

/** Allocate varySettings structure for settings for parameter
    variation batch runs: nrparams is the number of parameters to be
    varied, and nrdesignpoints is the number of values to be tested
    for each parameter.
*/

SBML_ODESOLVER_API 
varySettings_t *VarySettings_allocate(int nrparams, int nrdesignpoints)
{
  int i;
  varySettings_t *vs;
  ASSIGN_NEW_MEMORY(vs, struct varySettings, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(vs->id, nrparams, char *, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(vs->rid, nrparams, char *, NULL);
  ASSIGN_NEW_MEMORY_BLOCK(vs->params, nrparams, double *, NULL);
  for ( i=0; i<nrparams; i++ ) {
    ASSIGN_NEW_MEMORY_BLOCK(vs->params[i], nrdesignpoints, double, NULL);
  } 
  vs->nrparams = 0; /* set to 0, will be used as counter in addParameters */
  vs->nrdesignpoints = nrdesignpoints;
  return vs;
}


/** 

*/

SBML_ODESOLVER_API 
int VarySettings_addParameter(varySettings_t *vs, char *id, char *rid,
			      double start, double end)
{
  int j;
  double *designpoints;

  ASSIGN_NEW_MEMORY_BLOCK(designpoints, vs->nrdesignpoints, double, 0);

  /* calculating internal parameter array */  
  for ( j=0; j<vs->nrdesignpoints; j++ )
    designpoints[j] =  start + j*((end-start)/(vs->nrdesignpoints-1));

  j = VarySettings_addParameterSeries(vs, id, rid, designpoints);
  free(designpoints);
  return j;  
}


/* */

int VarySettings_addParameterSeries(varySettings_t *vs, char *id, char *rid,
				    double *designpoints)
{
  int j;  
  
  /* filling internal parameter array */
  for ( j=0; j<vs->nrdesignpoints; j++ )
    VarySettings_setValue(vs, vs->nrparams, j, designpoints[j]);
  
  VarySettings_setParameterName(vs, vs->nrparams, id, rid);  
  
  return vs->nrparams++;  /* counts already filled parametervalues */
  
}


/** 

*/

SBML_ODESOLVER_API 
const char *VarySettings_getParameterName(varySettings_t *vs, int i)
{   
  return (const char *) vs->id[i];
}


/** 

*/

SBML_ODESOLVER_API 
int VarySettings_setParameterName(varySettings_t *vs, int i,
				  char *id, char *rid)
{

  if ( vs->id[i] != NULL )
    free(vs->id[i]);
  if  ( vs->rid[i] != NULL )
    free(vs->rid[i]);
   
  /* concatenating parameter reaction id: will be `globalized' from
     input model, the same way. For global parameters to be changed
     rid must be passed as "" */
  if ( rid != NULL ) {
    ASSIGN_NEW_MEMORY_BLOCK(vs->rid[i], strlen(rid)+1, char, 0);
    sprintf(vs->rid[i], "%s", rid);    
  }
  else 
    vs->rid[i] = NULL;

  ASSIGN_NEW_MEMORY_BLOCK(vs->id[i], strlen(id)+1, char, 0);
  sprintf(vs->id[i], "%s", id);
  

  return 1;
}


/* */


int VarySettings_addParameterSet(varySettings_t *vs,
				 double **designpoints, char **id, char **rid)
{
  int i, j;
  for ( i=0; i<vs->nrparams; i++ )
    j += VarySettings_addParameterSeries(vs, id[i], rid[i], designpoints[i]);
  return j;      
}

/** Get the jth value of the ith parameter
*/

SBML_ODESOLVER_API
double VarySettings_getValue(varySettings_t *vs, int i, int j)
{
  return vs->params[i][j];
}


/** Set the jth value of the ith parameter, where
    0 <= i < nrparams  and
    0 <= j < nrdesignpoints
    as used for varySettings_allocate
*/

SBML_ODESOLVER_API
void VarySettings_setValue(varySettings_t *vs, int i, int j, double value)
{
  vs->params[i][j] = value;
}


/** Print all parameters and their values in varySettings
*/

SBML_ODESOLVER_API 
void VarySettings_dump(varySettings_t *vs)
{
  int i, j;
  printf("\n");
  printf("Design Series for %d Parameter(s) and %d values:",
	 vs->nrparams, vs->nrdesignpoints);fflush(stdout);
  for ( i=0; i<vs->nrparams; i++ ) {
    printf("\n%d. %s: ", i, vs->id[i]);
    for ( j=0; j<vs->nrdesignpoints; j++ ) {
      printf("%.3f ", vs->params[i][j]);
    }    
  }
  printf("\n\n");
}


/** Frees varySettings structure
*/

SBML_ODESOLVER_API 
void VarySettings_free(varySettings_t *vs)
{
  int i, j;
  
  for ( i=0; i<vs->nrparams; i++ ) {
    free(vs->id[i]);
    free(vs->rid[i]);
    free(vs->params[i]);
  }
  free(vs->id);
  free(vs->rid);
  free(vs->params);
  free(vs);    
}

/** Maps the integration results from internal back
    to SBML structures in  model `m' 
*/

SBML_ODESOLVER_API SBMLResults_t *
SBMLResults_fromIntegrator(Model_t *m, integratorInstance_t *ii) {

  int i, j, k;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;
  timeCourse_t *tc;
  SBMLResults_t *sbml_results;

  cvodeData_t *data = ii->data;
  cvodeResults_t *cvode_results = data->results;

  /* check if data is available */
  if ( data == NULL ) 
    return NULL;
  else if ( data->results == NULL ) 
    return NULL;

  sbml_results = SBMLResults_create(m, data->results->nout+1);    

  /* Allocating temporary kinetic law ASTs, for evaluation of fluxes */

  ASSIGN_NEW_MEMORY_BLOCK(kls, Model_getNumReactions(m), ASTNode_t *, NULL);

  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    kl = Reaction_getKineticLaw(r);
    kls[i] = copyAST(KineticLaw_getMath(kl));
    AST_replaceNameByParameters(kls[i], KineticLaw_getListOfParameters(kl));
    AST_replaceConstants(m, kls[i]);
  }
  
  
  /*
    Filling results for each calculated timepoint.
  */
  for ( i=0; i<sbml_results->timepoints; i++ ) {
    
    /* writing time steps */
    sbml_results->time[i] = cvode_results->time[i];
    /* updating time and values in cvodeData_t *for calculations */
    data->currenttime = cvode_results->time[i]; 
    for ( j=0; j<data->nvalues; j++ ) {
      data->value[j] = cvode_results->value[j][i]; 
    }
    /* filling time courses for SBML species  */
    tc = sbml_results->species;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in cvodeData_t for values */
       for ( k=0; k<data->nvalues; k++ ) {
	if ( (strcmp(tc->names[j], data->model->names[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];	
	}
      }      
    }
    
    /* filling variable compartment time courses */
    tc = sbml_results->compartments;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in cvodeData_t for values */
      for ( k=0; k<data->nvalues; k++ ) {
	if ( (strcmp(tc->names[j], data->model->names[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	}
      }  
    }         

    /* filling variable parameter time courses */
    tc = sbml_results->parameters;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in cvodeData_t for values */
      for ( k=0; k<data->nvalues; k++ ) {
	if ( (strcmp(tc->names[j], data->model->names[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	}
      }        
    }

    /* filling reaction flux time courses */
    tc = sbml_results->fluxes;
    for ( j=0; j<tc->num_val; j++ ) {
      tc->values[i][j] = evaluateAST(kls[j], data);
    }

  }

  /* freeing temporary kinetic law ASTs */
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    ASTNode_free(kls[i]);
  }  
  free(kls);
  
  return(sbml_results);
}


/* End of file */
