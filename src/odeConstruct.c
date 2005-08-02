/*
  Last changed Time-stamp: <2005-08-02 02:32:20 raim>
  $Id: odeConstruct.c,v 1.8 2005/08/02 13:20:28 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/util.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/solverError.h"

static void
ODE_replaceFunctionDefinitions(Model_t *m);
static Model_t *
Model_copyInits(Model_t *old);

/** C: Create an ODE system
    of the reaction network of the passed models
    and construct a reduced SBML model, only
    consisting of ODEs.    
*/

Model_t*
Model_reduceToOdes(Model_t *m) {

  Model_t *ode;
  Parameter_t *p;
  Species_t *s;
  Rule_t *rl;
  RateRule_t *rr, *rl_new;
  AssignmentRule_t *ar, *ar_new;
  AlgebraicRule_t *alr, *alr_new;
  Event_t *e, *e_new;
  EventAssignment_t *ea, *ea_new;
  SBMLTypeCode_t type;
  ASTNode_t *math;
  
  int i, j, errors, found;

  errors = 0;
  found  = 0;
  s      = NULL;
  p      = NULL;
  math   = NULL;

  /** Initialize a new model:
  */
  
  ode = Model_copyInits(m);
  
  /** Steps C.2 and C.3 will identify all ODEs and
      add them as Rate Rules and their initial conditions
      as Species to the new simplified model
  */

  /** C.2: Copy predefined ODEs
      copy rate rules to new model,
  */

  for ( j=0; j<Model_getNumRules(m); j++ ) {
    rl = Model_getRule(m,j);
    type = SBase_getTypeCode((SBase_t *)rl);
    if ( type == SBML_RATE_RULE ) {
      rr = (RateRule_t *)rl;
      if ( Rule_isSetMath(rl) && RateRule_isSetVariable(rr) ) {
	math = copyAST(Rule_getMath(rl));
	rl_new = RateRule_create();
	RateRule_setVariable(rl_new, RateRule_getVariable(rr));
	Rule_setMath((Rule_t *)rl_new, math);
	Model_addRule(ode, (Rule_t *)rl_new);
      }
    }
  }

  
  /** C.3: Construct ODEs from Reactions
      construct ODEs for non-constant and non-boundary
      species from reactions, if they have not already
      been set by a rate or an assignment rule in the
      old model. Local parameters of the kinetic laws
      are replaced on the fly for each reaction.
      Rate rules have been added to new model in C.2
      and assignment rule will be handled later in step
      C.5.
  */
  /* The species vector should later be returned by a
     function that finds mass-conservation relations and reduces the
     number of independent variables by matrix operations as developed
     in Metabolic Control Analysis. (eg. Reder 1988, Sauro 2003). */
  
  for ( i=0; i<Model_getNumSpecies(m); i++ ) {
    s = Model_getSpecies(m, i);
   
    found = 0;
    for ( j=0; j<Model_getNumRules(m); j++ ) {
      rl = Model_getRule(m, j);
      type = SBase_getTypeCode((SBase_t *)rl);
      if ( type == SBML_RATE_RULE ) {
	rr = (RateRule_t *)rl;
	if ( strcmp(Species_getId(s), RateRule_getVariable(rr)) == 0 ) {
	  found = 1;
	}
      }
      else if ( type == SBML_ASSIGNMENT_RULE ) {
	ar = (AssignmentRule_t *)rl;
	if ( strcmp(Species_getId(s),
		    AssignmentRule_getVariable(ar)) == 0 ) {
	  found = 1;
	}
      }
    }
    if ( found == 0 ) {
      if ( !Species_getConstant(s) && !Species_getBoundaryCondition(s) ) {

	math = Species_odeFromReactions(s, m);

	if ( math == NULL ) {
	  errors++;
	  SolverError_error(
			    ERROR_ERROR_TYPE,
		    SOLVER_ERROR_ODE_COULD_NOT_BE_CONSTRUCTED_FOR_SPECIES,
			    "ODE could not be constructed for species %s",
			    Species_getId(s));
	}
	else {
	  rl_new = RateRule_create();
	  RateRule_setVariable(rl_new, Species_getId(s));
	  Rule_setMath((Rule_t *)rl_new, math);
	  Model_addRule(ode, (Rule_t *)rl_new);
	}
      }
    }
  }

  /** C.4: Copy incompatible  SBML structures
      The next steps will copy remaining definitions that
      can't be simplified, 
      i.e. expressed in a system of ODEs to the new model.
      They will also print warnings that these definitions
      cannot be interpreted correctly by the current state
      of the SBML_odeSolver.
      Additionally all assignment rules are copied, only for printing
      out results.
   */

  /** C.4a: Copy Events
      copy events to new model and print warning
  */

  for ( i=0; i<Model_getNumEvents(m); i++ ) {
    e = Model_getEvent(m, i);
    e_new = Event_create();
    if ( Event_isSetTrigger(e) ) {
      Event_setTrigger(e_new, copyAST(Event_getTrigger(e)));
    }
    if ( Event_isSetId(e) ) {
      Event_setId(e_new, Event_getId(e));
    }    
    if ( Event_isSetTimeUnits(e) ) {
      Event_setTimeUnits(e_new, Event_getTimeUnits(e));
    }
    if ( Event_isSetName(e) ) {
      Event_setName(e_new, Event_getName(e));
    }
    if ( Event_isSetDelay(e) ) {
      Event_setDelay(e_new, copyAST(Event_getDelay(e)));
    }
    for ( j=0; j<Event_getNumEventAssignments(e); j++ ) {
      ea = Event_getEventAssignment(e, j);
      ea_new = EventAssignment_create();
      if ( EventAssignment_isSetVariable(ea) ) {
	EventAssignment_setVariable(ea_new, EventAssignment_getVariable(ea));
      }
      if ( EventAssignment_isSetMath(ea) ) {
	EventAssignment_setMath(ea_new, copyAST(EventAssignment_getMath(ea)));
      }
      Event_addEventAssignment(e_new, ea_new);
    }
    Model_addEvent(ode, e_new);
    if (!i)
        SolverError_error(
            WARNING_ERROR_TYPE,
            SOLVER_ERROR_THE_MODEL_CONTAINS_EVENTS,
            "The model contains events. "
            "The SBML_odeSolver implementation of events is not fully "
	    "SBML conformant. "
            "Results will depend on the simulation duration and the "
	    "number of output steps.");
  }
  
  /** C.4.b: Copy Algebraic Rules
      copy algebraic rules to new model and print warning

  */
  
  for ( j=i=0; i<Model_getNumRules(m); i++ ) {
    rl = Model_getRule(m, i);
    type = SBase_getTypeCode((SBase_t *)rl);
    if ( type == SBML_ALGEBRAIC_RULE ) {
      alr = (AlgebraicRule_t *)rl;
      if ( Rule_isSetMath(rl) ) {
	math = copyAST(Rule_getMath(rl));
	alr_new = AlgebraicRule_create();
	Rule_setMath((Rule_t *)alr_new, math);
	Model_addRule(ode, (Rule_t *)alr_new);
	errors++;
	if ( !j ) {
	  SolverError_error(ERROR_ERROR_TYPE,
			    SOLVER_ERROR_THE_MODEL_CONTAINS_ALGEBRAIC_RULES,
			    "The model contains Algebraic Rules. "
			    "SBML_odeSolver is unable to solve "
			    "models of this type.");
	}
	j++;
      }
    }
  }

  /** Step C.4.c: Copy Assignment Rules
  */
  
  for ( i=0; i<Model_getNumRules(m); i++ ) {

    rl = Model_getRule(m,i);
    type = SBase_getTypeCode((SBase_t *)rl);

    if ( type == SBML_ASSIGNMENT_RULE ) {
      ar = (AssignmentRule_t *)rl;
      if ( Rule_isSetMath(rl) && AssignmentRule_isSetVariable(ar) ) {

	math = copyAST(Rule_getMath(rl));
	ar_new = AssignmentRule_create();
	AssignmentRule_setVariable(ar_new, AssignmentRule_getVariable(ar));
	Rule_setMath((Rule_t *)ar_new, math);
	Model_addRule(ode, (Rule_t *)ar_new);
      }
    }
  }
    
  /** C.5: replace function definitions in all formulas
  */
  ODE_replaceFunctionDefinitions(ode);
  
  if ( errors>0 ) {
    SolverError_error(
        ERROR_ERROR_TYPE,
        SOLVER_ERROR_ODE_MODEL_COULD_NOT_BE_CONSTRUCTED,
        "ODE model could not be constructed");
    return NULL;
  }
  else {
    return ode;
  }
}
static Model_t *
Model_copyInits(Model_t *old)
{

  int i;
  Model_t *new;
  Compartment_t *c, *c_new;
  Parameter_t *p, *p_new;
  Species_t *s, *s_new;
  FunctionDefinition_t *f, *f_new;
  ASTNode_t *math;

  new = Model_create();
  
  if ( Model_isSetId(old) )
    Model_setId(new, Model_getId(old));
  if ( Model_isSetName(old) )
    Model_setId(new, Model_getName(old));

  for ( i=0; i<Model_getNumCompartments(old); i++) {
    c = Model_getCompartment(old, i);
    c_new = Compartment_createWith(Compartment_getId(c),
				   Compartment_getSize(c),
				   Compartment_getUnits(c),
				   Compartment_getOutside(c));
    Compartment_setConstant(c_new, Compartment_getConstant(c));
    Compartment_setSpatialDimensions(c_new,
				     Compartment_getSpatialDimensions(c));
    if ( Compartment_isSetName(c) )
      Compartment_setName(c_new, Compartment_getName(c));
    Model_addCompartment(new, c_new);
  }
  
  for ( i=0; i<Model_getNumParameters(old); i++) {
    p = Model_getParameter(old, i);
    p_new = Parameter_createWith(Parameter_getId(p),
				 Parameter_getValue(p),
				 Parameter_getUnits(p));
    Parameter_setConstant(p_new, Parameter_getConstant(p));

    if ( Parameter_isSetName(p) )
      Parameter_setName(p_new, Parameter_getName(p));
    Model_addParameter(new, p_new);
  }
  
  for ( i=0; i<Model_getNumSpecies(old); i++) {
    s = Model_getSpecies(old, i);
    s_new = Species_createWith(Species_getId(s),
			       Species_getCompartment(s),
			       0.0,
			       Species_getSubstanceUnits(s),
			       Species_getBoundaryCondition(s),
			       Species_getCharge(s));
    Species_setConstant(s_new, Species_getConstant(s));
    Species_setHasOnlySubstanceUnits(s_new,
				     Species_getHasOnlySubstanceUnits(s));
    Species_setSpatialSizeUnits(s_new,
				Species_getSpatialSizeUnits(s));
			
    if ( Species_isSetInitialConcentration(s) ) {
      Species_setInitialConcentration(s_new,
				      Species_getInitialConcentration(s));
    }
    else {
      c = Model_getCompartmentById(old, Species_getCompartment(s));
      Species_setInitialConcentration(s_new,
				      Species_getInitialAmount(s)/
				      Compartment_getSize(c));
    }
    if ( Species_isSetName(s) ) {
      Species_setName(s, Species_getName(s));
    }
    Model_addSpecies(new, s_new);
  }
  /** Function Definitions
  */
  for ( i=0; i<Model_getNumFunctionDefinitions(old); i++ ) {
    f = Model_getFunctionDefinition(old, i);
    f_new = FunctionDefinition_create();
    FunctionDefinition_setId(f_new, FunctionDefinition_getId(f));
    math = copyAST(FunctionDefinition_getMath(f));
    FunctionDefinition_setMath(f_new, math);
    Model_addFunctionDefinition(new, f_new);
  }

  return(new);
}


