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

static odeModel_t *ODEModel_fillStructures(Model_t *ode, int jacobian);
static odeModel_t *ODEModel_allocate(int neq, int nconst,
				    int nass, int nevents);


/** I.1: Create internal model odeModel_t from an SBML representation
    of a reaction network:   

    The function at first, attempts to construct a simplified SBML
    model, that contains all compartments, species, parameters, events
    and rules of the input model, and constructs new ODEs as SBML
    RateRules from the reaction network of the input model.  The
    function then creates the structure odeModel_t which contains
    variable, parameter and constant names and all formulas (ODEs and
    assignments) as indexed AST (iAST). This structure can be used to
    initialize and run several integration runs, each associated with
    initial conditions in cvodeData_t. Alternatively I.1a - I.1c
    allow to construct odeModel_t from higher-level data (a file, an
    SBML document or a reaction network model, respectively).
*/


/** I.1c: Create internal model odeModel_t  
    see I.1 for details
*/

SBML_ODESOLVER_API odeModel_t *
ODEModel_create(Model_t *m, int jacobian)
{
  Model_t *ode;
  odeModel_t *om;
 
  ode = Model_reduceToOdes(m);
  RETURN_ON_ERRORS_WITH(NULL);

  om = ODEModel_fillStructures(ode, jacobian);
  /* Errors will cause the program to stop, e.g. when some
      mathematical expressions are missing.  */
  RETURN_ON_ERRORS_WITH(NULL);
  
  om->m = m;
    
  return om;
}


/* allocates memory for substructures of a new odeModel, writes
   variable and parameter names and returns a pointer to the
   the newly created odeModel. */
static odeModel_t *
ODEModel_fillStructures(Model_t *ode, int jacobian)
{
  int i, j, found, neq, nconst, nass, nevents, nvalues;
  Compartment_t *c;
  Parameter_t *p;
  Species_t *s;
  Rule_t *rl;
  AssignmentRule_t *ar;
  RateRule_t *rr;
  SBMLTypeCode_t type;  
  ASTNode_t *math;  
  odeModel_t *om;

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
  
  om = ODEModel_allocate(neq, nconst, nass, nevents);

  RETURN_ON_FATALS_WITH(NULL);
  
  om->neq = neq;
  om->nconst = nconst;
  om->nass = nass;

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
      ASSIGN_NEW_MEMORY_BLOCK(om->names[neq],
			      strlen(RateRule_getVariable(rr))+1, char, NULL)
      sprintf(om->names[neq],RateRule_getVariable(rr));
      neq++;      
    }
    else if ( type == SBML_ASSIGNMENT_RULE ) {
      
      ar = (AssignmentRule_t *)rl;
      ASSIGN_NEW_MEMORY_BLOCK(om->names[om->neq+nass],
			      strlen(AssignmentRule_getVariable(ar))+1,
			      char, NULL);
      sprintf(om->names[om->neq+nass], AssignmentRule_getVariable(ar));
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
      if ( strcmp(Compartment_getId(c), om->names[j]) == 0 ) {
	found ++;
      }
    }
    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(om->names[neq+nass+nconst],
			      strlen(Compartment_getId(c))+1, char, NULL);
      sprintf(om->names[neq+nass+nconst], Compartment_getId(c));
      nconst++;      
    }
  }  
  for ( i=0; i<Model_getNumSpecies(ode); i++ ) {
    found = 0;
    s = Model_getSpecies(ode, i);
    for ( j=0; j<neq+nass; j++ ) {
      if ( strcmp(Species_getId(s), om->names[j]) == 0 ) {
	found ++;
      }
    }
    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(om->names[neq+nass+nconst],
			      strlen(Species_getId(s))+1, char, NULL);
      sprintf(om->names[neq+nass+nconst], Species_getId(s));
      nconst++;      
    }
  }
  for ( i=0; i<Model_getNumParameters(ode); i++ ) {
    found = 0;
    p = Model_getParameter(ode, i);
    for ( j=0; j<neq+nass; j++ ) {
      if ( strcmp(Parameter_getId(p), om->names[j]) == 0 ) {
	found ++;
      }
    }
    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(om->names[neq+nass+nconst],
			      strlen(Parameter_getId(p))+1, char, NULL);
      sprintf(om->names[neq+nass+nconst], Parameter_getId(p));
      nconst++;      
    }
  }

  /** Writing and Indexing Formulas:
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
      math = indexAST(Rule_getMath(rl), nvalues, om->names);
      om->ode[neq] = math; 
      neq++;      
    }
    else if ( type == SBML_ASSIGNMENT_RULE ) {
      ar = (AssignmentRule_t *)rl;
      math = indexAST(Rule_getMath(rl), nvalues, om->names); 
      om->assignment[nass] = math;
      nass++;      
    }
  }  
  

  om->simple = ode; 
  om->jacobian = jacobian;

  if ( jacobian ) {
    ODEModel_constructJacobian(om);
  }
  else {
    SolverError_error(
        WARNING_ERROR_TYPE,
        SOLVER_ERROR_MODEL_NOT_SIMPLIFIED,
        "Jacobian matrix construction skipped.");
    for ( i=0; i<om->neq; i++ ) {
      free(om->jacob[i]);
    }
    free(om->jacob);
    om->jacob = NULL;
  }

  return om;
}

/* allocates memory for a new odeModel structure and returns
   a pointer to it */ 
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


