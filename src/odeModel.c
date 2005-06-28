#include "sbmlsolver/odeModel.h"

#include <malloc.h>
#include <string.h>

#include <sbml/common/common.h>  /* for safe_free */

#include "sbmlsolver/sbml.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/util.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"

static odeModel_t *ODEModel_allocate(int neq, int nconst,
				     int nass, int nevents)
{
  int i;
  odeModel_t *data;

  if(!(data = (odeModel_t *) malloc(sizeof(odeModel_t))))
    fprintf(stderr, "failed!\n");
  
  if(!(data->species = (char **)calloc(neq, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  if(!(data->speciesname = (char **)calloc(neq, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  if(!(data->ode = (ASTNode_t **)calloc(neq, sizeof(ASTNode_t *))))    
    fprintf(stderr, "failed!\n");
  
  if(!(data->jacob =
       (ASTNode_t ***)calloc(neq, sizeof(ASTNode_t **))))
    fprintf(stderr, "failed!\n");
  for ( i=0; i<neq; i++ ) {
    if(!(data->jacob[i] = (ASTNode_t **)calloc(neq, sizeof(ASTNode_t *))))
      fprintf(stderr, "failed!\n");     
  }

  if(!(data->parameter = (char **)calloc(nconst, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  
  if(!(data->ass_parameter = (char **)calloc(nass, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  if(!(data->assignment =
       (ASTNode_t **)calloc(nass, sizeof(ASTNode_t *))))    
    fprintf(stderr, "failed!\n");  

  return data ;
}

SBML_ODESOLVER_API odeModel_t *
ODEModel_createFromModel(Model_t *model)
{
    return ODEModel_createFromModelAndOptions(
                model,
                1 /* simplify */,
                0 /* no determinant */,
                "" /* all parameters to be replaced during simplification */);
}

SBML_ODESOLVER_API odeModel_t *
ODEModel_createFromModelAndOptions(Model_t *m,
				   int simplify,
				   int determinant,
				   const char *parameterNotToBeReplaced)
{
  int i, j, found, neq, nconst, nass, nevents;
  Model_t *ode;
  Parameter_t *p;
  Species_t *s;
  Rule_t *rl;
  AssignmentRule_t *ar;
  RateRule_t *rr;
  SBMLTypeCode_t type;  
  ASTNode_t *math;  
  odeModel_t *data;

  /* C: construct ODE model */
  ode = Model_reduceToOdes(m, simplify, parameterNotToBeReplaced);

  RETURN_ON_ERRORS_WITH(NULL);

  neq    = 0;
  nconst = 0;
  nass   = 0;
  found  = 0;

  /*
    counting number of equations (ODEs/rateRules) and Parameters
    to initialize CvodeData structure
  */
  for ( j=0; j<Model_getNumRules(ode); j++ ) {

    rl = Model_getRule(ode,j);
    type = SBase_getTypeCode((SBase_t *)rl);    
    if ( type == SBML_RATE_RULE ) {
      neq++;
    }
    if ( type == SBML_ASSIGNMENT_RULE ) {
      nass++;
    }    
  }
  for ( j=0; j<Model_getNumParameters(ode); j++ ) {
    p = Model_getParameter(ode,j);
    if ( Parameter_getConstant(p) ) {
      nconst++;
    }
  }
  
  nevents = Model_getNumEvents(ode);
  
  data = ODEModel_allocate(neq, nconst, nass, nevents);
  data->neq = neq;
  data->nconst = nconst;
  data->nass = nass;

  /*
    filling structure with data from
    the ODE model
  */
  nconst = 0;  
  for ( i=0; i<Model_getNumParameters(ode); i++ ) {
    p = Model_getParameter(ode, i);
    if ( Parameter_getConstant(p) ) {     
      data->parameter[nconst] =
	(char *) calloc(strlen(Parameter_getId(p))+1, sizeof(char));
      sprintf(data->parameter[nconst], Parameter_getId(p));
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
      math = copyAST(Rule_getMath(rl));
      data->ode[neq] = math;
      s = Model_getSpeciesById(ode, RateRule_getVariable(rr));

      data->species[neq] =
	(char *) calloc(strlen(RateRule_getVariable(rr))+1, sizeof(char));
      sprintf(data->species[neq],RateRule_getVariable(rr));

      if ( Species_isSetName(s) ) {
	data->speciesname[neq] =
	  (char *) calloc(strlen(Species_getName(s))+1, sizeof(char));
	sprintf(data->speciesname[neq],Species_getName(s));
      }
      else {
	data->speciesname[neq] =
	  (char *) calloc(strlen(RateRule_getVariable(rr))+1, sizeof(char));
	sprintf(data->speciesname[neq], RateRule_getVariable(rr));
      }
      neq++;      
    }
    else if ( type == SBML_ASSIGNMENT_RULE ) {
      
      ar = (AssignmentRule_t *)rl;
      math = copyAST(Rule_getMath(rl));
      data->assignment[nass] = math;

      data->ass_parameter[nass] =
	(char *) calloc(strlen(AssignmentRule_getVariable(ar))+1,
			sizeof(char));
      sprintf(data->ass_parameter[nass], AssignmentRule_getVariable(ar));
      nass++;      
    }
  }

  /* setting model name and id and pointing to sbml model */    
  if ( Model_isSetName(m) ) {
    data->modelName = (char *)calloc(strlen(Model_getName(m))+1,
				     sizeof(char));
    sprintf(data->modelName, Model_getName(m));    
  }
  else {
    data->modelName = (char *)calloc(strlen(Model_getId(m))+1,
				     sizeof(char));
    sprintf(data->modelName, Model_getId(m));
  }
  data->modelId = (char *)calloc(strlen(Model_getId(m))+1,
				 sizeof(char));
  sprintf(data->modelId, Model_getId(m));

  data->m = m;
  data->simple = ode; 
  data->simplified = simplify ;

  if ( simplify ) {
    ODEs_constructJacobian(data, determinant);
  }
  else {
    SolverError_error(
        WARNING_ERROR_TYPE,
        SOLVER_ERROR_MODEL_NOT_SIMPLIFIED,
        "Model not simplified; Jacobian matrix construction skipped");
    for ( i=0; i<data->neq; i++ ) {
      safe_free(data->jacob[i]);
    }
    safe_free(data->jacob);
    data->jacob = NULL;
  }

  return data;
}

SBML_ODESOLVER_API odeModel_t *
ODEModel_create(char *sbmlFileName)
{
    SBMLDocument_t *d;
    Model_t *m;
    odeModel_t *om;

    d =
        parseModelPassingOptions(
            sbmlFileName,
            1 /* print message */,
            0 /* don't validate */,
            0, 0, 0, 0 /* empty validation parameters */);

    RETURN_ON_ERRORS_WITH(NULL);

    m = SBMLDocument_getModel(d);

    /* At first attempt to construct a simplified SBML model,
    that only consists of species and their ODEs, represented
    as Rate Rules, and of Events. All constant species, parameters
    compartments, assignment rules and function definitions
    will be replaced by their values or expressions respectively
    in all remaining formulas (ie. rate and algebraic rules and
    events).
    Then the initial values and ODEs of the remaining species
    will be written to the structure odeModel.
    */
    om = ODEModel_createFromModel(m);

    /** Errors will cause the program to stop,
    e.g. when some mathematical expressions are missing.
    */
    RETURN_ON_ERRORS_WITH(NULL);

    return om;
}

SBML_ODESOLVER_API void ODEModel_free(odeModel_t *data)
{

  int i,j;

  if(data == NULL){
    return;
  }

  /* free model name and id */
  safe_free(data->modelName);
  safe_free(data->modelId);

  /* free ODEs */
  for ( i=0; i<data->neq; i++ ) {
    safe_free(data->species[i]);
    safe_free(data->speciesname[i]);
    ASTNode_free(data->ode[i]);
  }
  safe_free(data->species);
  safe_free(data->speciesname);
  safe_free(data->ode);

  /* free Jacobian matrix */
  if ( data->jacob != NULL ) {
    for ( i=0; i<data->neq; i++ ) {
      for ( j=0; j<data->neq; j++ ) {
	ASTNode_free(data->jacob[i][j]);
      }
      safe_free(data->jacob[i]);
    }
    safe_free(data->jacob);
  }

  /* free determinant of Jacobian matrix */
  if ( data->det != NULL ) {
    ASTNode_free(data->det);
  }

  /* free assignments */
  for ( i=0; i<data->nass; i++ ) {
    safe_free(data->ass_parameter[i]);
    ASTNode_free(data->assignment[i]);
  }  
  safe_free(data->ass_parameter);
   safe_free(data->assignment);

  /* free constants */
  for ( i=0; i<data->nconst; i++ ) {
    safe_free(data->parameter[i]);
  }  
  safe_free(data->parameter);

  /* free simplified ODE model */
  if ( data->simple != NULL ) {
    Model_free(data->simple);
  }

  /* free model structure */
  safe_free(data);
}       

SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndex(odeModel_t *data, char *symbol)
{
    int i;
    variableIndex_t *vi = malloc(sizeof(variableIndex_t));
    
    if (!vi)
        return 0;

    for ( i=0; i<data->neq && strcmp(symbol, data->speciesname[i]); i++ );
    
    if (i<data->neq)
    {
        vi->type = SPECIES ;
        vi->index = i ;
        return vi;
    }

    for ( i=0; i<data->nass && strcmp(symbol, data->ass_parameter[i]); i++ );
    
    if (i<data->nass)
    {
        vi->type = ASSIGNMENT_PARAMETER ;
        vi->index = i ;
        return vi;
   }

    for ( i=0; i<data->nconst && strcmp(symbol, data->parameter[i]); i++ );
    
    if (i<data->nconst)
    {
        vi->type = PARAMETER ;
        vi->index = i ;
        return vi;
    }

    VariableIndex_free(vi);

    return 0;
}

SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *vi)
{
    free(vi);
}