/***/

double
Model_getValueById(Model_t *m, const char *id) {

  Species_t *s;
  Parameter_t *p;
  Compartment_t *c;

  if ( (p = Model_getParameterById(m, id)) !=NULL ) {
    if ( Parameter_isSetValue(p) ) {
      return Parameter_getValue(p);
    }
  }
  if ( (c = Model_getCompartmentById(m, id)) !=NULL ) {
    if ( Compartment_isSetSize(c) ) {
      return Compartment_getSize(c);
    }
  }

  if ( (s = Model_getSpeciesById(m, id)) !=NULL ) {
    if ( Species_isSetInitialConcentration(s) ) {
      return Species_getInitialConcentration(s);
    }
    else if ( Species_isSetInitialAmount(s) ) {
      c = Model_getCompartmentById(m, Species_getCompartment(s));
      return Species_getInitialAmount(s) / Compartment_getSize(c);
    }
  }
  Warn(stderr, "Value for \"%s\" not found!", id);
  Warn(stderr, "Defaults to 0. Please check model!");
  return (0.0);
}


/** Step C: Function Definition Replacement
    replaces all occurences of a user defined function
    by their function definition
*/
static void
ODE_replaceFunctionDefinitions(Model_t *m) {


  int i, j, k;
  Rule_t *rl_new;
  FunctionDefinition_t *f;
  Event_t *e;
  EventAssignment_t *ea;
  ASTNode_t *math;
  
 
  /** Step C.2: replace Function Definitions
      All Function Definitions will be replaced
      by the full expression
      in ODEs (rate rules), Algebraic Rules and Events
      of the ode model.
  */
  
  for ( i=0; i<Model_getNumFunctionDefinitions(m); i++ ) {
    f = Model_getFunctionDefinition(m, i);
    /*
      replacing functions in
      ODEs (rate rules) and algebraic rules
      of the ode model
    */
    for ( j=0; j<Model_getNumRules(m); j++ ) {
      rl_new = Model_getRule(m, j);
      math = copyAST(Rule_getMath(rl_new));
      AST_replaceFunctionDefinition(math,
				    FunctionDefinition_getId(f),
				    FunctionDefinition_getMath(f));
      Rule_setMath(rl_new, math);
    }
    /*
      replacing functions in all events
      and event assignments of the ode model
    */	
    for ( j=0; j<Model_getNumEvents(m); j++ ) {
      e = Model_getEvent(m, j);
      for ( k=0; k<Event_getNumEventAssignments(e); k++ ) {
	ea = Event_getEventAssignment(e, k);
	math = copyAST(EventAssignment_getMath(ea));
	AST_replaceFunctionDefinition(math,
				    FunctionDefinition_getId(f),
				    FunctionDefinition_getMath(f));
	EventAssignment_setMath(ea, math);
      }
      
      math = copyAST(Event_getTrigger(e));
      AST_replaceFunctionDefinition(math,
				    FunctionDefinition_getId(f),
				    FunctionDefinition_getMath(f));
      Event_setTrigger(e, math);
    }
  }
}


