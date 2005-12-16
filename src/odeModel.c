/*
  Last changed Time-stamp: <2005-12-16 11:06:28 raim>
  $Id: odeModel.c,v 1.33 2005/12/16 15:04:44 raimc Exp $ 
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

/*! \defgroup odeModel ODE Model: f(x,p,t) = dx/dt
    \ingroup symbolic
    \brief This module contains all functions to create and interface
    the internal ODE Model it's Jacobian matrix and other derivatives
    
    The internal ODE Model (structure odeModel) can be interfaced for
    analytical purposes. All formulae can be retrieved as libSBML
    Abstract Syntax Trees (AST).
*/
/*@{*/

#include "sbmlsolver/odeModel.h"

#include <string.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>


#include "sbmlsolver/sbml.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/modelSimplify.h"

static odeModel_t *ODEModel_fillStructures(Model_t *ode);
static odeModel_t *ODEModel_allocate(int neq, int nconst,
				    int nass, int nevents, int nalg);


/** \brief Create internal model odeModel from an SBML representation
    of a reaction network

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


/** \brief Create odeModel from input SBML model
*/

SBML_ODESOLVER_API odeModel_t *ODEModel_create(Model_t *m)
{
  Model_t *ode;
  odeModel_t *om;
 
  ode = Model_reduceToOdes(m);
  RETURN_ON_ERRORS_WITH(NULL);

  om = ODEModel_fillStructures(ode);
  /* Errors will cause the program to stop, e.g. when some
      mathematical expressions are missing.  */
  RETURN_ON_ERRORS_WITH(NULL);
  
  om->m = m;
  om->d = NULL; /* will be set if created from file */
    
  return om;
}


/* allocates memory for substructures of a new odeModel, writes
   variable and parameter names and returns a pointer to the
   the newly created odeModel. */
static odeModel_t *
ODEModel_fillStructures(Model_t *ode)
{
  int i, j, found, neq, nalg, nconst, nass, nevents, nvalues;
  Compartment_t *c;
  Parameter_t *p;
  Species_t *s;
  Rule_t *rl;
  AssignmentRule_t *ar;
  AlgebraicRule_t *alr;
  RateRule_t *rr;
  SBMLTypeCode_t type;  
  ASTNode_t *math;  
  odeModel_t *om;

  neq     = 0;
  nalg    = 0;
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
    if ( type == SBML_RATE_RULE )
      neq++;
    if ( type == SBML_ASSIGNMENT_RULE )
      nass++;
    if ( type == SBML_ALGEBRAIC_RULE ) {
      nalg++;
      
    }
  }
   
  nvalues = Model_getNumCompartments(ode) + Model_getNumSpecies(ode) +
    Model_getNumParameters(ode);

  nconst = nvalues - nass - neq - nalg;

  nevents = Model_getNumEvents(ode);
  
  om = ODEModel_allocate(neq, nconst, nass, nevents, nalg);

  RETURN_ON_FATALS_WITH(NULL);
  
  om->neq = neq;
  om->nalg = nalg; /* this causes crash at the moment, because
		      ODEs have been constructed for that
		      should be defined by alg. rules */
  om->nconst = nconst;
  om->nass = nass;

  om->nsens = 0; /* sensitivity parameters can be chosen later */

  /* 
    filling the Ids of all rate rules (ODEs) and assignment rules
    the ODE model
  */

  neq  = 0;
  nass = 0;
  nalg = 0;
  
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
      sprintf(om->names[om->neq + nass], AssignmentRule_getVariable(ar));
      nass++;      
    }
    else if ( type == SBML_ALGEBRAIC_RULE ) {
      
      alr = (AlgebraicRule_t *)rl;
      /* find variables defined by algebraic rules here! */
      ASSIGN_NEW_MEMORY_BLOCK(om->names[nvalues + nalg],
			      strlen("tmp")+3,
			      char, NULL);
      sprintf(om->names[om->neq+om->nass+om->nconst+ nalg], "tmp%d", nalg);
      printf("tmp%d \n", nalg);
      nalg++;
    }
  }

  

  /* filling constants, i.e. all values in the model, that are not
     defined by an assignment or rate rule */
  
  nconst = 0;
  for ( i=0; i<Model_getNumCompartments(ode); i++ ) {
    found = 0;
    c = Model_getCompartment(ode, i);
    
    for ( j=0; j<neq+nass; j++ ) 
      if ( strcmp(Compartment_getId(c), om->names[j]) == 0 ) 
	found ++;
    
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
    
    for ( j=0; j<neq+nass; j++ ) 
      if ( strcmp(Species_getId(s), om->names[j]) == 0 ) 
	found ++;

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
    for ( j=0; j<neq+nass; j++ ) 
      if ( strcmp(Parameter_getId(p), om->names[j]) == 0 ) 
	found ++;

    if ( !found ) {
      ASSIGN_NEW_MEMORY_BLOCK(om->names[neq+nass+nconst],
			      strlen(Parameter_getId(p))+1, char, NULL);
      sprintf(om->names[neq+nass+nconst], Parameter_getId(p));
      nconst++;      
    }
  }

  /* Writing and Indexing Formulas: Indexing rate rules and assignment
     rules, using the string array created above and writing the
     indexed formulas to the CvodeData structure. These AST are used
     for evaluation during the integration routines!! */
  neq = 0;
  nass = 0;
  nalg = 0;
  