/** I.1a: Create internal model odeModel_t from file
    see I.1 for details
*/

SBML_ODESOLVER_API odeModel_t *
ODEModel_createFromFile(char *sbmlFileName, int jacobian)
{
    SBMLDocument_t *d;
    odeModel_t *om;

    d =  parseModel(sbmlFileName,
		    1 /* print message */,
		    0 /* don't validate */,
		    0, 0, 0, 0 /* empty validation parameters */);
    
    RETURN_ON_ERRORS_WITH(NULL);
    
    om = ODEModel_createFromSBML2(d, jacobian);
    /* Errors will cause the program to stop, e.g. when some
    mathematical expressions are missing. */
    RETURN_ON_ERRORS_WITH(NULL);

    return om;
}

/** I.1b: Create internal model odeModel_t from SBMLDocument
    see I.1 for details
*/
SBML_ODESOLVER_API odeModel_t *
ODEModel_createFromSBML2(SBMLDocument_t *d, int jacobian)
{
  Model_t *m;
  odeModel_t *om;

  if ( SBMLDocument_getLevel(d) == 1 ) {
    SolverError_error(ERROR_ERROR_TYPE,
         SOLVER_ERROR_DOCUMENTLEVEL_ONE,
	 "SBML Level %d cannot be processed \n", SBMLDocument_getLevel(d));
    RETURN_ON_ERRORS_WITH(NULL);
  }
 
  
  m = SBMLDocument_getModel(d);
  
  om = ODEModel_create(m, jacobian);
  /* Errors will cause the program to stop, e.g. when some
      mathematical expressions are missing.  */
  RETURN_ON_ERRORS_WITH(NULL);
  
  return om;
}


/** I.1.1: Construct Jacobian Matrix for ODEModel
   Once an ODE system has been constructed from an SBML model, this
   function calculates the derivative of each species' ODE with respect
   to all other species for which an ODE exists, i.e. it constructs the
   jacobian matrix of the ODE system.
*/
void
ODEModel_constructJacobian(odeModel_t *model) {
  
  int i, j, k, failed, nvalues;
  ASTNode_t *fprime, *simple, *index;
  List_t *names;

  /* Calculate Jacobian */

  failed = 0;
  nvalues = model->neq + model->nass + model->nconst;
  
  if ( model != NULL ) {
    for ( i=0; i<model->neq; i++ ) {
      for ( j=0; j<model->neq; j++ ) {
	fprime = differentiateAST(model->ode[i], model->names[j]);
	simple =  AST_simplify(fprime);
	ASTNode_free(fprime);
	index = indexAST(simple, nvalues, model->names);
	ASTNode_free(simple);
	model->jacob[i][j] = index;
	/* check if the AST contains a failure notice */
	names = ASTNode_getListOfNodes(index ,
				       (ASTNodePredicate) ASTNode_isName);

	for ( k=0; k<List_size(names); k++ ) {
	  if ( strcmp(ASTNode_getName(List_get(names,k)),
		      "differentiation_failed") == 0 ) {
	    failed++;
	  }
	}
	List_free(names);
      }      
    }

    if ( failed != 0 ) {
       SolverError_error(
            WARNING_ERROR_TYPE,
         SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED,
	    "%d entries of the Jacobian matrix could not be constructed,\n"
            "due to failure of differentiation. Cvode will use internal\n"
            "approximation of the Jacobian instead.", failed);
      model->jacobian = 0;
      /* Opt.Jacobian = 0; */
    }
    else {
      model->jacobian = 1;
    }

  }
}