/**
   This function takes a species and a model, constructs and ODE for
   that species from all the reaction it appears in as either reactant
   or modifier. It directly constructs an Abstract Syntax Tree (AST) and
   returns a pointer to it.
*/
ASTNode_t *
Species_odeFromReactions(Species_t *s, Model_t *m){

  int j, k, errors;
  Reaction_t *r;
  SpeciesReference_t *sref;
  KineticLaw_t *kl;
  Compartment_t *c;
  ASTNode_t *simple, *ode, *tmp, *reactant;

  errors = 0;
  ode = NULL;

  /* search for the species in all reactions, and
     add up the kinetic laws * stoichiometry for
     all consuming and producing reactions to
     an ODE */

  for ( j=0; j<Model_getNumReactions(m); j++ ) {
      r = Model_getReaction(m,j);
      if ( Reaction_isSetKineticLaw(r) ) {
          kl = Reaction_getKineticLaw(r);
      }
      else
          kl = NULL;

      for ( k=0; k<Reaction_getNumReactants(r); k++ ) {
          sref = Reaction_getReactant(r,k);
          if ( strcmp(SpeciesReference_getSpecies(sref),
		      Species_getId(s)) == 0 ) {
              if ( kl != NULL ) {

                  /** Construct expression for reactant
                  by multiplying the kinetic law
                  with stoichiometry (math) and putting
                  a minus in front of it
                  */
                  if ( SpeciesReference_isSetStoichiometryMath(sref) ) {
                      reactant = ASTNode_create();
                      ASTNode_setCharacter(reactant, '*');
                      ASTNode_addChild(reactant,
                          copyAST( \
                          SpeciesReference_getStoichiometryMath(sref)));
                      ASTNode_addChild(reactant,
				       copyAST( KineticLaw_getMath(kl)));
                  }
                  else {
                      if ( SpeciesReference_getStoichiometry(sref) == 1. ) {
                          reactant = copyAST(KineticLaw_getMath(kl));
                      }
                      else {
                          reactant = ASTNode_create();
                          ASTNode_setCharacter(reactant, '*');
                          ASTNode_addChild(reactant, ASTNode_create());
                          ASTNode_setReal(ASTNode_getChild(reactant,0), 
                              SpeciesReference_getStoichiometry(sref));
                          ASTNode_addChild(reactant,
					   copyAST(KineticLaw_getMath(kl)));
                      }
                  }

                  /* replace local parameters by their value,
		     before adding to ODE */
                  AST_replaceNameByParameters(reactant,
                      KineticLaw_getListOfParameters(kl));
                  /** Add reactant expression to ODE
                  */
                  if ( ode == NULL ) {
                      ode = ASTNode_create();
                      ASTNode_setCharacter(ode,'-');
                      ASTNode_addChild(ode, reactant);
                  }
                  else {
                      tmp = copyAST(ode);
                      ASTNode_free(ode);
                      ode = ASTNode_create();
                      ASTNode_setCharacter(ode, '-');
                      ASTNode_addChild(ode, tmp);
                      ASTNode_addChild(ode, reactant);
                  }

              }
              else {
                  SolverError_error(
                      ERROR_ERROR_TYPE,
                      SOLVER_ERROR_NO_KINETIC_LAW_FOUND_FOR_REACTION,
                      "The model has no kinetic law for reaction %s",
                      Reaction_getId(r));
                  ++errors;
              }
          }
      }

      for ( k=0; k<Reaction_getNumProducts(r); k++ ) {
          sref = Reaction_getProduct(r,k);
          if ( strcmp(SpeciesReference_getSpecies(sref),
		      Species_getId(s)) == 0 ) {
              if ( kl != NULL ) {

                  reactant = ASTNode_create();
                  ASTNode_setCharacter(reactant, '*');

                  if ( SpeciesReference_isSetStoichiometryMath(sref) ) {
                      ASTNode_addChild(reactant,
                          copyAST( \
                          SpeciesReference_getStoichiometryMath(sref)));
                  }
                  else {
                      ASTNode_addChild(reactant, ASTNode_create());
                      ASTNode_setReal(ASTNode_getChild(reactant,0),
                          SpeciesReference_getStoichiometry(sref));
                  }
                  ASTNode_addChild(reactant, copyAST(KineticLaw_getMath(kl)));

                  /* replace local parameters by their value,
		     before adding to ODE */
                  AST_replaceNameByParameters(reactant,
                      KineticLaw_getListOfParameters(kl));
                  /** Add reactant expression to ODE
                  */
                  if ( ode == NULL ) {
                      ode = reactant;
                  }
                  else {
                      tmp = copyAST(ode);
                      ASTNode_free(ode);
                      ode = ASTNode_create();
                      ASTNode_setCharacter(ode, '+');
                      ASTNode_addChild(ode, tmp);
                      ASTNode_addChild(ode, reactant);
                  }	  

              }
              else {
                  SolverError_error(
                      ERROR_ERROR_TYPE,
                      SOLVER_ERROR_NO_KINETIC_LAW_FOUND_FOR_REACTION,
                      "The model has no kinetic law for reaction %s",
                      Reaction_getId(r));
                  ++errors;
              }
          }
      }
  }

  /* Divide ODE by Name of the species' compartment, if the
  compartment is set variable, by the size if the compartment is
  constant (but not if Volume is constant AND 1),

  If formula is empty skip division by compartment and set formula
  to 0.  The latter case can happen, if a species is neither
  constant nor a boundary condition but appears only as a modifier
  in reactions.  The rate for such species is set to 0. */

  if( ode != NULL ) {
      for ( j=0; j<Model_getNumCompartments(m); j++ ) {
          c = Model_getCompartment(m,j);
          if ( strcmp(Compartment_getId(c), Species_getCompartment(s)) == 0 ) {
              if ( Compartment_getConstant(c) ) {
                  if ( Compartment_getSize(c) != 1 ) {
                      tmp = copyAST(ode);
                      ASTNode_free(ode);
                      ode = ASTNode_create();
                      ASTNode_setCharacter(ode, '/');
                      ASTNode_addChild(ode, tmp);
                      ASTNode_addChild(ode, ASTNode_create());
                      ASTNode_setReal(ASTNode_getChild(ode,1),
				      Compartment_getSize(c));
                  }
              }
              else if ( !(Compartment_getConstant(c)) ) {
                  tmp = copyAST(ode);
                  ASTNode_free(ode);
                  ode = ASTNode_create();
                  ASTNode_setCharacter(ode, '/');
                  ASTNode_addChild(ode, tmp);
                  ASTNode_addChild(ode, ASTNode_create());
                  ASTNode_setName(ASTNode_getChild(ode,1),
				  Compartment_getId(c));
              }
          }
      }	 
  }
  else {
      /*
      for modifier species that never appear as products or reactants
      but are not defined as constant or boundarySpecies, set ODE to 0.
      */
      ode = ASTNode_create();
      ASTNode_setInteger(ode, 0);
  }

  simple = AST_simplify(ode);
  ASTNode_free(ode);

  if ( errors>0 ) {
      ASTNode_free(simple);
      return NULL;
  }
  else {
      return simple;
  }
}

/**
   Once an ODE system have been constructed from an SBML model, this
   function calculates the derivative of each species' ODE with respect
   to all other species for which an ODE exists. Ie. it constructs the
   Jacobian matrix.
*/

void
ODEs_constructJacobian(odeModel_t *model) {
  
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


/* End of file */