/*   ODEModel_dumpNames(om); */
/*   printf("\n\nHallo %d %d %d %d\n\n\n",
     om->neq,om->nass,om->nalg,om->nconst); */
/*   fflush(stdout); */

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
    else if ( type == SBML_ALGEBRAIC_RULE ) {
      alr = (AlgebraicRule_t *)rl;
      math = indexAST(Rule_getMath(rl), nvalues, om->names); 
      om->algebraic[nalg] = math;
      nalg++;
    }
  }  
  

  om->simple = ode;
  /* set jacobian to NULL */
  om->jacob = NULL;

  return om;
}

/* allocates memory for a new odeModel structure and returns
   a pointer to it */ 
static odeModel_t *ODEModel_allocate(int neq, int nconst,
				     int nass, int nalg, int nevents)
{
  int i;
  odeModel_t *data;

  ASSIGN_NEW_MEMORY(data, odeModel_t, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->names, neq+nalg+nass+nconst, char *, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->ode, neq, ASTNode_t *, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->assignment, nass, ASTNode_t *, NULL)
  ASSIGN_NEW_MEMORY_BLOCK(data->algebraic, nalg, ASTNode_t *, NULL)
    
  return data ;
}


/** \brief  Create internal model odeModel_t from file
*/

SBML_ODESOLVER_API odeModel_t *ODEModel_createFromFile(char *sbmlFileName)
{
    SBMLDocument_t *d;
    odeModel_t *om;

    d =  parseModel(sbmlFileName,
		    0 /* print message */,
		    0 /* don't validate */,
		    0, 0, 0, 0 /* empty validation parameters */);
    
    RETURN_ON_ERRORS_WITH(NULL);
    
    om = ODEModel_createFromSBML2(d);
    /* Errors will cause the program to stop, e.g. when some
    mathematical expressions are missing. */
    RETURN_ON_ERRORS_WITH(NULL);
    /* remember for freeing afterwards */
    om->d = d;

    return om;
}


/** \brief Create internal model odeModel_t from SBMLDocument
*/

SBML_ODESOLVER_API odeModel_t *ODEModel_createFromSBML2(SBMLDocument_t *d)
{
  Model_t *m;
  odeModel_t *om;

  if ( SBMLDocument_getLevel(d) == 1 ) {
    SolverError_error(ERROR_ERROR_TYPE,
         SOLVER_ERROR_DOCUMENTLEVEL_ONE,
	 "SBML Level %d cannot be processed", SBMLDocument_getLevel(d));
    RETURN_ON_ERRORS_WITH(NULL);
  }
 
  
  m = SBMLDocument_getModel(d);
  
  om = ODEModel_create(m);
  /* Errors will cause the program to stop, e.g. when some
      mathematical expressions are missing.  */
  RETURN_ON_ERRORS_WITH(NULL);
  
  return om;
}



/** \brief Frees the odeModel structures
*/

SBML_ODESOLVER_API void ODEModel_free(odeModel_t *om)
{

  int i,j;

  if(om == NULL)
    return;

  for ( i=0; i<om->neq+om->nass+om->nconst; i++ ) 
    free(om->names[i]);
  free(om->names);

  /* free ODEs */
  for ( i=0; i<om->neq; i++ )
    ASTNode_free(om->ode[i]);
  free(om->ode);
  
  /* free assignments */
  for ( i=0; i<om->nass; i++ ) 
    ASTNode_free(om->assignment[i]);
  free(om->assignment);
  
  /* free algebraic rules */
  for ( i=0; i<om->nalg; i++ ) 
    ASTNode_free(om->algebraic[i]);
  free(om->algebraic);
  
  /* free Jacobian matrix, if it has been constructed */
  if ( om->jacob != NULL ) 
  {
      for ( i=0; i<om->neq; i++ ) 
      {
          for ( j=0; j<om->neq; j++ ) 
	          ASTNode_free(om->jacob[i][j]);
          free(om->jacob[i]);
      }
      free(om->jacob);
  }

  ODEModel_freeSensitivity(om);

  /* free simplified ODE model */
  if ( om->simple != NULL ) 
    Model_free(om->simple); 

  /* free document, if model was constructed from file */
  if ( om->d != NULL ) 
    SBMLDocument_free(om->d);    

  /* free model structure */
  free(om);
}       