SBML_ODESOLVER_API ASTNode_t *ODEModel_constructDeterminant(odeModel_t *om)
{

  ASTNode_t ***A;

  if ( om->jacob != NULL && om->jacobian == 1 )
    return determinantNAST(om->jacob, om->neq);
  else
    return NULL; 
}

/**
   Frees the odeModel structures
*/

SBML_ODESOLVER_API void ODEModel_free(odeModel_t *om)
{

  int i,j;

  if(om == NULL){
    return;
  }


  for ( i=0; i<om->neq+om->nass+om->nconst; i++ ) {
    free(om->names[i]);
  }
  free(om->names);

  /* free ODEs */
  for ( i=0; i<om->neq; i++ ) {
    ASTNode_free(om->ode[i]);
  }
  free(om->ode);
  
  /* free assignments */
  for ( i=0; i<om->nass; i++ ) {
    ASTNode_free(om->assignment[i]);
  }  
  free(om->assignment);
  
  /* free Jacobian matrix */
  if ( om->jacob != NULL ) {
    for ( i=0; i<om->neq; i++ ) {
      for ( j=0; j<om->neq; j++ ) {
	ASTNode_free(om->jacob[i][j]);
      }
      free(om->jacob[i]);
    }
    free(om->jacob);
  }

  /* free simplified ODE model */
  if ( om->simple != NULL ) {
    Model_free(om->simple); 
  }

  /* free model structure */
  free(om);
}       


/* searches for the string "symbol" in the odeModel's names array
   and returns its index number, or -1 if it doesn't exist */
static int
ODEModel_getVariableIndexFields(odeModel_t *om, const char *symbol)
{
    int i, nvalues;

    nvalues = om->neq + om->nass + om->nconst;
    
    for ( i=0; i<nvalues && strcmp(symbol, om->names[i]); i++ );
    
    if (i<nvalues)
        return i;

    return -1;
}


/**
   Returns 1 if a variable or parameter with the SBML id
   exists in the ODEModel. 
*/

SBML_ODESOLVER_API int
ODEModel_hasVariable(odeModel_t *model, const char *symbol)
{
    return ODEModel_getVariableIndexFields(model, symbol) != -1;
}


/** Returns the total number of values in oodeModel, equivalent
    to ODEModel_getNeq + ODEModel_getNumAssignments +
    ODEModel_getNumConstants;
*/

SBML_ODESOLVER_API int ODEModel_getNumValues(odeModel_t *om)
{
  return om->neq + om->nass + om->nconst;
}


/** Returns the name of the variable corresponding to passed
    variableIndex. The returned string (const char *) may
    NOT be changed or freed by calling applications.
*/

SBML_ODESOLVER_API const char *ODEModel_getVariableName(odeModel_t *om,
							variableIndex_t *vi)
{
  return (const char*) om->names[vi->index];
}


/**
   Creates and returns a variable index for ith variable, and NULL if
   i > nvalues.  This functions works for all types of variables
   (ODE_VARIABLE, ASSIGNED_VARIABLE and CONSTANT). This variableIndex
   can be used to get and set values during an integration run with
   IntegratorInstance_getVariable and IntegratorInstance_setVariable,
   respectively. The variableIndex must be freed by the calling
   application.
*/

SBML_ODESOLVER_API variableIndex_t *
ODEModel_getVariableIndexByNum(odeModel_t *om, int i)
{
    variableIndex_t *vi = NULL;

    if ( i > ODEModel_getNumValues(om) )
    {
        VariableIndex_free(vi);
        SolverError_error(
            ERROR_ERROR_TYPE,
            SOLVER_ERROR_SYMBOL_IS_NOT_IN_MODEL,
            "No such variable is in the model");
    }
    else
      {
	ASSIGN_NEW_MEMORY(vi, variableIndex_t, NULL);
	vi->index = i;
	if ( i<om->neq )
	  vi->type = ODE_VARIABLE;
	else if ( i < om->neq + om->nass )
	  vi->type = ASSIGNMENT_VARIABLE;
	else if ( i < om->neq + om->nass )
	  vi->type = CONSTANT;
	  	  
      }

    return vi;
}


