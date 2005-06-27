/*
  Last changed Time-stamp: <2005-06-07 19:23:43 raim>
  $Id: modelSimplify.c,v 1.6 2005/06/27 15:12:19 afinney Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/processAST.h"

/** Replaces all parameters 'name' appearing in the formula
    'math' by the value 'x'.
*/

void
AST_replaceNameByValue(ASTNode_t *math, const char *name, double x) {

  int i;
  List_t *names;

  names = ASTNode_getListOfNodes(math,(ASTNodePredicate) ASTNode_isName);

  for ( i=0; i<List_size(names); i++ ) {
    if ( strcmp(ASTNode_getName(List_get(names,i)), name) == 0 ) {
      ASTNode_setReal(List_get(names,i), x);
    }
  }

  List_free(names);
  
}

/** Replaces all parameters appearing in the formula
    'math' by their value defined in the passed parameter
    list 'lp'.
*/

void
AST_replaceNameByParameters(ASTNode_t *math, ListOf_t *lp) {

  int i,j;
  Parameter_t *p;
  List_t *names;

  for ( i=0; i<ListOf_getNumItems(lp); i++ ) {
    p = ListOf_get(lp, i);
    names = ASTNode_getListOfNodes(math,(ASTNodePredicate) ASTNode_isName);

    for ( j=0; j<List_size(names); j++ ) {
      if ( strcmp(ASTNode_getName(List_get(names,j)),
                  Parameter_getId(p)) == 0 ) {
        if ( Parameter_getConstant(p) == 1 ) {
          ASTNode_setReal(List_get(names,j), Parameter_getValue(p));
        }
      }
    }
    List_free(names);
  }
}

/** Replaces an assigned variable 'name' by the full
    assingment in the passed mathematical expression
    math.
*/

void
AST_replaceNameByFormula(ASTNode_t *math, const char *name,
			 const ASTNode_t *formula) {

  int i, j;  
  ASTNode_t *old;
  List_t *names;

  names = ASTNode_getListOfNodes(math,(ASTNodePredicate) ASTNode_isName);

  for ( i=0; i<List_size(names); i++ ) {
    old = List_get(names,i);
    if ( strcmp(ASTNode_getName(old), name) == 0 ) {

      /*
	works just like function copyAST, see processAST.c,
	which could probably be used instead if their was
	a way to reset a node but keep its position in
	other formulas (e.g. remembering pointer value)??
	Maybe this would be the case anyways for
	ASTNode_free(old);
	old = copyAST(formula);
	That works in normal usage but causes errors and even
	a segmentation fault when run under valgrind --tool=memcheck
      */

      if ( ASTNode_isName(formula) ) {
	ASTNode_setName(old, ASTNode_getName(formula));
      }
      else if ( ASTNode_isInteger(formula) ) {
	ASTNode_setInteger(old, ASTNode_getInteger(formula));
      }
      else if ( ASTNode_isReal(formula) ) {
	ASTNode_setReal(old, ASTNode_getReal(formula));
      }  
      else {
	ASTNode_setType(old, ASTNode_getType(formula)); 
	/* a user defined function has a name that must be set */
      	if ( ASTNode_getType(formula) == AST_FUNCTION ) {
	  ASTNode_setName(old, ASTNode_getName(formula));
	}
	for ( j=0; j<ASTNode_getNumChildren(formula); j++ ) {
	  ASTNode_addChild(old, copyAST(ASTNode_getChild(formula,j)));
	}
      }
    }
  }
  List_free(names);
}

/** Replaces all user defined functions by the ful
    expression in the passed mathematical expression
    math. This is quite a dirty solution and might be
    dangerous. See comments in function.
*/

void
AST_replaceFunctionDefinition(ASTNode_t *math, const char *name,
	 const ASTNode_t *function) {
  
  int i, j;  
  ASTNode_t *old, *new;
  List_t *names;

  names = ASTNode_getListOfNodes(math,(ASTNodePredicate) ASTNode_isFunction);

  for ( i=0; i<List_size(names); i++ ) {
    new     = copyAST(ASTNode_getRightChild(function));
    old = List_get(names,i);
    /* if `old' is the searched function defintion ... */
    if ( strcmp(ASTNode_getName(old), name) == 0 ) {

      /* replace the arguments of the function definition copied to `new', 
         with the arguments passed by the function call(s) in `math' */
      for ( j=0; j<(ASTNode_getNumChildren(function)-1); j++ ) {
	AST_replaceNameByFormula(new,
				 ASTNode_getName(ASTNode_getChild(function,
								  j)),
				 ASTNode_getChild(old, j));
      }

      /* copy the `new' function defintion with replaced parameters
	 into the `old' function call */
      
      /* first set possible names or numbers */
      if ( ASTNode_isName(new) ) {
	ASTNode_setName(old, ASTNode_getName(new));
	
      }
      else if ( ASTNode_isInteger(new) ) {
	ASTNode_setInteger(old, ASTNode_getInteger(new));
      }
      else if ( ASTNode_isReal(new) ) {
	ASTNode_setReal(old, ASTNode_getReal(new));
      }
      /* ... if none of the above, just set the AST Type ... */
      else {
	ASTNode_setType(old, ASTNode_getType(new));
	/* (a user defined function has a name that must be set) */
	if ( ASTNode_getType(new) == AST_FUNCTION ) {
	  ASTNode_setName(old, ASTNode_getName(new));
	}
	/* ... and exchange the children. That should be it! */
	ASTNode_swapChildren(old, new);
      }
        
    }
    ASTNode_free(new);
  }
  List_free(names);
}