/* searches for the string "symbol" in the odeModel's names array
   and returns its index number, or -1 if it doesn't exist */
static int ODEModel_getVariableIndexFields(odeModel_t *om, const char *symbol)
{
    int i, nvalues;

    nvalues = om->neq + om->nass + om->nconst + om->nalg;
    
    for ( i=0; i<nvalues && strcmp(symbol, om->names[i]); i++ );
    
    if (i<nvalues)
        return i;

    return -1;
}


/** \brief Returns 1 if a variable or parameter with the SBML id
   exists in the ODEModel.
*/

SBML_ODESOLVER_API int ODEModel_hasVariable(odeModel_t *model, const char *symbol)
{
    return ODEModel_getVariableIndexFields(model, symbol) != -1;
}


/** \brief Returns the total number of values in oodeModel, equivalent
    to ODEModel_getNeq + ODEModel_getNumAssignments +
    ODEModel_getNumConstants;
*/

SBML_ODESOLVER_API int ODEModel_getNumValues(odeModel_t *om)
{
  return om->neq + om->nass + om->nconst + om->nalg ;
}


/** \brief Returns the name of the variable corresponding to passed
    variableIndex. The returned string (const char *) may NOT be
    changed or freed by calling applications.
*/

SBML_ODESOLVER_API const char *ODEModel_getVariableName(odeModel_t *om,
							variableIndex_t *vi)
{
  return (const char*) om->names[vi->index];
}


/** \brief Creates and returns a variable index for ith variable

   Returns NULL if i > nvalues. This functions works for all types of
   variables (ODE_VARIABLE, ASSIGNED_VARIABLE and CONSTANT). This
   variableIndex can be used to get and set values during an
   integration run with IntegratorInstance_getVariable and
   IntegratorInstance_setVariable, respectively. The variableIndex
   must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndexByNum(odeModel_t *om, int i)
{
    variableIndex_t *vi;

    if ( i > ODEModel_getNumValues(om) )
    {
        /* VariableIndex_free(vi); */
        SolverError_error(
            ERROR_ERROR_TYPE,
            SOLVER_ERROR_SYMBOL_IS_NOT_IN_MODEL,
            "No such variable in the model");
	return NULL;
	
    }
    else
      {
	ASSIGN_NEW_MEMORY(vi, variableIndex_t, NULL);
	vi->index = i;
	if ( i<om->neq ) {
	  vi->type = ODE_VARIABLE;
	  vi->type_index = vi->index;
	}
	else if ( i < om->neq + om->nass ) {
	  vi->type = ASSIGNMENT_VARIABLE;
	  vi->type_index = i - om->neq;
	}
	else if( i < om->neq + om->nass + om->nconst) {
	  vi->type = CONSTANT;
	  vi->type_index = i - om->neq - om->nass;
	}
	else {
	  vi->type = ALGEBRAIC_VARIABLE;
	  vi->type_index = i - om->neq - om->nass - om->nconst;
	}
      }
    return vi;
}


/** \brief Creates and returns the variableIndex for the string "symbol"

   where `symbol' is the ID (corresponding to the SBML ID in the input
   model) of one of the models variables (ODE_VARIABLE,
   ASSIGNED_VARIABLE and CONSTANT) or NULL if the symbol was not
   found. The variableIndex must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndex(odeModel_t *om, const char *symbol)
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


/** \brief Returns the number of ODEs (number of equations) in the
    odeModel
*/

SBML_ODESOLVER_API int ODEModel_getNeq(odeModel_t *om)
{
  return om->neq;
}


/** \brief Returns the number variables that are defined by
    an algebraic rule
*/

SBML_ODESOLVER_API int ODEModel_getNalg(odeModel_t *om)
{
  return om->nalg;
}

/** \brief Returns the number parameters for which sensitivity
    analysis might be requested.

*/

