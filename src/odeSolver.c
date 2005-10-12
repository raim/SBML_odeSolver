/*
  Last changed Time-stamp: <2005-10-12 14:45:04 raim>
  $Id: odeSolver.c,v 1.14 2005/10/12 12:52:08 raimc Exp $
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


SBMLResults_t *
Model_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set) {
  
  SBMLDocument_t *d2 = NULL;
  Model_t *m;
  odeModel_t *om;
  integratorInstance_t *ii; 
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
  
  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }
  /* ... well done. */
  return(results);
}

/** 

*/

SBMLResults_t **
Model_odeSolverBatch (SBMLDocument_t *d,
		      cvodeSettings_t *set, VarySettings vary) {

  int i;
  double value, increment;
  SBMLDocument_t *d2 = NULL;
  Model_t *m;

  odeModel_t *om;
  integratorInstance_t *ii;
  variableIndex_t *vi;
  SBMLResults_t **results;

  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    m = SBMLDocument_getModel(d2);
  }
  else {
    m = SBMLDocument_getModel(d);
  }

  ASSIGN_NEW_MEMORY_BLOCK(results, vary.steps+1, SBMLResults_t *, NULL);
  
  /** At first, ODEModel_create, attempts to construct a simplified
     SBML model with reactions replaced by ODEs.
     See comments in Model_odeSolver for details.
  */
  
  om = ODEModel_create(m, set->UseJacobian);      
  RETURN_ON_FATALS_WITH(NULL);
  vi = ODEModel_getVariableIndex(om, vary.id);
  if ( vi == NULL )
    return NULL;
  
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

  value = vary.start;
  increment = (vary.end - vary.start) / vary.steps;

  for ( i=0; i<=vary.steps; i++ ) {

    IntegratorInstance_setVariableValue(ii, vi, value);
    printf("set %s to %f\n", ODEModel_getVariableName(om, vi),
	   IntegratorInstance_getVariableValue(ii, vi) );
    
    while (!IntegratorInstance_timeCourseCompleted(ii)) {
      if (!IntegratorInstance_integrateOneStep(ii))
	IntegratorInstance_handleError(ii);
    }
    RETURN_ON_FATALS_WITH(NULL);
    
    printf("now %s is %f\n", ODEModel_getVariableName(om, vi),
	   IntegratorInstance_getVariableValue(ii, vi) );
    
    /* map cvode results to SBML compartments,
       species and parameters  */
    results[i] = SBMLResults_fromIntegrator(m, ii);
    IntegratorInstance_reset(ii);
    value = value + increment;
  }

  /* free integration data */
  IntegratorInstance_free(ii);
  /* free odeModel */
  ODEModel_free(om);

  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }

  return(results);

}

/** 

*/

SBMLResults_t ***
Model_odeSolverBatch2 (SBMLDocument_t *d, cvodeSettings_t *settings,
		      VarySettings vary1, VarySettings vary2) {

  int i, j;
  double value1, increment1, value2, increment2;
  SBMLResults_t ***results;
  SBMLDocument_t *d2 = NULL;
  Model_t *m;

  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    m = SBMLDocument_getModel(d2);
  }
  else {
    m = SBMLDocument_getModel(d);
  }
      
  ASSIGN_NEW_MEMORY_BLOCK(results, vary1.steps+1, SBMLResults_t **, NULL)
  for ( i=0; i<=vary1.steps; i++ ) 
    ASSIGN_NEW_MEMORY_BLOCK(results[i], vary2.steps+1, SBMLResults_t *, NULL);

  value1 = vary1.start;
  increment1 = (vary1.end - vary1.start) / vary1.steps;
  
  value2 = vary2.start;
  increment2 = (vary2.end - vary2.start) / vary2.steps;
  
  for ( i=0; i<=vary1.steps; i++ ) {
    if ( ! Model_setValue(m, vary1.id, vary1.rid, value1) ) {
      Warn(stderr, "Parameter for variation not found in the model.",
	   vary1.id);
      return(NULL);
    }
    value2 = vary2.start; 
    for ( j=0; j<=vary2.steps; j++ ) {      
      if ( ! Model_setValue(m, vary2.id, vary2.rid, value2) ) {
	Warn(stderr, "Parameter for variation not found in the model.",
	     vary2.id);
      return(NULL);
      }
      
      results[i][j] = Model_odeSolver(d, settings);
      value2 = value2 + increment2;
    }
    value1 = value1 + increment1;
  }

  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }

  return(results);

}

int
Model_setValue(Model_t *m, const char *id, const char *rid, double value) {

  int i;
  Compartment_t *c;
  Species_t *s;
  Parameter_t *p;
  Reaction_t *r;
  KineticLaw_t *kl;

  if ( (r = Model_getReactionById(m, rid)) != NULL ) {
    kl = Reaction_getKineticLaw(r);
    for ( i=0; i<KineticLaw_getNumParameters(kl); i++ ) {
      p = KineticLaw_getParameter(kl, i);
      if ( strcmp(id, Parameter_getId(p)) == 0 ) {
	Parameter_setValue(p, value);
	return 1;
      }
    }
  }
  if ( (c = Model_getCompartmentById(m, id)) != NULL ) {
    Compartment_setSize(c, value);
    return 1;
  }
  if ( (s = Model_getSpeciesById(m, id)) != NULL ) {
    if ( Species_isSetInitialAmount(s) ) {
      Species_setInitialAmount(s, value);
    }
    else {
      Species_setInitialConcentration(s, value);
    }
    return 1;
  }
  if ( (p = Model_getParameterById(m, id)) != NULL ) {
    Parameter_setValue(p, value);
    return 1;
  }
  return 0;  
}

/** The function Results_fromCvode(cvodeData_t *data)
   maps the integration results of CVODE
   back to SBML structures.
*/

SBMLResults_t *
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
  if ( data == NULL ) {
    fatal(stderr, "No data, please construct ODE system first.\n");
    return NULL;
  }
  else if ( data->results == NULL ) {    
    fatal(stderr, "No results, please integrate first.\n");
    return NULL;
  }

  sbml_results = SBMLResults_create(m, data->results->nout+1);    

  /* Allocating temporary kinetic law ASTs, for evaluation of fluxes */

  if(!(kls =
       (ASTNode_t **)calloc(Model_getNumReactions(m),
			    sizeof(ASTNode_t *)))) {
    fprintf(stderr, "failed!\n");
  }  
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

/** writes current simulation data to
    original model */
int
updateModel(cvodeData_t *data, int nout) {

  int i;
  Species_t *s;
  Compartment_t *c;
  Parameter_t *p;  
  
  for ( i=0; i<data->nvalues; i++ ) {
    if ( (s = Model_getSpeciesById(data->model->m, data->model->names[i]))
	 != NULL ) {
      Species_setInitialConcentration(s, data->results->value[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->model->m,
					    data->model->names[i])) != NULL ) {
      Compartment_setSize(c, data->results->value[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->model->m,
					  data->model->names[i])) !=  NULL ) {
      Parameter_setValue(p, data->results->value[i][nout]);
    }
  }

  return 1;

}


/* End of file */