/**
   Creates and returns the variableIndex if the string "symbol" is the
   ID (corresponding to the SBML ID in the input model) of one of the
   models variables (ODE_VARIABLE, ASSIGNED_VARIABLE and CONSTANT) or
   NULL if the symbol was not found. The variableIndex must be freed
   by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *
ODEModel_getVariableIndex(odeModel_t *om, const char *symbol)
{

  int index;

  index = ODEModel_getVariableIndexFields(om, symbol);

  if (index == -1)
    {
      SolverError_error(
			ERROR_ERROR_TYPE,
			SOLVER_ERROR_SYMBOL_IS_NOT_IN_MODEL,
			"symbol %s is not in the model",
			symbol);

      return NULL;
    }

  return ODEModel_getVariableIndexByNum(om, index);
}


/** Returns the number of ODEs (number of equations) in the odeModel
*/

SBML_ODESOLVER_API int ODEModel_getNeq(odeModel_t *om)
{
  return om->neq;
}


/** Returns the ith ODE from the odeModel, where must be 0 <= i <
    ODEModel_GetNeq. The ODE is returned as an `indexed abstract
    syntax tree' (iAST), which is an extension to the usual libSBML
    AST. Every AST_NAME type node in the tree has been replaced by an
    ASTIndexNameNode, that allows a O(1) retrieval of values for this
    node from an array of all values of the odeModel.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getOde(odeModel_t *om, variableIndex_t *vi)
{
  if ( 0 < vi->index < om->neq )
    return (const ASTNode_t *) om->ode[vi->index];
  else
    return NULL;
}


/**
   Creates and returns a variable index for ith ODE variable, and NULL
   if not existing (i > ODEModel_getNeq(om)). The variableIndex must
   be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *
ODEModel_getOdeVariableIndex(odeModel_t *om, int i)
{
  if ( i < om->neq )
    return ODEModel_getVariableIndexByNum(om, i);
  else
    return NULL;
}


/** Returns the number of variable assignements in the odeModel
*/

SBML_ODESOLVER_API int ODEModel_getNumAssignments(odeModel_t *om)
{
  return om->nass;
}


/** Returns the ith assignment formula from the odeModel, where must
    be 0 <= i < ODEModel_GetNumAssignments. The ODE is returned as an
    `indexed abstract syntax tree' (iAST), which is an extension to
    the usual libSBML AST. Every AST_NAME type node in the tree has
    been replaced by an ASTIndexNameNode, that allows a O(1) retrieval
    of value for this node from an array of all values of the
    odeModel.
*/

SBML_ODESOLVER_API
const ASTNode_t *ODEModel_getAssignment(odeModel_t *om, variableIndex_t *vi)
{
  if ( om->neq <= vi->index < om->nass )
    return (const ASTNode_t *) om->assignment[vi->index - om->neq];  
  else
    return NULL;
}


/**
   Creates and returns a variable index for ith assigned variable, and
   NULL if not existing (i > ODEModel_getNumAssignedVar(om)). The
   variableIndex must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *
ODEModel_getAssignedVariableIndex(odeModel_t *om, int i)
{
  if ( i < om->nass )
    return ODEModel_getVariableIndexByNum(om, i + om->neq);
  else
    return NULL;  
}

/** Returns the number of constant parameters of the odeModel
*/

SBML_ODESOLVER_API int ODEModel_getNumConstants(odeModel_t *om)
{
  return om->nconst;
}


/**
   Creates and returns a variable index for ith constant, and NULL if
   not existing (i > ODEModel_getNumConstants(om)). The variableIndex
   must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *
ODEModel_getConstantIndex(odeModel_t *om, int i)
{
  if ( i < om->nconst )
    return ODEModel_getVariableIndexByNum(om, i + om->neq + om->nass);
  else
    return NULL;
}


/**
  Frees a variableIndex structure
*/

SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *vi)
{
    free(vi);
}


/* to be implemented */
SBML_ODESOLVER_API void ODEModel_dumpNames(odeModel_t *om);


/** Returns the SBML model that has been extracted from the input
    SBML model's reaction network and structures; contains only
    compartments, species, parameters and SBML Rules. The returned
    model is constant and must not be changed.
*/

SBML_ODESOLVER_API const Model_t *ODEModel_getModel(odeModel_t *om)
{
  return (const Model_t *) om->simple;
}


/* End of file */