SBML_ODESOLVER_API int ODEModel_getNsens(odeModel_t *om)
{
  return om->nsens;
}


/** \brief Returns the ith ODE from the odeModel, where must be 0 <= i
    < ODEModel_GetNeq.

    The ODE is returned as an `indexed abstract syntax tree' (iAST),
    which is an extension to the usual libSBML AST. Every AST_NAME
    type node in the tree has been replaced by an ASTIndexNameNode,
    that allows a O(1) retrieval of values for this node from an array
    of all values of the odeModel.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getOde(odeModel_t *om, variableIndex_t *vi)
{
  if ( 0 < vi->index < om->neq )
    return (const ASTNode_t *) om->ode[vi->index];
  else
    return NULL;
}


/** \brief  Creates and returns a variable index for ith ODE variable.

   Returns NULL if not existing (i > ODEModel_getNeq(om)). The
   variableIndex must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *ODEModel_getOdeVariableIndex(odeModel_t *om, int i)
{
  if ( i < om->neq )
    return ODEModel_getVariableIndexByNum(om, i);
  else
    return NULL;
}


/** \brief Returns the number of variable assignements in the odeModel
*/

SBML_ODESOLVER_API int ODEModel_getNumAssignments(odeModel_t *om)
{
  return om->nass;
}


/** \brief  Returns the ith assignment formula from the odeModel, where 
     0 <= i < ODEModel_GetNumAssignments.

     The ODE is returned as an `indexed abstract syntax tree' (iAST),
     which is an extension to the usual libSBML AST. Every AST_NAME
     type node in the tree has been replaced by an ASTIndexNameNode,
     that allows a O(1) retrieval of value for this node from an array
     of all values of the odeModel.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getAssignment(odeModel_t *om, variableIndex_t *vi)
{
  if ( om->neq <= vi->index < om->nass )
    return (const ASTNode_t *) om->assignment[vi->type_index];  
  else
    return NULL;
}


/** \brief Creates and returns a variable index for ith assigned variable.

    Returns NULL if not existing (i > ODEModel_getNumAssignedVar(om)).
    The variableIndex must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *ODEModel_getAssignedVariableIndex(odeModel_t *om, int i)
{
  if ( i < om->nass )
    return ODEModel_getVariableIndexByNum(om, i + om->neq);
  else
    return NULL;  
}

/** \brief Returns the number of constant parameters of the odeModel
*/

SBML_ODESOLVER_API int ODEModel_getNumConstants(odeModel_t *om)
{
  return om->nconst;
}


/**\brief  Creates and returns a variable index for ith constant.

   Returns NULL if not existing (i > ODEModel_getNumConstants(om)).
   The variableIndex must be freed by the calling application.
*/

SBML_ODESOLVER_API variableIndex_t *ODEModel_getConstantIndex(odeModel_t *om, int i)
{
  if ( i < om->nconst )
    return ODEModel_getVariableIndexByNum(om, i + om->neq + om->nass);
  else
    return NULL;
}


/** \brief  Frees a variableIndex structure
*/

SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *vi)
{
    free(vi);
}

/** \brief Prints the names (SBML IDs) of all model variables
    and parameters
*/

SBML_ODESOLVER_API void ODEModel_dumpNames(odeModel_t *om)
{
  int i;
  for ( i=0; i<(om->neq+om->nass+om->nconst+om->nalg); i++ )
    printf("%s ", om->names[i]);
  printf("\n");
}


/** \brief Returns the SBML model that has been extracted from the input
    SBML model's reaction network and structures;

    The model contains only compartments, species, parameters and SBML
    Rules. The returned model is constant and must not be changed.
*/

SBML_ODESOLVER_API const Model_t *ODEModel_getModel(odeModel_t *om)
{
  return (const Model_t *) om->simple;
}

/** @} */


/*! \defgroup jacobian Jacobian Matrix: J = df(x)/dx
    \ingroup odeModel
    \brief Constructing and Interfacing the Jacobian Matrix of an ODE
    system

    as used for CVODES and IDA Dense Solvers
*/
/*@{*/

/** \brief Construct Jacobian Matrix for ODEModel.
    
   Once an ODE system has been constructed from an SBML model, this
   function calculates the derivative of each species' ODE with respect
   to all other species for which an ODE exists, i.e. it constructs the
   jacobian matrix of the ODE system. Returns 1 if successful, 0 otherwise. 
*/

