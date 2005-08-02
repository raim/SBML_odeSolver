/*
  Last changed Time-stamp: <2005-08-02 17:08:42 raim>
  $Id: odeSolver.c,v 1.12 2005/08/02 15:47:59 raimc Exp $
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
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/solverError.h"


SBMLResults_t *
Model_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set) {
  
  SBMLDocument_t *d2 = NULL;
  cvodeData_t *data;
  SBMLResults_t *results;
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
  
  /** At first, the function constructODEs(m)
      will attempt to construct a simplified SBML model,
      that only consists of species and their ODEs, represented
      as Rate Rules, and of Events. All constant species, parameters
      compartments, assignment rules and function definitions
      will be replaced by their values or expressions respectively
      in all remaining formulas (ie. rate and algebraic rules and
      events).
      Then the initial values and ODEs of the remaining species
      will be written to the structure cvodeData_t *data.
  */
  
  data = constructODEs(m, set->UseJacobian);
  /** Errors, found during odeConstruct, will cause the program to exit,
      e.g. when some mathematical expressions are missing.
  */
  SolverError_haltOnErrors();
    
  /** Set integration parameters:
      Now that we have arrived here, we can set the parameters
      for integration, like
      the end time and the number of steps to be printed out,
      absolute and relative error tolerances,
      use of exact or approximated Jacobian matrix,
      printing of messages during integration,
      event handling, steady state detection, and
      runtime printing of results.
      And then ..
  */
  data->tout  = set->Time;
  data->nout  = set->PrintStep;
  data->tmult = set->Time / set->PrintStep;
  data->currenttime = 0.0;
  data->t0 = 0.0;
  data->opt = set;

  /* allow setting of Jacobian,
     only if its construction was succesfull */
   if ( data->opt->UseJacobian == 1 ) { 
     data->opt->UseJacobian = set->UseJacobian && data->model->jacobian; 
   } 
  
  /** .... we can call the integrator function,
      that invokeds CVODE and stores results.
      The function will also handle events and
      check for steady states.
  */    

  integrator(data, 0, 0, stdout);
  SolverError_dump();
  RETURN_ON_FATALS_WITH(NULL);
  SolverError_clear();

  /* Write simulation results into result structure */
  results = Results_fromCvode(data); 
  CvodeData_free(data);
  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }
  return(results);
}

SBMLResults_t **
Model_odeSolverBatch (SBMLDocument_t *d,
		      cvodeSettings_t *settings, VarySettings vary) {

  int i;
  double value, increment;
  SBMLResults_t **results;
  SBMLDocument_t *d2 = NULL;
  Model_t *m;

  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    d = d2;
  }
  m = SBMLDocument_getModel(d);
    
  if(!(results = (SBMLResults_t **)calloc(vary.steps+1, sizeof(*results)))){
    fprintf(stderr, "failed!\n");
  }

  value = vary.start;
  increment = (vary.end - vary.start) / vary.steps;
  
  for ( i=0; i<=vary.steps; i++ ) {
    
      
    if ( ! Model_setValue(m, vary.id, vary.rid, value) ) {
      Warn(stderr, "Parameter for variation not found in the model.", vary.id);
      return(NULL);
    }
    results[i] = Model_odeSolver(d, settings);
    value = value + increment;
  }

  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }

  return(results);

}

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
    d = d2;
  }
  m = SBMLDocument_getModel(d);
    
  if(!(results = (SBMLResults_t ***)calloc(vary1.steps+1, sizeof(**results)))){
    fprintf(stderr, "failed!\n");
  }
  for ( i=0; i<=vary1.steps; i++ ) {
    if(!(results[i] = (SBMLResults_t **)calloc(vary2.steps+1, sizeof(*results)))){
      fprintf(stderr, "failed!\n");
    }    
  }

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

/* The function Results_fromCvode(cvodeData_t *data)
   maps the integration results of CVODE
   back to SBML structures.
*/

SBMLResults_t *
Results_fromCvode(cvodeData_t *data) {

  int i, j, k;
  SBMLResults_t *sbml_results;
  Model_t *m;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;
  
  cvodeResults_t *cvode_results;
  timeCourse_t *tc;
  
  if ( data == NULL ) {
    fatal(stderr, "No data, please construct ODE system first.\n");
    return NULL;
  }
  else if ( data->results == NULL ) {    
    fatal(stderr, "No results, please integrate first.\n");
    return NULL;
  }

  sbml_results = SBMLResults_create(data->model->m, data->results->nout+1);    
  cvode_results = data->results;
  m = data->model->m;

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
