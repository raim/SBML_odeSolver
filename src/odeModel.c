#include "sbmlsolver/odeModel.h"

#include <string.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>


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

  ASSIGN_NEW_MEMORY(data, odeModel_t, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->names, neq+nass+nconst, char *, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->ode, neq, ASTNode_t *, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->assignment, nass, ASTNode_t *, NULL)
    
  ASSIGN_NEW_MEMORY_BLOCK(data->jacob, neq, ASTNode_t **, NULL)
  for ( i=0; i<neq; i++ )
    ASSIGN_NEW_MEMORY_BLOCK(data->jacob[i], neq, ASTNode_t *, NULL)

  return data ;
}

SBML_ODESOLVER_API odeModel_t *
ODEModel_createFromModel(Model_t *model)
{
  return ODEModel_createFromModelAndOptions(model, 1 /*jacobian*/);
}

SBML_ODESOLVER_API odeModel_t *
ODEModel_createFromModelAndOptions(Model_t *m, int jacobian)
{
  int i, j, found, neq, nconst, nass, nevents, nvalues;
  Model_t *ode;
  Compartment_t *c;
  Parameter_t *p;
  Species_t *s;
  Rule_t *rl;
  AssignmentRule_t *ar;
  RateRule_t *rr;
  SBMLTypeCode_t type;  
  ASTNode_t *math;  
  odeModel_t *model;

  /* C: construct ODE model */
  ode = Model_reduceToOdes(m);

  RETURN_ON_ERRORS_WITH(NULL);

  neq     = 0;
  nconst  = 0;
  nass    = 0;
  nvalues = 0;
  found   = 0;

  /*
    counting number of equations (ODEs/rateRules and assignment Rules)
    to initialize CvodeData structure. Any other occuring values are
    stored as parameters.
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

  nvalues = Model_getNumCompartments(ode) + Model_getNumSpecies(ode) +
    Model_getNumParameters(ode);

  nconst = nvalues - nass - neq;

  nevents = Model_getNumEvents(ode);
  
  model = ODEModel_allocate(neq, nconst, nass, nevents);

  RETURN_ON_FATALS_WITH(NULL);
  
  model->neq = neq;
  model->nconst = nconst;
  model->nass = nass;

  /* 
    filling the Ids of all rate rules (ODEs) and assignment rules
    the ODE model
  */

  neq  = 0;
  nass = 0;
  
  for ( j=0; j<Model_getNumRules(ode); j++ ) {
    rl = Model_getRule(ode,j);
    type = SBase_getTypeCode((SBase_t *)rl);
  
    if ( type == SBML_RATE_RULE ) {

      rr = (RateRule_t *)rl;
      ASSIGN_NEW_MEMORY_BLOCK(model->names[neq],
			      strlen(RateRule_getVariable(rr))+1, char, NULL)
      sprintf(model->names[neq],RateRule_getVariable(rr));
      neq++;      
    }
    else if ( type == SBML_ASSIGNMENT_RULE ) {
      
      ar = (AssignmentRule_t *)rl;
      ASSIGN_NEW_MEMORY_BLOCK(model->names[model->neq+nass],
			      strlen(AssignmentRule_getVariable(ar))+1,
			      char, NULL);
      sprintf(model->names[model->neq+nass], AssignmentRule_getVariable(ar));
      nass++;      
    }
  }

  /* filling constants, i.e. all values in the model, that are not
     defined by and assignment or rate rule */
  
  nconst = 0;
  for ( i=0; i<Model_getNumCompartments(ode); i++ ) {
    found = 0;
    c = Model_getCompartment(ode, i);
    for ( j=0; j<neq+nass; j++ ) {
      if ( strcmp(Compartment_getId(c), model->names[j]) == 0 ) {
	found ++;
      }
    }
    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(model->names[neq+nass+nconst],
			      strlen(Compartment_getId(c))+1, char, NULL);
      sprintf(model->names[neq+nass+nconst], Compartment_getId(c));
      nconst++;      
    }
  }  
  for ( i=0; i<Model_getNumSpecies(ode); i++ ) {
    found = 0;
    s = Model_getSpecies(ode, i);
    for ( j=0; j<neq+nass; j++ ) {
      if ( strcmp(Species_getId(s), model->names[j]) == 0 ) {
	found ++;
      }
    }
    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(model->names[neq+nass+nconst],
			      strlen(Species_getId(s))+1, char, NULL);
      sprintf(model->names[neq+nass+nconst], Species_getId(s));
      nconst++;      
    }
  }
  for ( i=0; i<Model_getNumParameters(ode); i++ ) {
    found = 0;
    p = Model_getParameter(ode, i);
    for ( j=0; j<neq+nass; j++ ) {
      if ( strcmp(Parameter_getId(p), model->names[j]) == 0 ) {
	found ++;
      }
    }
    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(model->names[neq+nass+nconst],
			      strlen(Parameter_getId(p))+1, char, NULL);
      sprintf(model->names[neq+nass+nconst], Parameter_getId(p));
      nconst++;      
    }
  }

  /** Writing Formulas:
      Indexing rate rules and assignment rules, using the string array
      created above and writing the indexed formulas to the CvodeData
      structure. These AST are used for evaluation during
      the integration routines!!
  */
  neq = 0;
  nass = 0;
  
  for ( j=0; j<Model_getNumRules(ode); j++ ) {
    rl = Model_getRule(ode, j);
    type = SBase_getTypeCode((SBase_t *)rl);
  
    if ( type == SBML_RATE_RULE ) {
      rr = (RateRule_t *)rl;
/*       math = copyAST(Rule_getMath(rl)); */
      math = indexAST(Rule_getMath(rl), nvalues, model->names);
      model->ode[neq] = math; 
      neq++;      
    }
    else if ( type == SBML_ASSIGNMENT_RULE ) {
      ar = (AssignmentRule_t *)rl;
/*       math = copyAST(Rule_getMath(rl)); */
      math = indexAST(Rule_getMath(rl), nvalues, model->names); 
      model->assignment[nass] = math;
      nass++;      
    }
  }  
  

  model->m = m;
  model->simple = ode; 
  model->jacobian = jacobian;

  if ( jacobian ) {
    ODEs_constructJacobian(model);
  }
  else {
    SolverError_error(
        WARNING_ERROR_TYPE,
        SOLVER_ERROR_MODEL_NOT_SIMPLIFIED,
        "Model not simplified; Jacobian matrix construction skipped");
    for ( i=0; i<model->neq; i++ ) {
      free(model->jacob[i]);
    }
    free(model->jacob);
    model->jacob = NULL;
  }

  return model;
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


  for ( i=0; i<data->neq+data->nass+data->nconst; i++ ) {
    free(data->names[i]);
  }
  free(data->names);

  /* free ODEs */
  for ( i=0; i<data->neq; i++ ) {
    ASTNode_free(data->ode[i]);
  }
  free(data->ode);
  
  /* free assignments */
  for ( i=0; i<data->nass; i++ ) {
    ASTNode_free(data->assignment[i]);
  }  
  free(data->assignment);
  
  /* free Jacobian matrix */
  if ( data->jacob != NULL ) {
    for ( i=0; i<data->neq; i++ ) {
      for ( j=0; j<data->neq; j++ ) {
	ASTNode_free(data->jacob[i][j]);
      }
      free(data->jacob[i]);
    }
    free(data->jacob);
  }

  /* free simplified ODE model */
  if ( data->simple != NULL ) {
    Model_free(data->simple);
  }

  /* free model structure */
  free(data);
}       

int
ODEModel_getVariableIndexFields(odeModel_t *data, const char *symbol)
{
    int i, nvalues;

    nvalues = data->neq + data->nass + data->nconst;
    
    for ( i=0; i<nvalues && strcmp(symbol, data->names[i]); i++ );
    
    if (i<nvalues)
        return i;

    return -1;
}

SBML_ODESOLVER_API int
ODEModel_hasVariable(odeModel_t *model, const char *symbol)
{
    return ODEModel_getVariableIndexFields(model, symbol) != -1;
}

SBML_ODESOLVER_API variableIndex_t *
ODEModel_getVariableIndex(odeModel_t *data, const char *symbol)
{
    variableIndex_t *vi;

    ASSIGN_NEW_MEMORY(vi, variableIndex_t, NULL);
    vi->index = ODEModel_getVariableIndexFields(data, symbol);

    if (vi->index == -1)
    {
        VariableIndex_free(vi);
        SolverError_error(
            ERROR_ERROR_TYPE,
            SOLVER_ERROR_SYMBOL_IS_NOT_IN_MODEL,
            "symbol %s is not in the model",
            symbol);

        return 0;
    }

    return vi;
}

SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *vi)
{
    free(vi);
}