SBML_ODESOLVER_API int ODEModel_constructJacobian(odeModel_t *om)
{  
  int i, j, k, failed, nvalues;
  ASTNode_t *fprime, *simple, *index, *ode;
  List_t *names;

  if ( om == NULL )
    return 0;
  
  /******************** Calculate Jacobian ************************/
  
  failed = 0;
  nvalues = om->neq + om->nass + om->nconst;
  
  ASSIGN_NEW_MEMORY_BLOCK(om->jacob, om->neq, ASTNode_t **, 0);
  for ( i=0; i<om->neq; i++ )
    ASSIGN_NEW_MEMORY_BLOCK(om->jacob[i], om->neq, ASTNode_t *, 0);
      
  for ( i=0; i<om->neq; i++ ) {
    ode = copyAST(om->ode[i]);
    /* assignment rule replacement: reverse to satisfy
       SBML specifications that variables defined by
       an assignment rule can appear in rules declared afterwards */
    for ( j=om->nass-1; j>=0; j-- )
      AST_replaceNameByFormula(ode, om->names[j], om->assignment[j]);
    
    for ( j=0; j<om->neq; j++ ) {
      fprime = differentiateAST(om->ode[i], om->names[j]);
      simple =  simplifyAST(fprime);
      ASTNode_free(fprime);
      index = indexAST(simple, nvalues, om->names);
      ASTNode_free(simple);
      om->jacob[i][j] = index;
      /* check if the AST contains a failure notice */
      names = ASTNode_getListOfNodes(index ,
				     (ASTNodePredicate) ASTNode_isName);

      for ( k=0; k<List_size(names); k++ ) 
	if ( strcmp(ASTNode_getName(List_get(names,k)),
		    "differentiation_failed") == 0 ) 
	  failed++;
      List_free(names);
    }
    ASTNode_free(ode);
  }
  if ( failed != 0 ) {
    SolverError_error(WARNING_ERROR_TYPE,
	  SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED,
		"%d entries of the Jacobian matrix could not be constructed, "
		"due to failure of differentiation. Cvode will use internal "
		"approximation of the Jacobian instead.", failed);
    om->jacobian = 0;
  }
  else 
    om->jacobian = 1;
    
  return om->jacobian;
}


/**  \brief Returns the ith/jth entry of the jacobian matrix
     
    Returns NULL if either the jacobian has not been constructed yet,
    or if i or j are >neq. Ownership remains within the odeModel_t
    structure.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getJacobianIJEntry(odeModel_t *om, int i, int j)
{
  if ( om->jacob == NULL )
    return NULL;
  if ( i >= om->neq || j >= om->neq )
    return NULL;  
  return (const ASTNode_t *) om->jacob[i][j];
}


/** \brief Returns the entry (d(vi1)/dt)/d(vi2) of the jacobian matrix.
    
    Returns NULL if either the jacobian has not been constructed yet,
    or if the v1 or vi2 are not ODE variables. Ownership remains
    within the odeModel_t structure.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getJacobianEntry(odeModel_t *om, variableIndex_t *vi1, variableIndex_t *vi2)
{
  return ODEModel_getJacobianIJEntry(om, vi1->index, vi2->index);
}


/** \brief Constructs and returns the determinant of the jacobian matrix.
    
    The calling application takes ownership of the returned ASTNode_t
    and must free it, if not required.
*/

SBML_ODESOLVER_API ASTNode_t *ODEModel_constructDeterminant(odeModel_t *om)
{

  ASTNode_t ***A;

  if ( om->jacob != NULL && om->jacobian == 1 )
    return determinantNAST(om->jacob, om->neq);
  else
    return NULL; 
}


/** @} */



/*! \defgroup parametric `Parametric Matrix': P = df(x)/dp
    \ingroup odeModel
    \brief Constructing and Interfacing a `Parametric Matrix' of
    an ODE system

    as used for CVODES sensitivity analysis
*/
/*@{*/


/** \brief Construct Sensitivity R.H.S. for ODEModel.
    
*/