/** Replace all constants of a model in an AST math */

void
AST_replaceConstants(Model_t *m, ASTNode_t *math) {


  int i, j, found;
  Parameter_t *p;
  Compartment_t *c;
  Species_t *s;
  Rule_t *rl;
  AssignmentRule_t *ar;
  RateRule_t *rr;
  SBMLTypeCode_t type;
  FunctionDefinition_t *f;

  /** Step R.1: replace Assignment Rules
      Parameters, compartments or species defined by
      assignment rules in the model will be replaced
      by the assignment expression in the AST formula      
  */

  /**
     Starting from the back, because variables defined by
     assignment rules can be used is subsequent assignments.
     Thus this direction should catch all assignments.
  */
  for ( i=(Model_getNumRules(m)-1); i>=0; i-- ) {
    rl = Model_getRule(m, i);
    type = SBase_getTypeCode((SBase_t *)rl);
    if ( type == SBML_ASSIGNMENT_RULE ) {
      ar = (AssignmentRule_t *)rl;
      if ( Rule_isSetMath(rl) && AssignmentRule_isSetVariable(ar) ) {
	AST_replaceNameByFormula(math,
				 AssignmentRule_getVariable(ar),
				 Rule_getMath(rl));
      }
    }
  }

 
  /** Step R.2: replace Function Definitions
      All Function Definitions will be replaced
      by the full expression
  */
  
  for ( i=0; i<Model_getNumFunctionDefinitions(m); i++ ) {
    f = Model_getFunctionDefinition(m, i);
    AST_replaceFunctionDefinition(math,
				  FunctionDefinition_getId(f),
				  FunctionDefinition_getMath(f));
  }

  /** Steps R.3: replacing all constant global parameters
      in rate rules, algebraic rules and events
      by their value.
  */
  for ( i=0; i<Model_getNumParameters(m); i++) {
    p = Model_getParameter(m, i);
    if ( Parameter_getConstant(p) ) {
      AST_replaceNameByValue(math,
			     Parameter_getId(p),
			     Parameter_getValue(p));
    }
  }
  
  /** Steps R.4: replacing all constant compartments
      in rate rules, algebraic rules and events
      by their size
  */

  for ( i=0; i<Model_getNumCompartments(m); i++) {
    c = Model_getCompartment(m, i);
    if ( Compartment_getConstant(c) ) {
      AST_replaceNameByValue(math,
			     Compartment_getId(c),
			     Compartment_getSize(c));
    }
  }
  /** Steps R.5: replacing all species that
      are defined as either constant or boundary
      but not defined by a rate rule (i.e. also constant)
      by their initial concentration.
      Species that are set by an assignment rules
      have already been replaced above in Step R.1
  */
  for ( i=0; i<Model_getNumSpecies(m); i++) {
    found = 0;
    s = Model_getSpecies(m, i);
    c = Model_getCompartmentById(m, Species_getCompartment(s));
    if ( Species_getConstant(s) ) {
      AST_replaceNameByValue(math,
			     Species_getId(s),
			     Species_isSetInitialConcentration(s) ?
			     Species_getInitialConcentration(s) :
			     Species_getInitialAmount(s) /
			     Compartment_getSize(c));
    }
    else if ( Species_getBoundaryCondition(s) ) {
      for ( j=0; j<Model_getNumRules(m); j++ ) {
	rl = Model_getRule(m, j);
	type = SBase_getTypeCode((SBase_t *)rl);
	if ( type == SBML_RATE_RULE ) {
	  rr = (RateRule_t *)rl;
	  if ( Rule_isSetMath(rl) && RateRule_isSetVariable(rr) ) {
	    if ( strcmp(RateRule_getVariable(rr), Species_getId(s)) ==0 ) {
	      ++found;
	    }
	  }
	}
	else if ( type == SBML_ASSIGNMENT_RULE ) {
	  ar = (AssignmentRule_t *)rl;	  
	  if ( Rule_isSetMath(rl) && AssignmentRule_isSetVariable(ar) ) {
	    if ( strcmp(AssignmentRule_getVariable(ar), Species_getId(s))
		 == 0 ) {
	      ++found;
	    }
	  }
	}
      }
      if ( found == 0 ) {
	AST_replaceNameByValue(math,
			       Species_getId(s),
			       Species_isSetInitialConcentration(s) ?
			       Species_getInitialConcentration(s) :
			       Species_getInitialAmount(s) /
			       Compartment_getSize(c));
      }    
    }
  }
} 
/* End of file */