SBML_ODESOLVER_API int ODEModel_constructSensitivity(odeModel_t *om)
{
  int i, j, k, failed, nvalues;
  ASTNode_t *ode, *fprime, *simple, *index;
  List_t *names;

  failed = 0;
  nvalues = om->neq + om->nass + om->nconst;

  om->sensitivity = 0;
  om->jacob_sens = NULL;
  om->nsens = om->nconst;

  ASSIGN_NEW_MEMORY_BLOCK(om->index_sens, om->nsens, int, 0);
  /*!!! non-default case:
    these values should be passed for other cases !!!*/
  for ( i=0; i<om->nsens; i++ )
    om->index_sens[i] = om->neq + om->nass + i;
 
  ASSIGN_NEW_MEMORY_BLOCK(om->jacob_sens, om->neq, ASTNode_t **, 0);
  for ( i=0; i<om->neq; i++ )
    ASSIGN_NEW_MEMORY_BLOCK(om->jacob_sens[i], om->nsens, ASTNode_t *, 0);

  for ( i=0; i<om->neq; i++ ) {
    ode = copyAST(om->ode[i]);
    /* assignment rule replacement: reverse to satisfy
       SBML specifications that variables defined by
       an assignment rule can appear in rules declared afterwards */
    for ( j=om->nass-1; j>=0; j-- )
      AST_replaceNameByFormula(ode, om->names[j], om->assignment[j]);
    
    for ( j=0; j<om->nsens; j++ ) {
      /* differentiate d(dYi/dt) / dPj */
      fprime = differentiateAST(ode, om->names[om->index_sens[j]]);
      simple =  simplifyAST(fprime);
      ASTNode_free(fprime);
      index = indexAST(simple, nvalues, om->names);
      ASTNode_free(simple);
      om->jacob_sens[i][j] = index;
      /* check if the AST contains a failure notice */
      names = ASTNode_getListOfNodes(index,
				     (ASTNodePredicate) ASTNode_isName);

      for ( k=0; k<List_size(names); k++ ) 
	if ( strcmp(ASTNode_getName(List_get(names,k)),
		    "differentiation_failed") == 0 ) 
	  failed++;
      List_free(names);      
    }
    ASTNode_free(ode);
  }

  if ( failed != 0 ) {
    SolverError_error(WARNING_ERROR_TYPE,
    SOLVER_ERROR_ENTRIES_OF_THE_PARAMETRIC_MATRIX_COULD_NOT_BE_CONSTRUCTED,
	      "%d entries of the parametric `Jacobian' matrix could not "
	      "be constructed, due to failure of differentiation. "
	      "Cvode will use internal approximation instead.", failed);
    om->sensitivity = 0;
  }
  else 
    om->sensitivity = 1;

  return om->sensitivity;

}


/** \brief Free Sensitivity R.H.S. for ODEModel.    
*/

SBML_ODESOLVER_API void ODEModel_freeSensitivity(odeModel_t *om)
{
  int i, j;

  /* free parameter index */
  free(om->index_sens);
  /* free parametric matrix, if it has been constructed */
  if ( om->jacob_sens != NULL )
    {
      for ( i=0; i<om->neq; i++ ) {
          for ( j=0; j<om->nsens; j++ ) 
	          ASTNode_free(om->jacob_sens[i][j]);
          free(om->jacob_sens[i]);
      }
      free(om->jacob_sens);      
  }
}


/**  \brief Returns the ith/jth entry of the parametric matrix
     
    Returns NULL if either the parametric has not been constructed yet,
    or if i > neq or j > nsens. Ownership remains within the odeModel_t
    structure.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getSensIJEntry(odeModel_t *om, int i, int j)
{
  if ( om->jacob_sens == NULL )
    return NULL;
  if ( i >= om->neq || j >= om->nsens )
    return NULL;  
  return (const ASTNode_t *) om->jacob_sens[i][j];
}


/** \brief Returns the entry (d(vi1)/dt)/d(vi2) of the parametric matrix.
    
    Returns NULL if either the parametric matrix has not been constructed
    yet, or if vi1 or vi2 are not an ODE variable or a constant for 
    sensitivity analysis, respectively. Ownership remains within the
    odeModel_t structure.
*/

SBML_ODESOLVER_API const ASTNode_t *ODEModel_getSensEntry(odeModel_t *om, variableIndex_t *vi1, variableIndex_t *vi2)
{
  /*!!! needs better solution, if sensitivity for selected params
        will be implemented !!!*/
  return ODEModel_getSensIJEntry(om, vi1->index, vi2->type_index);
}


/** \brief Returns the variableIndex for the jth parameter for
    which sensitivity analysis was requested, where
    0 < j < ODEModel_getNsens;
    
    Returns NULL if either the parametric matrix has not been constructed
    yet, or if j => ODEModel_getNsens;
*/

SBML_ODESOLVER_API variableIndex_t *ODEModel_getSensParamIndexByNum(odeModel_t *om, int j)
{
  if ( j < om->nsens )
    return ODEModel_getVariableIndexByNum(om, om->index_sens[j]);
  else
    return NULL;
}


/** @} */

/* End of file */
