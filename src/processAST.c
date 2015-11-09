/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Last changed Time-stamp: <2008-10-13 16:21:41 raim>
  $Id: processAST.c,v 1.65 2008/10/16 17:25:40 raimc Exp $
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
 *     Rainer Machne
 *
 * Contributor(s):
 *     Stefan Müller
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*! \defgroup processAST Formula Processing: f(x), df/dx
  \ingroup symbolic
  \brief This module contains all functions for evaluation of and
  symbolic operations on formulae represented as libSBML
  Abstract Syntax Trees (AST).

  These functions are used both for numerical and symbolic analysis

*/
/*@{*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sbml/SBMLTypes.h>

/* System specific definitions,
   created by configure script */
#ifndef _WIN32
#include "config.h"
#endif

#include "sbmlsolver/processAST.h"
#include "sbmlsolver/ASTIndexNameNode.h"
#include "sbmlsolver/solverError.h"

/* Helper Macros to get the second or the third child
   of an Abstract Syntax Tree */
#define child2(x,y,z)  ASTNode_getChild(ASTNode_getChild(x,y),z)
#define child3(x,y,z,w) ASTNode_getChild(ASTNode_getChild(ASTNode_getChild(x,y),z),w)

/* local functions  */

static int user_defined(ASTNode_t *node);
static int zero(ASTNode_t *f);
static int one(ASTNode_t *f); 
static ASTNode_t *ASTNode_cutRoot(ASTNode_t *old);

/* ------------------------------------------------------------------------ */

/** writes the given AST to standard out.  The string 'context'
    followed by a space is output before the AST
*/
void AST_dump(const char *context, ASTNode_t *node)
{
  char *buffer = SBML_formulaToString(node);
  printf("%s %s\n", context, buffer);
  free(buffer);
}

/** Copies the passed AST, including potential SOSlib ASTNodeIndex, and
    returns the copy.
    Now this function is equivalent to ASTNode_deepCopy() because
    the index is stored as its user data.
*/

SBML_ODESOLVER_API ASTNode_t *copyAST(const ASTNode_t *f)
{
	return ASTNode_deepCopy(f);
}

/* ------------------------------------------------------------------------ */

static ASTNode_t *getDerivArccsc(ASTNode_t *f, char *x)
{
  /** f(x)=arccsc(a(x)) => f' = - a' / (abs(a) * sqrt(a^2 - 1)) */

  ASTNode_t *f0;
  ASTNode_t *g, *g0, *g1, *g10, *g11, *g111, *g1110, *g1111;

  f0 = ASTNode_getChild(f, 0);

  g = ASTNode_create();
  ASTNode_setType(g, AST_DIVIDE);

  g0 = ASTNode_create();
  ASTNode_addChild(g, g0);
  ASTNode_setType(g0, AST_MINUS);
  ASTNode_addChild(g0, differentiateAST(f0, x));

  g1 = ASTNode_create();
  ASTNode_addChild(g, g1);
  ASTNode_setType(g1, AST_TIMES);

  g10 = ASTNode_create();
  ASTNode_addChild(g1, g10);
  ASTNode_setType(g10, AST_FUNCTION_ABS);
  ASTNode_addChild(g10, copyAST(f0));

  g11 = ASTNode_create();
  ASTNode_addChild(g1, g11);
  ASTNode_setType(g11, AST_FUNCTION_ROOT);
  ASTNode_addChild(g11, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g11, 0), 2);

  g111 = ASTNode_create();
  ASTNode_addChild(g11, g111);
  ASTNode_setType(g111, AST_MINUS);

  g1110 = ASTNode_create();
  ASTNode_addChild(g111, g1110);
  ASTNode_setType(g1110, AST_POWER);
  ASTNode_addChild(g1110, copyAST(f0));
  ASTNode_addChild(g1110, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g1110, 1), 2);

  g1111 = ASTNode_create();
  ASTNode_addChild(g111, g1111);
  ASTNode_setInteger(g1111, 1);

  return g;
}

static ASTNode_t *getDerivArccsch(ASTNode_t *f, char *x)
{
  /** f(x)=arccsch(a(x)) => f' = - a' / (a^2 * sqrt(1 + 1/a^2)) */

  ASTNode_t *f0;
  ASTNode_t *p2a, *p2b;
  ASTNode_t *g, *g0, *g1, *g11, *g111, *g1111;

  f0 = ASTNode_getChild(f, 0);

  p2a = ASTNode_create();
  ASTNode_setType(p2a, AST_POWER);
  ASTNode_addChild(p2a, copyAST(f0));
  ASTNode_addChild(p2a, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(p2a, 1), 2);

  p2b = copyAST(p2a);

  g = ASTNode_create();
  ASTNode_setType(g, AST_DIVIDE);

  g0 = ASTNode_create();
  ASTNode_addChild(g, g0);
  ASTNode_setType(g0, AST_MINUS);
  ASTNode_addChild(g0, differentiateAST(f0, x));

  g1 = ASTNode_create();
  ASTNode_addChild(g, g1);
  ASTNode_setType(g1, AST_TIMES);
  ASTNode_addChild(g1, p2a);

  g11 = ASTNode_create();
  ASTNode_addChild(g1, g11);
  ASTNode_setType(g11, AST_FUNCTION_ROOT);
  ASTNode_addChild(g11, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g11, 0), 2);

  g111 = ASTNode_create();
  ASTNode_addChild(g11, g111);
  ASTNode_setType(g111, AST_PLUS);
  ASTNode_addChild(g111, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g111, 0), 1);

  g1111 = ASTNode_create();
  ASTNode_addChild(g111, g1111);
  ASTNode_setType(g1111, AST_DIVIDE);
  ASTNode_addChild(g1111, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g1111, 0), 1);
  ASTNode_addChild(g1111, p2b);

  return g;
}

static ASTNode_t *getDerivArcsec(ASTNode_t *f, char *x)
{
  /** f(x)=arcsec(a(x)) => f' = - a' / (a^2 * sqrt(1 - 1/a^2)) */

  ASTNode_t *f0;
  ASTNode_t *p2a, *p2b;
  ASTNode_t *g, *g0, *g1, *g11, *g111, *g1111;

  f0 = ASTNode_getChild(f, 0);

  p2a = ASTNode_create();
  ASTNode_setType(p2a, AST_POWER);
  ASTNode_addChild(p2a, copyAST(f0));
  ASTNode_addChild(p2a, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(p2a, 1), 2);

  p2b = copyAST(p2a);

  g = ASTNode_create();
  ASTNode_setType(g, AST_DIVIDE);

  g0 = ASTNode_create();
  ASTNode_addChild(g, g0);
  ASTNode_setType(g0, AST_MINUS);
  ASTNode_addChild(g0, differentiateAST(f0, x));

  g1 = ASTNode_create();
  ASTNode_addChild(g, g1);
  ASTNode_setType(g1, AST_TIMES);
  ASTNode_addChild(g1, p2a);

  g11 = ASTNode_create();
  ASTNode_addChild(g1, g11);
  ASTNode_setType(g11, AST_FUNCTION_ROOT);
  ASTNode_addChild(g11, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g11, 0), 2);

  g111 = ASTNode_create();
  ASTNode_addChild(g11, g111);
  ASTNode_setType(g111, AST_MINUS);
  ASTNode_addChild(g111, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g111, 0), 1);

  g1111 = ASTNode_create();
  ASTNode_addChild(g111, g1111);
  ASTNode_setType(g1111, AST_DIVIDE);
  ASTNode_addChild(g1111, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g1111, 0), 1);
  ASTNode_addChild(g1111, p2b);

  return g;
}

static ASTNode_t *getDerivArcsech(ASTNode_t *f, char *x)
{
  /** f(x)=arcsech(a(x)) => f' = a' * (sqrt((1-a)/(1+a)) / (a * (a-1)))
   */

  ASTNode_t *f0;
  ASTNode_t *g, *g1, *g10, *g11, *g101, *g1010, *g1011, *g111;

  f0 = ASTNode_getChild(f, 0);

  g = ASTNode_create();
  ASTNode_setType(g, AST_TIMES);
  ASTNode_addChild(g, differentiateAST(f0, x));

  g1 = ASTNode_create();
  ASTNode_addChild(g, g1);
  ASTNode_setType(g1, AST_DIVIDE);

  g10 = ASTNode_create();
  ASTNode_addChild(g1, g10);
  ASTNode_setType(g10, AST_FUNCTION_ROOT);
  ASTNode_addChild(g10, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g10, 0), 2);

  g101 = ASTNode_create();
  ASTNode_addChild(g10, g101);
  ASTNode_setType(g101, AST_DIVIDE);

  g1010 = ASTNode_create();
  ASTNode_addChild(g101, g1010);
  ASTNode_setType(g1010, AST_MINUS);
  ASTNode_addChild(g1010, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g1010, 0), 1);
  ASTNode_addChild(g1010, copyAST(f0));

  g1011 = ASTNode_create();
  ASTNode_addChild(g101, g1011);
  ASTNode_setType(g1011, AST_PLUS);
  ASTNode_addChild(g1011, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g1011, 0), 1);
  ASTNode_addChild(g1011, copyAST(f0));

  g11 = ASTNode_create();
  ASTNode_addChild(g1, g11);
  ASTNode_setType(g11, AST_TIMES);
  ASTNode_addChild(g11, copyAST(f0));

  g111 = ASTNode_create();
  ASTNode_addChild(g11, g111);
  ASTNode_setType(g111, AST_MINUS);
  ASTNode_addChild(g111, copyAST(f0));
  ASTNode_addChild(g111, ASTNode_create());
  ASTNode_setInteger(ASTNode_getChild(g111, 1), 1);

  return g;
}

/** Returns the derivative f' of the passed formula f with respect to
    the passed variable x, using basic differentiation rules.
*/

SBML_ODESOLVER_API ASTNode_t *differentiateAST(ASTNode_t *f, char *x)
{
  unsigned int i, j, childnum;
  int found;
  ASTNodeType_t type;
  ASTNode_t *fprime, *helper, *simple;
  ASTNode_t *help_1, *help_2, *help_3;
  ASTNode_t *prod, *sum, *tmp;
  List_t *list;
  char *fname, *dfname;
  
  fprime = ASTNode_create();

  /** VARIABLES */
  
  /** check if variable x is part of f(x): if not f' = 0 */  
  found = 0;
  list = ASTNode_getListOfNodes(f, (ASTNodePredicate) user_defined);
  for ( i=0; i<List_size(list); i++ )
    if ( strcmp(ASTNode_getName(List_get(list, i)), x) == 0 ) 
      found = 1;
  List_free(list);

  if ( found == 0 )
  {
    ASTNode_setReal(fprime, 0.0);
    return fprime;
  }

  /* helpful information used below */
  type = ASTNode_getType(f);
  childnum = ASTNode_getNumChildren(f);

  /* DISTINCTION OF CASES */

  /* if the ASTNode is a Name node it must be the variable x because  
     other name node would have been catched above; the same is true
     for or a Constant nodes (Pi, E, TRUE, FALSE) */
  /** NAME: f(x)=x  =>  f'(x)=1.0*/


 if ( ASTNode_isName(f) )
 {
  if ( ASTNode_isSetData(f)  ) 
    ASTNode_setReal(fprime, 0.0); /* it's observation data */ 
  else 
    ASTNode_setReal(fprime, 1.0); /* is variable, and not observation data */
 }
    

  /** OPERATORS */
  else if ( ASTNode_isOperator(f) || type==AST_FUNCTION_POWER )
  {
    switch(type)
    {
    case AST_PLUS:
      /** f(x)=a(x)+b(x) => f'(x) = a' + b' */
      ASTNode_setType(fprime, AST_PLUS);
      for ( i=0; i<childnum; i++ ) 
	ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f, i),x));
      break;
    case AST_MINUS:
      /** f(x)=a(x)-b(x) => f'(x) = a' - b' */
      ASTNode_setType(fprime, AST_MINUS);
      for ( i=0; i<childnum; i++ ) 
	ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f, i),x));
      break;
    case AST_TIMES:
      /** catch n-ary operators with operand number != 2,
	  and decompose in simplifyAST */
      if ( ASTNode_getNumChildren(f) != 2 )
      {
	helper = simplifyAST(f); /* decomposes the n-ary operator */
	ASTNode_free(fprime);
	fprime = differentiateAST(helper, x);
	ASTNode_free(helper);
      }
      else
      {
	/** f(x)=a(x)*b(x) => f'(x) = a'*b + a*b' */
	ASTNode_setType(fprime, AST_PLUS);    

	ASTNode_addChild(fprime, ASTNode_create());
	help_1 = ASTNode_getChild(fprime, 0);
	ASTNode_setType (help_1, AST_TIMES);
	ASTNode_addChild(help_1, differentiateAST(ASTNode_getChild(f, 0), x));
	ASTNode_addChild(help_1, copyAST(ASTNode_getChild(f, 1)));

	ASTNode_addChild(fprime, ASTNode_create());
	help_1 = ASTNode_getChild(fprime, 1);
	ASTNode_setType (help_1, AST_TIMES);
	ASTNode_addChild(help_1, copyAST(ASTNode_getChild(f, 0)));
	ASTNode_addChild(help_1, differentiateAST(ASTNode_getChild(f, 1), x));
      }
      break;
    case AST_DIVIDE:
      /** f(x)=a(x)/b(x) => f'(x) = a'/b - a/b^2*b' */    
      ASTNode_setType(fprime, AST_MINUS);

      ASTNode_addChild(fprime, ASTNode_create());
      help_1 = ASTNode_getChild(fprime, 0);
      ASTNode_setType (help_1, AST_DIVIDE);
      ASTNode_addChild(help_1, differentiateAST(ASTNode_getChild(f, 0), x));
      ASTNode_addChild(help_1, copyAST(ASTNode_getChild(f, 1)));

      ASTNode_addChild(fprime, ASTNode_create());
      help_1 = ASTNode_getChild(fprime, 1);
      ASTNode_setType (help_1, AST_TIMES);
      ASTNode_addChild(help_1, ASTNode_create());
      ASTNode_addChild(help_1, differentiateAST(ASTNode_getChild(f, 1), x));

      help_2 = ASTNode_getChild(help_1, 0);
      ASTNode_setType (help_2, AST_DIVIDE);
      ASTNode_addChild(help_2, copyAST(ASTNode_getChild(f, 0)));
      ASTNode_addChild(help_2, ASTNode_create());

      help_3 = ASTNode_getChild(help_2, 1);
      ASTNode_setType (help_3, AST_POWER);
      ASTNode_addChild(help_3, copyAST(ASTNode_getChild(f, 1)));
      ASTNode_addChild(help_3, ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(help_3, 1), 2);
      break;
    case AST_POWER:
    case AST_FUNCTION_POWER:
      /** f(x)=a(x)^b => f'(x) = b * a^(b-1)*a' */
      /* check if variable x is part of b(x) */
      found = 0;  
      list = ASTNode_getListOfNodes(ASTNode_getChild(f, 1),
				    (ASTNodePredicate) user_defined);
      for ( i=0; i<List_size(list); i++ ) 
	if ( strcmp(ASTNode_getName(List_get(list, i)), x) == 0 ) 
	  found = 1;
      List_free(list);
      if ( found == 0 )
      {
	ASTNode_setType (fprime, AST_TIMES);

	ASTNode_addChild(fprime, copyAST(ASTNode_getChild(f, 1)));
	ASTNode_addChild(fprime, ASTNode_create());

	help_1 = ASTNode_getChild(fprime, 1);
	ASTNode_setType (help_1, AST_TIMES);
	ASTNode_addChild(help_1, ASTNode_create());
	ASTNode_addChild(help_1, differentiateAST(ASTNode_getChild(f, 0), x));

	help_2 = ASTNode_getChild(help_1, 0);
	ASTNode_setType (help_2, AST_POWER);
	ASTNode_addChild(help_2, copyAST(ASTNode_getChild(f, 0)));
	ASTNode_addChild(help_2, ASTNode_create());

	help_3 = ASTNode_getChild(help_2, 1);
	ASTNode_setType (help_3, AST_MINUS);
	ASTNode_addChild(help_3, copyAST(ASTNode_getChild(f, 1)));
	ASTNode_addChild(help_3, ASTNode_create());
	ASTNode_setReal(ASTNode_getChild(help_3, 1), 1.0);	
	break;
      }
      /** f(x)=a^b(x) => f'(x) = f * ln(a)*b' */
      /* check if variable x is part of a(x) */
      found = 0;  
      list = ASTNode_getListOfNodes(ASTNode_getChild(f, 0),
				    (ASTNodePredicate) user_defined);
      for ( i=0; i<List_size(list); i++ )
	if ( strcmp(ASTNode_getName(List_get(list, i)), x) == 0 ) 
	  found = 1;
      List_free(list);
      if ( found == 0 )
      {
	ASTNode_setType (fprime, AST_TIMES);
	ASTNode_addChild(fprime, copyAST(f));
	ASTNode_addChild(fprime, ASTNode_create());
	help_1 = ASTNode_getChild(fprime, 1);
	ASTNode_setType (help_1, AST_TIMES);
	ASTNode_addChild(help_1, ASTNode_create());
	ASTNode_addChild(help_1, differentiateAST(ASTNode_getChild(f, 1), x));
	help_2 = ASTNode_getChild(help_1, 0);
	ASTNode_setType (help_2, AST_FUNCTION_LN);
	ASTNode_addChild(help_2, copyAST(ASTNode_getChild(f, 0)));
	break;
      }
      /** f(x)=a(x)^b(x) => f'(x)= f * ( b/a*a' + ln(a)*b' ) */
      ASTNode_setType (fprime, AST_TIMES);
      ASTNode_addChild(fprime, copyAST(f));
      ASTNode_addChild(fprime, ASTNode_create());
      help_1 = ASTNode_getChild(fprime, 1);
      ASTNode_setType (help_1, AST_PLUS);
      ASTNode_addChild(help_1, ASTNode_create());
      ASTNode_addChild(help_1, ASTNode_create());
      help_2 = ASTNode_getChild(help_1, 0);
      ASTNode_setType (help_2, AST_TIMES);
      ASTNode_addChild(help_2, ASTNode_create());
      ASTNode_addChild(help_2, differentiateAST(ASTNode_getChild(f, 0), x));
      help_3 = ASTNode_getChild(help_2, 0);
      ASTNode_setType (help_3, AST_DIVIDE);
      ASTNode_addChild(help_3, copyAST(ASTNode_getChild(f, 1)));
      ASTNode_addChild(help_3, copyAST(ASTNode_getChild(f, 0)));
      help_2 = ASTNode_getChild(help_1, 1);
      ASTNode_setType (help_2, AST_TIMES);
      ASTNode_addChild(help_2, ASTNode_create());
      ASTNode_addChild(help_2, differentiateAST(ASTNode_getChild(f, 1), x));
      help_3 = ASTNode_getChild(help_2, 0);
      ASTNode_setType (help_3, AST_FUNCTION_LN);
      ASTNode_addChild(help_3, copyAST(ASTNode_getChild(f, 0)));
      break;
    default:
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_OPERATOR,
                        "differentiateAST: operator: impossible case");
      ASTNode_setName(fprime, "differentiation_failed");
    }
  }
  /** FUNCTIONS: */
  else if ( ASTNode_isFunction(f) || type==AST_LAMBDA )
  {
    switch(type)
    {
    case AST_LAMBDA:
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_LAMBDA,
                        "differentiateAST: lambda: not implemented");
      ASTNode_setName(fprime, "differentiation_failed");
      break;
    case AST_FUNCTION:
      fname  = (char *) ASTNode_getName(f);
      /* feature for inverse problems:
	 differentiation for (sic!) a user-defined function */
      if ( strcmp(fname, x) == 0 )
      {
	ASTNode_setType (fprime, AST_FUNCTION);	
	ASSIGN_NEW_MEMORY_BLOCK(dfname, strlen(fname)+3, char, NULL);
	sprintf(dfname, "d_%s", fname);
	ASTNode_setName(fprime, dfname);
	free(dfname);
	for ( j=0; j<childnum; j++ )
	  ASTNode_addChild(fprime, copyAST(ASTNode_getChild(f, j)));
	break;
      }
      /* differentiation of a user-defined function for a variable */
      ASTNode_free(fprime); /* not needed yet */ 
      sum = tmp = NULL;
      /* loop over arguments */
      for ( i=0; i<childnum; i++ )
      {
	/* does argument depend on x? */
	found = 0;  
	list = ASTNode_getListOfNodes(ASTNode_getChild(f, i),
				      (ASTNodePredicate) user_defined);
	for ( j=0; j<List_size(list); j++ ) 
	  if ( strcmp(ASTNode_getName(List_get(list, j)), x) == 0 ) 
	    found = 1;
	List_free(list);
	/* calculate summands */
	prod = ASTNode_create();
	if ( found == 0 )	  /* argument does not depend on x */
	  ASTNode_setReal(prod, 0.0);
	else                      /* argument depends on x */
	{
	  ASTNode_setType (prod, AST_TIMES);
	  ASTNode_addChild(prod, ASTNode_create());
	  ASTNode_addChild(prod, differentiateAST(ASTNode_getChild(f, i) , x));
	  helper = ASTNode_getChild(prod, 0);
	  ASTNode_setType (helper, AST_FUNCTION);
	  ASSIGN_NEW_MEMORY_BLOCK(dfname, strlen(fname)+3+32, char, NULL);
	  sprintf(dfname, "d%i_%s", i, fname);
	  ASTNode_setName(helper, dfname);
	  free(dfname);
	  for ( j=0; j<childnum; j++ ) 
	    ASTNode_addChild(helper, copyAST(ASTNode_getChild(f, j)));
	}
	if ( i==0 )  /* first summand */
	  sum = prod;	
	else         /* other summands */
	{
	  sum = ASTNode_create();
	  ASTNode_setType (sum, AST_PLUS);
	  ASTNode_addChild(sum, tmp);
	  ASTNode_addChild(sum, prod);
	}
	tmp = sum;
      }
      /* result */
      fprime = sum;
      break;
    case AST_FUNCTION_ABS:             
      /** f(x)=abs(a(x)) => f' = sig(a)*a'\n
	  RESULTS IN A DISCONTINUOUS FUNCTION!\n
          sig(a) is set to 0 if a==0, while it is actually not defined */ 
      ASTNode_setType(fprime, AST_TIMES);
      /* piecewise node for signum function, sig(a) */
      helper = ASTNode_create();
      ASTNode_setType(helper, AST_FUNCTION_PIECEWISE);
      /* -1 if a < 0 */
      ASTNode_addChild(helper, ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(helper, 0), -1);
      ASTNode_addChild(helper, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(helper, 1), AST_RELATIONAL_LT);
      ASTNode_addChild(ASTNode_getChild(helper, 1),
		       copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild(ASTNode_getChild(helper, 1), ASTNode_create());
      ASTNode_setInteger(child2(helper,1,1), 0);      
      /* 0 if a = 0; ACTUALLY sig(a) is not defined for a==0 !! */
      ASTNode_addChild(helper, ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(helper, 2), 0);
      ASTNode_addChild(helper, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(helper, 3), AST_RELATIONAL_EQ);
      ASTNode_addChild(ASTNode_getChild(helper, 3),
		       copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild(ASTNode_getChild(helper, 3), ASTNode_create());
      ASTNode_setInteger(child2(helper,3,1), 0);      
      /* +1 if a > 0 */
      ASTNode_addChild(helper, ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(helper, 4), 1);
      ASTNode_addChild(helper, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(helper, 5), AST_RELATIONAL_GT);
      ASTNode_addChild(ASTNode_getChild(helper, 5),
		       copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild(ASTNode_getChild(helper, 5), ASTNode_create());
      ASTNode_setInteger(child2(helper,5,1), 0);      
      /* multiply: sig(a) * a' */		       
      ASTNode_addChild(fprime, helper);
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      break;

      /** TRIGONOMETRIC FUNCTION DERIVATIVES
	  mostly taken from\n
	  http://www.rism.com/Trig/circular.htm and\n
	  http://www.rism.com/Trig/hyperbol.htm */  
    case AST_FUNCTION_ARCCOS:
      /** f(x)=arccos(a(x)) => f' = - a' / sqrt(1 - a^2)  */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  - a'  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /*  sqrt(...)  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_FUNCTION_ROOT);
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,0), 2);
      /* 1 - a^2) */
      /*  1  - */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType( child2(fprime,1,1), AST_MINUS );
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger(child3(fprime,1,1,0), 1);
      /*  a^2  */
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setType(  child3(fprime,1,1,1), AST_POWER);
      ASTNode_addChild( child3(fprime,1,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child3(fprime,1,1,1), ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(child3(fprime,1,1,1),1), 2);
      break;
    case AST_FUNCTION_ARCCOSH:
      /** f(x)=arccosh(a(x)) => f' =   a' / sqrt(a^2 - 1)  */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  a'  */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /*  sqrt(...)  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_FUNCTION_ROOT);
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,0), 2);
      /* a^2 - 1 */
      /*  a^2  - */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType( child2(fprime,1,1), AST_MINUS );
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setType(child3(fprime,1,1,0), AST_POWER);
      ASTNode_addChild(child3(fprime,1,1,0), copyAST(ASTNode_getChild(f,0)));
    
      ASTNode_addChild(child3(fprime,1,1,0), ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(child3(fprime,1,1,0),1), 2);
      /*  1  */
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger(child3(fprime,1,1,1), 1);
      break;
    case AST_FUNCTION_ARCCOT:
      /** f(x)=arccot(a(x)) => f' = - a' / (1 + a^2) */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  - a'  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /*  1 + a^2  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_PLUS);
      /*  1  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger( child2(fprime,1,0), 1);
      /*  a^2  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(  child2(fprime,1,1), AST_POWER);
      ASTNode_addChild( child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger( child3(fprime,1,1,1), 2);     
      break;
    case AST_FUNCTION_ARCCOTH:
      /** f(x)=arccoth(a(x)) => f' = - a' / (-1 + a^2) */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  - a'  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /*  -1 + a^2  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_PLUS);
      /*  -1  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_MINUS);
      ASTNode_addChild(child2(fprime,1,0), ASTNode_create());
      ASTNode_setInteger(child3(fprime,1,0,0), 1);
      /*  a^2  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(  child2(fprime,1,1), AST_POWER);
      ASTNode_addChild( child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger( child3(fprime,1,1,1), 2);    
      break;
    case AST_FUNCTION_ARCCSC:
      fprime = getDerivArccsc(f, x);
      break;
    case AST_FUNCTION_ARCCSCH:
      fprime = getDerivArccsch(f, x);
      break;
    case AST_FUNCTION_ARCSEC:
      fprime = getDerivArcsec(f, x);
      break;
    case AST_FUNCTION_ARCSECH:
      fprime = getDerivArcsech(f, x);
      break;
    case AST_FUNCTION_ARCSIN:
      /** f(x)=arcsin(a(x)) => f' = a' / sqrt(1 - a^2)  */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  a'  */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /*  sqrt(...)  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_FUNCTION_ROOT);
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,0), 2);
      /* 1 - a^2) */
      /*  1  - */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType( child2(fprime,1,1), AST_MINUS );
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger(child3(fprime,1,1,0), 1);
      /*  a^2  */
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setType(  child3(fprime,1,1,1), AST_POWER);
      ASTNode_addChild( child3(fprime,1,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child3(fprime,1,1,1), ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(child3(fprime,1,1,1),1), 2);
      break;
    case AST_FUNCTION_ARCSINH:
      /** f(x)=arcsinh(a(x)) => f' = a' / sqrt(1 + a^2)  */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  a'  */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /*  sqrt(...)  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_FUNCTION_ROOT);
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,0), 2);
      /* 1 + a^2) */
      /*  1  + */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType( child2(fprime,1,1), AST_PLUS );
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger(child3(fprime,1,1,0), 1);
      /*  a^2  */
      ASTNode_addChild(child2(fprime,1,1), ASTNode_create());
      ASTNode_setType(  child3(fprime,1,1,1), AST_POWER);
      ASTNode_addChild( child3(fprime,1,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child3(fprime,1,1,1), ASTNode_create());
      ASTNode_setInteger(ASTNode_getChild(child3(fprime,1,1,1),1), 2);
      break;
    case AST_FUNCTION_ARCTAN:
      /** f(x)=atan(a(x)) => f' = a' / (1 + a^2) */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  a'  */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /*  1 + a^2  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_PLUS);
      /*  1  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger( child2(fprime,1,0), 1);
      /*  a^2  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(  child2(fprime,1,1), AST_POWER);
      ASTNode_addChild( child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger( child3(fprime,1,1,1), 2);    
      break;
    case AST_FUNCTION_ARCTANH:
      /** f(x)=atan(a(x)) => f' = a' / (1 - a^2) */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  a'  */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /*  1 - a^2  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_MINUS);
      /*  1  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger( child2(fprime,1,0), 1);
      /*  a^2  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(  child2(fprime,1,1), AST_POWER);
      ASTNode_addChild( child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild( child2(fprime,1,1), ASTNode_create());
      ASTNode_setInteger( child3(fprime,1,1,1), 2);     
      break;
    case AST_FUNCTION_CEILING:
      /** f(x) = ceil(a(x)) */
      ASTNode_setType(fprime, ASTNode_getType(f));
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_COS:
      /** f(x)=cos(a(x)) => f' = a' * -sin(a) */   
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0), ASTNode_create());      
      ASTNode_setType(child2(fprime,0,0), AST_FUNCTION_SIN);
      ASTNode_addChild(child2(fprime,0,0), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild(fprime,differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_COSH:
      /** f(x)=cosh(a(x)) => f' = a' * sinh(a) */
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_FUNCTION_SINH);
      ASTNode_addChild(ASTNode_getChild(fprime,0), copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild(fprime,differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_COT:
      /** f(x)=cot(a(x)) => f' = - a' / (sin(a))^2 */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  - a'  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /*  (sin(a))^2  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_POWER);
      /*  sin(a)  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_SIN);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /*  ^2  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,1), 2);  
      break;
    case AST_FUNCTION_COTH:
      /** f(x)=cot(a(x)) => f' = - a' / (sinh(a))^2 */
      ASTNode_setType(fprime,AST_DIVIDE);
      /*  - a'  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /*  (sinh(a))^2  */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_POWER);
      /*  sinh(a)  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_SINH);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /*  ^2  */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,1), 2);  
      break;  
    case AST_FUNCTION_CSC:
      /** f(x)=csc(a(x)) => f' = - a' * csc(a) * cot(a) */
      /* - a' * ... */
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /* csc(a) * cot(x) */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_TIMES);
      /* csc(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_CSC);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /* tan(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,1), AST_FUNCTION_COT);
      ASTNode_addChild(child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));
      break;
    case AST_FUNCTION_CSCH:
      /** f(x)=csch(a(x)) => f' = - a' * csch(a) * coth(a) */
      /* - a' * ... */
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /* csch(a) * coth(x) */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_TIMES);
      /* csch(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_CSCH);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /* tanh(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,1), AST_FUNCTION_COTH);
      ASTNode_addChild(child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));
      break;
    case AST_FUNCTION_DELAY:
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_DELAY,
                        "differentiateAST: delay: not implemented");
      ASTNode_setName(fprime, "differentiation_failed");
      break;
    case AST_FUNCTION_EXP:
      /** f(x)=e^a(x) =>  f' = e^a * a'  */
      ASTNode_setType(fprime, AST_TIMES);
      ASTNode_addChild(fprime, copyAST(f));
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_FACTORIAL:
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_FACTORIAL,
                        "differentiateAST: factorial: impossible case");
      ASTNode_setName(fprime, "differentiation_failed");
      break;
    case AST_FUNCTION_FLOOR:     /* WRONG */
      /** f(x) = floor(a(x))  \n
	  WRONG: CAN RESULT IN A DISCONTINUOUS FUNCTION! */
      ASTNode_setType(fprime, ASTNode_getType(f));
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_LN:
      /** f(x)=ln(a(x)) => f' = 1 / a * a' */
      ASTNode_setType (fprime, AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f, 0), x));
      help_1 = ASTNode_getChild(fprime, 0);
      ASTNode_setType (help_1, AST_DIVIDE);
      ASTNode_addChild(help_1, ASTNode_create());
      ASTNode_addChild(help_1, copyAST(ASTNode_getChild(f, 0)));
      help_2 = ASTNode_getChild(help_1, 0);
      ASTNode_setReal (help_2, 1.0);    
      break;
    case AST_FUNCTION_LOG:
      /** f(x)=log(b,x) = 1/ln(b)*ln(x)  \n
	  replace and differentiate */
      ASTNode_setType (fprime, AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      help_1 = ASTNode_getChild(fprime, 0);
      ASTNode_setType (help_1, AST_DIVIDE);
      ASTNode_addChild(help_1, ASTNode_create());
      ASTNode_addChild(help_1, ASTNode_create());
      help_2 = ASTNode_getChild(help_1, 0);
      ASTNode_setReal (help_2, 1.0);
      help_2 = ASTNode_getChild(help_1, 1);
      ASTNode_setType (help_2, AST_FUNCTION_LN);
      ASTNode_addChild(help_2, copyAST(ASTNode_getChild(f, 0)));

      helper = ASTNode_create();
      ASTNode_setType (helper, AST_FUNCTION_LN);
      ASTNode_addChild(helper, copyAST(ASTNode_getChild(f, 1)));
      
      ASTNode_addChild(fprime, differentiateAST(helper, x));
      ASTNode_free(helper);
      break;
    case AST_FUNCTION_PIECEWISE:
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_FACTORIAL,
                        "differentiateAST: piecewise: not implemented");
      ASTNode_setName(fprime, "differentiation_failed");
      break;
    case AST_FUNCTION_ROOT:
      /** f(x)=root(a(x),b(x)) = a(x)^(1/b(x))  \n
	  replace and differentiate */
      helper = ASTNode_create();
      /* a ^ . */
      ASTNode_setType (helper, AST_FUNCTION_POWER);
      ASTNode_addChild(helper, copyAST(ASTNode_getChild(f,0)));
      ASTNode_addChild(helper, ASTNode_create());      
      /* a ^ . / . */     
      help_1 = ASTNode_getChild(helper, 1);
      ASTNode_setType (help_1, AST_DIVIDE);
      ASTNode_addChild(help_1, ASTNode_create());
      /* a ^ . / b */     
      ASTNode_addChild(help_1, copyAST(ASTNode_getChild(f,1)));      
      /* a ^ 1 / b */     
      help_2 = ASTNode_getChild(help_1, 0);
      ASTNode_setReal (help_2, 1.0);

      fprime = differentiateAST(helper, x);
      ASTNode_free(helper);
      break;
    case AST_FUNCTION_SEC:
      /** f(x)=sec(a(x)) => f' = a' * sec(a) * tan(x) */
      /* a' * ... */
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /* sec(a) * tan(x) */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_TIMES);
      /* sec(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_SEC);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /* tan(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,1), AST_FUNCTION_TAN);
      ASTNode_addChild(child2(fprime,1,1),copyAST(ASTNode_getChild(f,0)));
      break;
    case AST_FUNCTION_SECH:
      /** f(x)=sech(a(x)) f' = - a' * sech(a) * tanh(a)  */
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_MINUS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       differentiateAST(ASTNode_getChild(f,0),x));
      /* sec(a) * tan(x) */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_TIMES);
      /* sec(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_SECH);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /* tan(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,1), AST_FUNCTION_TANH);
      ASTNode_addChild(child2(fprime,1,1), copyAST(ASTNode_getChild(f,0)));    
      break;
    case AST_FUNCTION_SIN:
      /** f(x)=sin(a(x)) => f' = a' * cos(a) */
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_FUNCTION_COS);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       copyAST(ASTNode_getChild(f,0)));      
      ASTNode_addChild(fprime,differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_SINH:
      /** f(x)=sinh(a(x)) => f' = a' * cosh(a) */    
      ASTNode_setType(fprime,AST_TIMES);
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,0), AST_FUNCTION_COSH);
      ASTNode_addChild(ASTNode_getChild(fprime,0),
		       copyAST(ASTNode_getChild(f,0)));      
      ASTNode_addChild(fprime,differentiateAST(ASTNode_getChild(f,0),x));
      break;
    case AST_FUNCTION_TAN:
      /** f(x)= tan(a(x)) => f' = a' / (cos(a))^2  */
      ASTNode_setType(fprime,AST_DIVIDE);
      /* a' */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /* (cos(a))^2 */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_POWER);
      /* cos(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_COS);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /* ^2 */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,1), 2);        
      break;
    case AST_FUNCTION_TANH:
      /** f(x)= tanh(a(x)) => f' = a' / (cosh(a))^2  */
      ASTNode_setType(fprime,AST_DIVIDE);
      /* a' */
      ASTNode_addChild(fprime, differentiateAST(ASTNode_getChild(f,0),x));
      /* (cos(a))^2 */
      ASTNode_addChild(fprime, ASTNode_create());
      ASTNode_setType(ASTNode_getChild(fprime,1), AST_POWER);
      /* cos(a) */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setType(child2(fprime,1,0), AST_FUNCTION_COSH);
      ASTNode_addChild(child2(fprime,1,0), copyAST(ASTNode_getChild(f,0)));
      /* ^2 */
      ASTNode_addChild(ASTNode_getChild(fprime,1), ASTNode_create());
      ASTNode_setInteger(child2(fprime,1,1), 2); 
      break;
    default:
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_AST_UNKNOWN_FAILURE,
                        "differentiateAST: unknown failure for function type");
      ASTNode_setName(fprime, "differentiation_failed");
      break;
    }
  }
  else if ( ASTNode_isLogical(f) || ASTNode_isRelational(f) )
  {
    SolverError_error(WARNING_ERROR_TYPE,
		      SOLVER_ERROR_AST_DIFFERENTIATION_FAILED_LOGICAL_OR_RELATIONAL,
		      "differentiateAST: logical and relational not possible");
    ASTNode_setName(fprime, "differentiation_failed");
  }
  else if ( ASTNode_isUnknown(fprime) )
  {
    SolverError_error(WARNING_ERROR_TYPE,
		      SOLVER_ERROR_AST_UNKNOWN_NODE_TYPE,
		      "differentiateAST: unknown ASTNode type");
    ASTNode_setName(fprime, "differentiation_failed");
  }

  simple = simplifyAST(fprime);
  ASTNode_free(fprime);
  return simple;



}

/* ------------------------------------------------------------------------ */

/**
   !! experimental function determinantNAST !! NOT TESTED !!
   !! WILL RUN OUT OF MEMORY FOR BIG SYSTEMS !!
   calculates the determinant of the jacobian matrix, is not
   used and can only be activated with hidden
   commandline option -d.
   Doesn't work if expressions get too big for yet unknown
   reasons!??
*/


SBML_ODESOLVER_API ASTNode_t *determinantNAST(ASTNode_t ***A, int N)
{
  int k, i, j, l, check;
  ASTNode_t ***B;
  ASTNode_t *det, *tmp, *tmp2, *simple;
  
  det = NULL;
  B = NULL;
  tmp = NULL;

  if ( N == 1 ) 
    return copyAST(A[0][0]);    

  det = ASTNode_create();

  /* for i = 1 to N */
  for ( k=0; k<N; k++ )
  {    
    check = 0;
    
    if ( ASTNode_isInteger(A[k][0]) )
      if ( ASTNode_getInteger(A[k][0]) == 0 )
	check = 1;

    if (  ASTNode_isReal(A[k][0]) ) 
      if ( ASTNode_getReal(A[k][0]) == 0.0 ) 
	check = 1;    
        
    if ( check < 1 )
    { 
      /* B = A, with row i and column 1 deleted */
      B = (ASTNode_t ***) calloc(N-1, sizeof(ASTNode_t **));
      l = 0;
      for ( i=0; i<N-1; i++ )
      {
	B[i] = (ASTNode_t **) calloc(N-1, sizeof(ASTNode_t *));
	
	if ( k == i ) 
	  l++;
	
	for ( j=0; j<N-1; j++ ) 
	  B[i][j] = copyAST(A[l][j+1]); /* ... or A[i+1][l] ?? */
	l++;
      }

      /*  det = det + (-1)^(i-1)*a(i1) * det(B); but k from 0 to N-1 */

      tmp = ASTNode_create();
      /* ... * ... */
      ASTNode_setType(tmp, AST_TIMES);
      /* (-1)^(i-1)*a(i1) */
      if ( k%2 != 0 )
      {
	ASTNode_addChild(tmp, ASTNode_create());
	ASTNode_setType(ASTNode_getChild(tmp, 0), AST_MINUS);
	ASTNode_addChild(ASTNode_getChild(tmp, 0), copyAST(A[k][0]));
      }
      else 
	ASTNode_addChild(tmp, copyAST(A[k][0]));
      
      /* det(B) */ 
      ASTNode_addChild(tmp, determinantNAST(B, N-1));

      /* det + ... */
      if ( det == NULL )
      {
	det = copyAST(tmp);
	ASTNode_free(tmp);
      }
      else
      {
	tmp2 = ASTNode_create();
	ASTNode_setType(tmp2, AST_PLUS);
	ASTNode_addChild(tmp2, copyAST(det));
	ASTNode_addChild(tmp2, copyAST(tmp));
	ASTNode_free(tmp);
	ASTNode_free(det);
	det = ASTNode_create();
	det = copyAST(tmp2);
	ASTNode_free(tmp2);
      }

      
      for ( i=0; i<N-1; i++ ) {
	for ( j=0; j<N-1; j++ ) 
	  ASTNode_free(B[i][j]);	
	free(B[i]);
      }
      free(B);

      /* printf(" +(-1)^(%d-1)*a(%d1)*det(%d) ", k+1, k+1, N-1); */
      
    } 
  }

  simple =  simplifyAST(det);
  ASTNode_free(det);
  return simple;
  
}

/** @} */

/* ------------------------------------------------------------------------ */

/*! \addtogroup simplifyAST */
/*@{*/

/** takes an ASTNode and odeModel and returns a corresponding indexed ASTNode,

    The function converts AST_NAME to AST_IndexName, which holds the
    name of the variable and additionally the index used in odeModel,
    integratorInstance and cvodeData */
SBML_ODESOLVER_API ASTNode_t *ASTNode_indexAST(const ASTNode_t *f, odeModel_t *om)
{
  return indexAST(f, om->neq+om->nass+om->nconst+om->nalg, om->names);
}

/** takes an ASTNode and a string array `names' and returns
    a corresponding indexed ASTNode.

    The function converts AST_NAME to AST_IndexName, which holds the
    name of the variable and additionally the index of the variable
    in the passed array `names' */
ASTNode_t *indexAST(const ASTNode_t *f, int nvalues, char **names)
{
  int i, found;
  unsigned int k;
  ASTNode_t *index;

  const char *str;
  char *short_str = NULL;

  index = ASTNode_create();

  /* DISTINCTION OF CASES */

  /* integers, reals */
  if ( ASTNode_isInteger(f) ) 
    ASTNode_setInteger(index, ASTNode_getInteger(f));  
  else if ( ASTNode_isReal(f) ) 
    ASTNode_setReal(index, ASTNode_getReal(f));
  
  /* copy existing indexes */
  /*   else if ( ASTNode_isSetIndex(f) ) { */
  /*     ASTNode_free(index); */
  /*     index = ASTNode_createIndexName(); */
  /*     ASTNode_setName(index, ASTNode_getName(f)); */
  /*     ASTNode_setIndex(index, ASTNode_getIndex(f));     */
  /*   } */
  
  /* writing indexed name nodes */
  else if ( ASTNode_isName(f) )
  {
    found = 0;
    str = ASTNode_getName(f);
    /* alloc mem for (experimental) data variable */
    if ( strstr(str, "_data") != NULL )
    {
      /* short_str = space((strlen(str)-5+1) * sizeof(char)); */
      ASSIGN_NEW_MEMORY_BLOCK(short_str, strlen(str)-5+1, char, 0);
      strncpy(short_str, str, strlen(str)-5);
    }
    for ( i=0; i<nvalues; i++ )
    {
      if ( strcmp(str, names[i]) == 0 )
      {
        ASTNode_free(index);
	index = ASTNode_createIndexName(); 
	ASTNode_setName(index, str);
	ASTNode_setIndex(index, i);
	found++;
	break;
      }
      else if ( short_str != NULL && strcmp(short_str, names[i]) == 0 )
      {
        ASTNode_free(index);
	index = ASTNode_createIndexName(); 
	ASTNode_setName(index, short_str);
	ASTNode_setIndex(index, i);
	ASTNode_setData(index);
	found++;
  
	break;
      }
    }
    if ( !found )
      ASTNode_setName(index, str);
    /* free mem */
    if ( short_str != NULL )
      free(short_str);
    
    /* time and delay nodes */
    ASTNode_setType(index, ASTNode_getType(f)); 
  }
  /* constants */
  /* functions, operators */
  else
  {
    ASTNode_setType(index, ASTNode_getType(f));
    /* user-defined functions: name must be set */
    if ( ASTNode_getType(f) == AST_FUNCTION ) 
      ASTNode_setName(index, ASTNode_getName(f));    
    for ( k=0; k<ASTNode_getNumChildren(f); k++ )
      ASTNode_addChild(index, indexAST(ASTNode_getChild(f,k), nvalues, names));
  }
  
  return index;
}

/** Takes an AST f, and returns a simplified copy of f.

decomposes n-ary `times' and `plus' nodes into an AST of binary AST. 
   
simplifies (arithmetic) operations involving 0 and 1: \n   
-0 -> 0;\n
x+0 -> x, 0+x -> x;\n
x-0 -> x, 0-x -> -x;\n
x*0 -> 0, 0*x -> 0, x*1 -> x, 1*x -> x;\n
0/x -> 0, x/1 -> x;\n
x^0 -> 1, x^1 -> x, 0^x -> 0, 1^x -> 1;\n
   
propagates unary minuses\n
--x -> x; \n
-x + -y -> -(x+y), -x + y -> y-x,    x + -y -> x-y; \n
-x - -y -> y-x,    -x - y -> -(x+y), x - -y -> x+y; \n 
-x * -y -> x*y,    -x * y -> -(x*y), x * -y -> -(x*y);\n
-x / -y -> x/y,    -x / y -> -(x/y), x / -y -> -(x/y); \n
   
calls evaluateAST(subtree), if no variables or user-defined
functions occur in the AST subtree,

calls itself recursively for childnodes,
   
   
*/

SBML_ODESOLVER_API ASTNode_t *simplifyAST(const ASTNode_t *f)
{  
  unsigned int i, childnum;
  int simplify;
  ASTNode_t *simple, *left, *right, *helper;
  ASTNodeType_t type;

  /* new ASTNode */
  simple = ASTNode_create();
  
  type = ASTNode_getType(f);

  /* DISTINCTION OF CASES */

  /* integers, reals */
  if ( ASTNode_isInteger(f) ) 
    ASTNode_setInteger(simple, ASTNode_getInteger(f));
  else if ( ASTNode_isReal(f) ) 
    ASTNode_setReal(simple, ASTNode_getReal(f));
  /* variables */
  else if ( ASTNode_isName(f) )
  {
    if ( ASTNode_isSetIndex(f) )
    {
      ASTNode_free(simple);
      simple = ASTNode_createIndexName();
      ASTNode_setIndex(simple, ASTNode_getIndex(f));

      if ( ASTNode_isSetData(f) )
	 ASTNode_setData(simple);
    } 
    ASTNode_setName(simple, ASTNode_getName(f));
    ASTNode_setType(simple, ASTNode_getType(f));
  }
  /* --------------- operators with possible simplifications -------------- */
  /* special operator: unary minus */
  else if ( ASTNode_isUMinus(f) )
  {
    left = simplifyAST(ASTNode_getLeftChild(f));
    if ( zero(left) )         /* -0 = 0 */
    {
      ASTNode_free(simple);
      simple = left;
    }
    else if ( ASTNode_isUMinus(left) )   /* - -x */
    {
      ASTNode_free(simple);
      simple = ASTNode_cutRoot(left);
    }
    else  /* no simplification */
    {
      ASTNode_setType (simple, AST_MINUS);
      ASTNode_addChild(simple, left);
    }
  }
  /* general operators */
  else if ( ASTNode_isOperator(f) || type==AST_FUNCTION_POWER)
  {
    childnum = ASTNode_getNumChildren(f);  
    /* zero operands: set to neutral element */
    if ( childnum == 0 )
    {
      if ( type == AST_PLUS ) 
	ASTNode_setInteger(simple, 0);
      else if ( type == AST_TIMES )
	ASTNode_setInteger(simple, 1);
    }
    /* one operand: set node to operand */
    else if ( childnum == 1 )
    {
      ASTNode_free(simple);
      if ( type == AST_PLUS ) 
	simple = simplifyAST(ASTNode_getChild(f, 0));
      else if ( type == AST_TIMES )
	simple = simplifyAST(ASTNode_getChild(f, 0));
    }
    /* >2 operands: recursively decompose
       into tree with 2 operands */
    else if ( childnum > 2 )
    {
      if ( type == AST_PLUS ) 
	ASTNode_setType(simple, AST_PLUS);		  
      else if ( type == AST_TIMES ) 
	ASTNode_setType(simple, AST_TIMES);
      /* copy/simplify left child ... */
      ASTNode_addChild(simple, simplifyAST(ASTNode_getChild(f,0)));
      /* ... and move other child down */
      helper = ASTNode_create();
      ASTNode_setType(helper, type);
      for ( i=1; i<childnum; i++ )
	ASTNode_addChild(helper, simplifyAST(ASTNode_getChild(f,i)));
      ASTNode_addChild(simple, simplifyAST(helper));
      ASTNode_free(helper);
    }
    /* 2 operands: remove 0s and 1s and unary minuses */
    else
    {    
      left  = simplifyAST(ASTNode_getLeftChild(f));
      right = simplifyAST(ASTNode_getRightChild(f));
      /* default: simplification */
      simplify = 1; /* set flag */
      switch(type)
      {
	/* binary plus x + y */
      case AST_PLUS:
	if ( zero(right) )       /* x+0 = x */
	{
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( zero(left) )   /* 0+x = x */
	{
	  ASTNode_free(left);
	  ASTNode_free(simple);
	  simple = right;
	}
	else if ( ASTNode_isUMinus(left) && ASTNode_isUMinus(right) )
	{
	  /* -x + -y */
	  ASTNode_setType (simple, AST_MINUS);  
	  ASTNode_addChild(simple, ASTNode_create());
	  helper = ASTNode_getChild(simple, 0);
	  ASTNode_setType (helper, AST_PLUS);
	  ASTNode_addChild(helper, ASTNode_cutRoot(left));
	  ASTNode_addChild(helper, ASTNode_cutRoot(right));
	}
	else if ( ASTNode_isUMinus(left) )
	{
	  /* -x + y */
	  ASTNode_setType (simple, AST_MINUS);
	  ASTNode_addChild(simple, right);
	  ASTNode_addChild(simple, ASTNode_cutRoot(left));
	}
	else if ( ASTNode_isUMinus(right) )
	{
	  /* x + -y */
	  ASTNode_setType (simple, AST_MINUS);
	  ASTNode_addChild(simple, left);
	  ASTNode_addChild(simple, ASTNode_cutRoot(right));
	}
	else 
	  simplify = 0;	
	break;
	/* binary minus x - y */
      case AST_MINUS:
	if ( zero(right) )
	{
	  /* x-0 = x */
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( zero(left) )
	{
	  /* 0-x =-x */
	  ASTNode_free(left);
	  ASTNode_setType (simple, type);
	  ASTNode_addChild(simple, right);
	}
	else if ( ASTNode_isUMinus(left) && ASTNode_isUMinus(right) )
	{
	  /* -x - -y */
	  ASTNode_setType (simple, AST_MINUS);
	  ASTNode_addChild(simple, ASTNode_cutRoot(right));
	  ASTNode_addChild(simple, ASTNode_cutRoot(left));
	}
	else if ( ASTNode_isUMinus(left) )
	{
	  /* -x - y */
	  ASTNode_setType (simple, AST_MINUS); 
	  ASTNode_addChild(simple, ASTNode_create());
	  helper = ASTNode_getChild(simple, 0);
	  ASTNode_setType (helper, AST_PLUS);
	  ASTNode_addChild(helper, ASTNode_cutRoot(left));
	  ASTNode_addChild(helper, right);
	}
	else if ( ASTNode_isUMinus(right) )
	{
	  /* x - -y */
	  ASTNode_setType (simple, AST_PLUS);
	  ASTNode_addChild(simple, left);
	  ASTNode_addChild(simple, ASTNode_cutRoot(right));
	}
	else 
	  simplify = 0;	
	break;
	/* binary times x * y */
      case AST_TIMES:
	if ( zero(right) )
	{
	  /* x*0 = 0 */
	  ASTNode_free(left);
	  ASTNode_free(simple);
	  simple = right;
	}
	else if ( zero(left) )
	{
	  /* 0*x = 0 */
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( one(right) )
	{
	  /* x*1 = x */
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( one(left) )
	{
	  /* 1*x = x */
	  ASTNode_free(left);
	  ASTNode_free(simple);
	  simple = right;
	}
	else if ( ASTNode_isUMinus(left) && ASTNode_isUMinus(right) )
	{
	  /* -x * -y */
	  ASTNode_setType (simple, AST_TIMES);  
	  ASTNode_addChild(simple, ASTNode_cutRoot(left));
	  ASTNode_addChild(simple, ASTNode_cutRoot(right));
	}
	else if ( ASTNode_isUMinus(left) )
	{
	  /* -x * y */
	  ASTNode_setType (simple, AST_MINUS); 
	  ASTNode_addChild(simple, ASTNode_create());
	  helper = ASTNode_getChild(simple, 0);
	  ASTNode_setType (helper, AST_TIMES);
	  ASTNode_addChild(helper, ASTNode_cutRoot(left));
	  ASTNode_addChild(helper, right);
	}
	else if ( ASTNode_isUMinus(right) )
	{
	  /* x * -y */
	  ASTNode_setType (simple, AST_MINUS); 
	  ASTNode_addChild(simple, ASTNode_create());
	  helper = ASTNode_getChild(simple, 0);
	  ASTNode_setType (helper, AST_TIMES);
	  ASTNode_addChild(helper, left);
	  ASTNode_addChild(helper, ASTNode_cutRoot(right));
	}
	else 
	  simplify = 0;
	break;
	/* binary divide x / y */
      case AST_DIVIDE:
	if ( zero(left) )
	{
	  /* 0/x = 0 */
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( one(right) )
	{
	  /* x/1 = x */
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( ASTNode_isUMinus(left) && ASTNode_isUMinus(right) )
	{
	  /* -x / -y */
	  ASTNode_setType (simple, AST_DIVIDE); 
	  ASTNode_addChild(simple, ASTNode_cutRoot(left));
	  ASTNode_addChild(simple, ASTNode_cutRoot(right));
	}
	else if ( ASTNode_isUMinus(left) )
	{
	  /* -x / y */
	  ASTNode_setType (simple, AST_MINUS); 
	  ASTNode_addChild(simple, ASTNode_create());
	  helper = ASTNode_getChild(simple, 0);
	  ASTNode_setType (helper, AST_DIVIDE);
	  ASTNode_addChild(helper, ASTNode_cutRoot(left));
	  ASTNode_addChild(helper, right);
	}
	else if ( ASTNode_isUMinus(right) )
	{
	  /* x / -y */
	  ASTNode_setType (simple, AST_MINUS); 
	  ASTNode_addChild(simple, ASTNode_create());
	  helper = ASTNode_getChild(simple, 0);
	  ASTNode_setType (helper, AST_DIVIDE);
	  ASTNode_addChild(helper, left);
	  ASTNode_addChild(helper, ASTNode_cutRoot(right));
	}
	else
	  simplify = 0;
	break;
	/* power x^y */
      case AST_POWER:
      case AST_FUNCTION_POWER:
	if ( zero(right) )
	{
	  /* x^0 = 1 */
	  ASTNode_free(left);
	  ASTNode_free(right);
	  ASTNode_setReal(simple, 1.0);      
	}
	else if ( one(right) )
	{
	  /* x^1 = x */
	  ASTNode_free(right);
	  ASTNode_free(simple);
	  simple = left;
	}
	else if ( zero(left) )
	{
	  /* 0^x = 0 */
	  ASTNode_free(left);
	  ASTNode_free(right);
	  ASTNode_setReal(simple, 0.0);      
	}
	else if ( one(left) )
	{
	  /* 1^x = 1 */
	  ASTNode_free(left);
	  ASTNode_free(right);
	  ASTNode_setReal(simple, 1.0);      
	}
	else 
	  simplify = 0;
	break;    
      default:
	SolverError_error(WARNING_ERROR_TYPE,
			  SOLVER_ERROR_AST_UNKNOWN_FAILURE,
			  "simplifyAST: unknown failure for operator type");
	break;
      }
      /* after all no simplification */
      if (!simplify)
      {

	ASTNode_setType (simple, type);
	ASTNode_addChild(simple, left);
	ASTNode_addChild(simple, right);
      }
    }
  }
  /* -------------------- cases with no simplifications ------------------- */
  /* constants (leaves) */ 
  /* functions, operators (branches) */
  else
  {
    ASTNode_setType(simple, type);

   /*  if( ASTNode_isSetData(f) ) */
/*       ASTNode_setData(simple); */

    /* user-defined functions: name must be set*/
    if ( ASTNode_getType(f) == AST_FUNCTION ) 
      ASTNode_setName(simple, ASTNode_getName(f));
    for ( i=0; i<ASTNode_getNumChildren(f); i++ ) 
      ASTNode_addChild(simple, simplifyAST(ASTNode_getChild(f,i)));
  }
  
  return (simple);
}


/** @} */


/* ------------------------------------------------------------------------ */

/* logical predicate that checks if ASTNode is a user-defined variable name,
   (or time), or a user defined function */
static int user_defined(ASTNode_t *node)
{
  if ( ASTNode_isName(node) || ASTNode_getType(node)==AST_FUNCTION )
    return 1;
  return 0;
}


/* ------------------------------------------------------------------------ */

static int zero(ASTNode_t *f)
{
  if ( ASTNode_isReal(f) ) 
    return (ASTNode_getReal(f)==0.0);
  if ( ASTNode_isInteger(f) ) 
    return (ASTNode_getInteger(f)==0);
  return 0;
}

/* ------------------------------------------------------------------------ */

static int one(ASTNode_t *f)
{
  if ( ASTNode_isReal(f) ) 
    return (ASTNode_getReal(f)==1.0);
  if ( ASTNode_isInteger(f) ) 
    return (ASTNode_getInteger(f)==1);
  return 0;
}

/* ------------------------------------------------------------------------ */

static ASTNode_t *ASTNode_cutRoot(ASTNode_t *old)
{
  ASTNode_t *new;
  new = copyAST(ASTNode_getChild(old, 0));
  ASTNode_free(old);
  return new;
}

/* appends the indices in the given indexed AST to the given list. */
int ASTNode_getIndices(const ASTNode_t *node, List_t *indices)
{
  unsigned int i;

  if ( ASTNode_isSetIndex(node) )
  {
    unsigned int *idx;
    ASSIGN_NEW_MEMORY(idx, unsigned int, 0);
    *idx = ASTNode_getIndex(node);
    List_add(indices, idx);
  }

  for ( i=0; i<ASTNode_getNumChildren(node); i++ )
    ASTNode_getIndices(ASTNode_getChild(node, i), indices);

  return 1;
}
/* generates a boolean vector of size nvalues, indicating whether
   an index occurs in the given indexed AST */
int *ASTNode_getIndexArray(const ASTNode_t *node, int nvalues)
{
  int *result;
  List_t *indices;

  ASSIGN_NEW_MEMORY_BLOCK(result, nvalues, int, NULL);
  if ( !node ) return result;
  indices = List_create();
    /* get indices from equation */
    ASTNode_getIndices(node, indices);
    
    /* set indices to 1 and free list items */
    while ( List_size(indices) )
    {
      unsigned int *k;
      k = (unsigned int *)List_remove(indices, 0);
      result[*k] = 1;
      free(k);
    }
  List_free(indices);

  return result;
}

/* returns boolean result: whether the given AST contains a time symbol. */
int ASTNode_containsTime(const ASTNode_t *node)
{
  unsigned int i;

  if ( ASTNode_getType(node) == AST_NAME_TIME /* || */
/*        (ASTNode_getType(node) == AST_NAME && */
/* 	(strcmp(ASTNode_getName(node),"time") == 0 || */
/* 	 strcmp(ASTNode_getName(node),"Time") == 0 || */
/* 	 strcmp(ASTNode_getName(node),"TIME") == 0)) */ )
    return 1;

  for ( i = 0; i != ASTNode_getNumChildren(node); i++ )
    if ( ASTNode_containsTime(ASTNode_getChild(node, i)) )
      return 1;

  return 0;
}

/* returns boolean result: whether the given AST contains a time symbol. */
int ASTNode_containsPiecewise(const ASTNode_t *node)
{
  unsigned int i;

  if ( ASTNode_getType(node) == AST_FUNCTION_PIECEWISE )
    return 1;

  for ( i = 0; i != ASTNode_getNumChildren(node); i++ )
    if ( ASTNode_containsPiecewise(ASTNode_getChild(node, i)) )
      return 1;

  return 0;
}



/* ------------------------------------------------------------------------ */


/* appends the given AST in compilable form to the given buffer.
   The form is enclosed in brackets when necessary so that the AST
   can be incorporated as a sub expression of another expression. */
static void ASTNode_generateNestedExpression(charBuffer_t *expressionStream,
				      const ASTNode_t *node)
{
  switch ( ASTNode_getType(node) )
  {
    /* expressions that don't need to be bracketed */
  case AST_INTEGER :
  case AST_REAL :
  case AST_REAL_E :
  case AST_RATIONAL :
  case AST_NAME :
  case AST_NAME_TIME :
  case AST_CONSTANT_E :
  case AST_CONSTANT_FALSE :
  case AST_CONSTANT_PI :
  case AST_CONSTANT_TRUE :
  case AST_FUNCTION_ABS :
  case AST_FUNCTION_ARCCOS :
  case AST_FUNCTION_ARCCOSH :
  case AST_FUNCTION_ARCCOT :
  case AST_FUNCTION_ARCCOTH :
  case AST_FUNCTION_ARCCSC :
  case AST_FUNCTION_ARCCSCH :
  case AST_FUNCTION_ARCSEC :
  case AST_FUNCTION_ARCSECH :
  case AST_FUNCTION_ARCSIN :
  case AST_FUNCTION_ARCSINH :
  case AST_FUNCTION_ARCTAN :
  case AST_FUNCTION_ARCTANH :
  case AST_FUNCTION_CEILING :
  case AST_FUNCTION_COS :
  case AST_FUNCTION_COSH :
  case AST_FUNCTION_COT :
  case AST_FUNCTION_COTH :
  case AST_FUNCTION_CSC :
  case AST_FUNCTION_CSCH :
  case AST_FUNCTION_EXP :
  case AST_FUNCTION_FACTORIAL :
  case AST_FUNCTION_FLOOR :
  case AST_FUNCTION_LN :
  case AST_FUNCTION_LOG :
  case AST_FUNCTION_ROOT :
  case AST_FUNCTION_SEC :
  case AST_FUNCTION_SECH :
  case AST_FUNCTION_SIN :
  case AST_FUNCTION_SINH :
  case AST_FUNCTION_TAN :
  case AST_FUNCTION_TANH :
  case AST_LOGICAL_XOR :
  case AST_FUNCTION :
    generateAST(expressionStream, node);
    break;

    /* expressions that do */
  default :
    CharBuffer_append(expressionStream, "(") ;
    generateAST(expressionStream, node);
    CharBuffer_append(expressionStream, ")");
    break;
  }
}

/* appends the given node to the given buffer in compilable form assuming
   the node is a unary operator.
   'op' is the compilable operator string for the node. */
static void ASTNode_generateUnaryOperator(charBuffer_t *expressionStream,
				   const ASTNode_t *node, const char *op)
{
  CharBuffer_append(expressionStream,op); ;
  ASTNode_generateNestedExpression(expressionStream,
				   ASTNode_getChild(node, 0));
}

/* appends the given node to the given buffer in compilable form assuming
   the node is a Nary operator.
   'op' is the compilable operator string for the node. */
static void ASTNode_generateNaryOperator(charBuffer_t *expressionStream,
				  const ASTNode_t *node, const char *op)
{
  unsigned int i;
    
  for ( i = 0 ; i != ASTNode_getNumChildren(node); i++ )
  {
    ASTNode_generateNestedExpression(expressionStream,
				     ASTNode_getChild(node, i));
    if ( i != ASTNode_getNumChildren(node) - 1 )
    {
      CharBuffer_append(expressionStream, " ");
      CharBuffer_append(expressionStream, op);
      CharBuffer_append(expressionStream, " ");
    }
  }
}

/* appends the given node to the given buffer in compilable form assuming
   the node is a function.
   'func' is the compilable function string for the node. */
static void ASTNode_generateFunctionCall(charBuffer_t *expressionStream,
				  const ASTNode_t *node, const char *func)
{
  unsigned int i;

  CharBuffer_append(expressionStream, func);
  CharBuffer_append(expressionStream, "(");
  for ( i = 0 ; i != ASTNode_getNumChildren(node); i++ )
  {
    generateAST(expressionStream, ASTNode_getChild(node, i));
    if ( i != ASTNode_getNumChildren(node) - 1 )
      CharBuffer_append(expressionStream, ", ") ;
  }
  CharBuffer_append(expressionStream, ")") ;   
}

/* appends compilable code to represent the given AST_Name node to the
   give buffer.  The code consists of a reference to an item in the
   array 'value' indexed by the the index associated with the node by
   the function 'indexAST'.  If the ASTNode doesn't have an index
   value then an error is created and '0' is appended to the buffer. */
static void ASTNode_generateName(charBuffer_t *expressionStream, const ASTNode_t *n)
{
  int found = 0;

  if ( ASTNode_isSetIndex((ASTNode_t *)n) )
  {
    if ( ASTNode_isSetData((ASTNode_t *)n) )
    {
      SolverError_error(FATAL_ERROR_TYPE,
			SOLVER_ERROR_AST_COMPILATION_FAILED_DATA_AST_NODE_NOT_SUPPORTED_YET,
			"Compilation process ignoring data state on name ",
			"node %s - Not supported yet\n",
			ASTNode_getName(n));
    }
        
    /* else */
    {
      CharBuffer_append(expressionStream, "value[");
      CharBuffer_appendInt(expressionStream, ASTNode_getIndex((ASTNode_t *)n));
      CharBuffer_append(expressionStream, "]");
    }

    found++;
  }

  /* this is what we'd do if the index isn't always set -
     this should not be neccessary IMHO - AMF 
     if ( found == 0 ) {
     for ( j=0; j<data->nvalues; j++ ) {
     if ( (strcmp(ASTNode_getName(n),data->model->names[j]) == 0) )
     {
     CharBuffer_append(expressionStream, "value[");
     CharBuffer_appendInt(expressionStream, j);
     CharBuffer_append(expressionStream, "]");
     found++;
     }
     }
     }
  */

  if ( found == 0 )
  {
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_AST_COMPILATION_FAILED_MISSING_VALUE,
		      "ASTNode_generateName: "
		      "No value found for AST_NAME %s. Defaults to Zero "
		      "to avoid program crash", ASTNode_getName(n));
    CharBuffer_append(expressionStream, "0.0");
  }
}

/** appends compilable macros and functions to buffer to enable the code
    in buffer to support the code generated by the 'generateAST' function. */
SBML_ODESOLVER_API void generateMacros(charBuffer_t *buffer)
{
  /* was using
     "#define asech(x) log((1.0 + sqrt(1.0 - (x)*(x))) / (x))\n"\ */

  CharBuffer_append(buffer, 
		    "#define acot(x) atan(1.0/(x))\n"\
		    "#define acoth(x) (0.5*log(((x)+1.0)/((x)-1.0)))\n"\
		    "#define acsc(x) atan(1.0/sqrt(((x)-1.0)*((x)+1.0)))\n"\
		    "#define acsch(x) log((1.0+sqrt(1.0 + (x)*(x)))/(x))\n"\
		    "#define asec(x) atan(sqrt(((x) - 1.0)*((x) + 1.0)))\n"\
		    "#define asech(x) acosh(1.0/(x))\n"\
		    "#define cot(x) (1.0 / tan(x))\n"\
		    "#define coth(x) (cosh(x) / sinh(x))\n"\
		    "#define csch(x) (1.0/sinh(x))\n"\
		    "#define MyLog(x,y) (log10(y)/log10(x))\n"\
		    /*!!! account for piecewise with more then 3 children!*/
		    "#define piecewise(x, y, z) ((y) ? (x) : (z))\n"\
		    /*!!! account for odd root degrees of negative values!*/
		    "#define root(x, y) pow(y, 1.0 / (x))\n"\
		    "#define sec(x) (1.0/cos(x))\n"\
		    "#define sech(x) (1.0/cosh(x))\n"\
		    "#define acosh(x) (log((x) + (sqrt((x) - 1.0) * sqrt((x) + 1.0))))\n"\
		    "#define asinh(x) (log((x) + sqrt(((x) * (x)) + 1.0)))\n"\
		    "#define atanh(x) ((log(1.0 + (x)) - log(1.0-(x)))/2.0)\n"\
		    "#define csc(x) (1.0/sin(x))\n"\
		    "\n"\
		    "double factorial(double x)\n"\
		    "{\n"\
		    "    double result ;\n"\
		    "    int j = floor(x);\n"\
		    "    for(result=1;j>1;--j)\n"\
		    "        result *= j;\n"\
		    "    return result;\n"\
		    "}\n");
}

/* appends compilable code to the given buffer for the given AST assuming
   the AST is an XOR expression. */
static void ASTNode_generateXOR(charBuffer_t *expressionStream, const ASTNode_t *node)
{
  unsigned int i;
    
  CharBuffer_append(expressionStream, "((");

  for ( i = 0 ; i != ASTNode_getNumChildren(node); i++ )
  {
    CharBuffer_append(expressionStream, "(");
    ASTNode_generateNestedExpression(expressionStream,
				     ASTNode_getChild(node, i));
    CharBuffer_append(expressionStream, " ? 1 : 0)");
    if ( i != ASTNode_getNumChildren(node) - 1 )
      CharBuffer_append(expressionStream, " + ");
  }

  CharBuffer_append(expressionStream, ") % 2) != 0");
}

/* appends compilable code to the given buffer that
   implements the given AST. */
SBML_ODESOLVER_API void generateAST(charBuffer_t *expressionStream, const ASTNode_t *node)
{
  switch (ASTNode_getType(node))
  {
  case AST_PLUS :
    ASTNode_generateNaryOperator(expressionStream, node, "+");
    break;
  case AST_TIMES :
    ASTNode_generateNaryOperator(expressionStream, node, "*");
    break;
  case AST_MINUS :
    if (ASTNode_getNumChildren(node) == 1)
      ASTNode_generateUnaryOperator(expressionStream, node, "-");
    else
      ASTNode_generateNaryOperator(expressionStream, node, "-");
    break;
  case AST_DIVIDE : 
    ASTNode_generateNaryOperator(expressionStream, node, "/");
    break;
  case AST_POWER :
    ASTNode_generateFunctionCall(expressionStream, node, "pow");
    break;
  case AST_INTEGER :
    CharBuffer_append(expressionStream, "((realtype)");
    CharBuffer_appendInt(expressionStream, ASTNode_getInteger(node));
    CharBuffer_append(expressionStream, ")");
    break;
  case AST_REAL :
  case AST_REAL_E :
  case AST_RATIONAL :
    CharBuffer_append(expressionStream, "((realtype)");
    CharBuffer_appendDouble(expressionStream, ASTNode_getReal(node));
    CharBuffer_append(expressionStream, ")");
    break;
  case AST_NAME :
    ASTNode_generateName(expressionStream, node);
    break;
  case AST_NAME_TIME :
    CharBuffer_append(expressionStream, "data->currenttime");
    break;
  case AST_CONSTANT_E:
    /** exp(1) is used to adjust exponentiale to machine precision */
    CharBuffer_appendDouble(expressionStream, exp(1));
    break;
  case AST_CONSTANT_FALSE:
    CharBuffer_appendDouble(expressionStream, 0.0);
    break;
  case AST_CONSTANT_PI:
    /** pi = 4 * atan 1  is used to adjust Pi to machine precision */
    CharBuffer_appendDouble(expressionStream, 4.*atan(1.));
    break;
  case AST_CONSTANT_TRUE:
    CharBuffer_appendDouble(expressionStream, 1.0);
    break;
  case AST_FUNCTION_ABS :
    ASTNode_generateFunctionCall(expressionStream, node, "fabs");
    break;
  case AST_FUNCTION_ARCCOS :
    ASTNode_generateFunctionCall(expressionStream, node, "acos");
    break;
  case AST_FUNCTION_ARCCOSH :
    ASTNode_generateFunctionCall(expressionStream, node, "acosh");
    break;
  case AST_FUNCTION_ARCCOT :
    ASTNode_generateFunctionCall(expressionStream, node, "acot");
    break;
  case AST_FUNCTION_ARCCOTH :
    ASTNode_generateFunctionCall(expressionStream, node, "acoth");
    break;
  case AST_FUNCTION_ARCCSC :
    ASTNode_generateFunctionCall(expressionStream, node, "acsc");
    break;
  case AST_FUNCTION_ARCCSCH :
    ASTNode_generateFunctionCall(expressionStream, node, "acsch");
    break;
  case AST_FUNCTION_ARCSEC :
    ASTNode_generateFunctionCall(expressionStream, node, "asec");
    break;
  case AST_FUNCTION_ARCSECH :
    ASTNode_generateFunctionCall(expressionStream, node, "asech");
    break;
  case AST_FUNCTION_ARCSIN :
    ASTNode_generateFunctionCall(expressionStream, node, "asin");
    break;
  case AST_FUNCTION_ARCSINH :
    ASTNode_generateFunctionCall(expressionStream, node, "asinh");
    break;
  case AST_FUNCTION_ARCTAN :
    ASTNode_generateFunctionCall(expressionStream, node, "atan");
    break;
  case AST_FUNCTION_ARCTANH :
    ASTNode_generateFunctionCall(expressionStream, node, "atanh");
    break;
  case AST_FUNCTION_CEILING :
    ASTNode_generateFunctionCall(expressionStream, node, "ceil");
    break;
  case AST_FUNCTION_COS :
    ASTNode_generateFunctionCall(expressionStream, node, "cos");
    break;
  case AST_FUNCTION_COSH :
    ASTNode_generateFunctionCall(expressionStream, node, "cosh");
    break;
  case AST_FUNCTION_COT :
    ASTNode_generateFunctionCall(expressionStream, node, "cot");
    break;
  case AST_FUNCTION_COTH :
    ASTNode_generateFunctionCall(expressionStream, node, "coth");
    break;
  case AST_FUNCTION_CSC :
    ASTNode_generateFunctionCall(expressionStream, node, "csc");
    break;
  case AST_FUNCTION_CSCH :
    ASTNode_generateFunctionCall(expressionStream, node, "csch");
    break;
  case AST_FUNCTION_EXP :
    ASTNode_generateFunctionCall(expressionStream, node, "exp");
    break;
  case AST_FUNCTION_FACTORIAL :
    ASTNode_generateFunctionCall(expressionStream, node, "factorial");
    break;
  case AST_FUNCTION_FLOOR :
    ASTNode_generateFunctionCall(expressionStream, node, "floor");
    break;
  case AST_FUNCTION_LN :
    ASTNode_generateFunctionCall(expressionStream, node, "log");
    break;
  case AST_FUNCTION_LOG :
    ASTNode_generateFunctionCall(expressionStream, node, "MyLog");
    break;
  case AST_FUNCTION_PIECEWISE :
    ASTNode_generateFunctionCall(expressionStream, node, "piecewise");
    break;
  case AST_FUNCTION_POWER :
    ASTNode_generateFunctionCall(expressionStream, node, "pow");
    break;
  case AST_FUNCTION_ROOT :
    ASTNode_generateFunctionCall(expressionStream, node, "root");
    break;
  case AST_FUNCTION_SEC :
    ASTNode_generateFunctionCall(expressionStream, node, "sec");
    break;
  case AST_FUNCTION_SECH :
    ASTNode_generateFunctionCall(expressionStream, node, "sech");
    break;
  case AST_FUNCTION_SIN :
    ASTNode_generateFunctionCall(expressionStream, node, "sin");
    break;
  case AST_FUNCTION_SINH :
    ASTNode_generateFunctionCall(expressionStream, node, "sinh");
    break;
  case AST_FUNCTION_TAN :
    ASTNode_generateFunctionCall(expressionStream, node, "tan");
    break;
  case AST_FUNCTION_TANH :
    ASTNode_generateFunctionCall(expressionStream, node, "tanh");
    break;
  case AST_LOGICAL_AND :
    ASTNode_generateNaryOperator(expressionStream, node, "&&");
    break;
  case AST_LOGICAL_NOT :
    ASTNode_generateUnaryOperator(expressionStream, node, "!");
    break;
  case AST_LOGICAL_OR :
    ASTNode_generateNaryOperator(expressionStream, node, "||");
    break;
  case AST_LOGICAL_XOR :
    ASTNode_generateXOR(expressionStream, node);
    break;
  case AST_RELATIONAL_EQ :
    ASTNode_generateNaryOperator(expressionStream, node, "==");
    break;
  case AST_RELATIONAL_GEQ :
    ASTNode_generateNaryOperator(expressionStream, node, ">=");
    break;
  case AST_RELATIONAL_GT :
    ASTNode_generateNaryOperator(expressionStream, node, ">");
    break;
  case AST_RELATIONAL_LEQ :
    ASTNode_generateNaryOperator(expressionStream, node, "<=");
    break;
  case AST_RELATIONAL_LT :
    ASTNode_generateNaryOperator(expressionStream, node, "<");
    break;
  case AST_RELATIONAL_NEQ :
    ASTNode_generateNaryOperator(expressionStream, node, "!=");
    break;
  default :
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_AST_COMPILATION_FAILED_STRANGE_NODE_TYPE,
		      "Found strange node type whilst generating code.  ",
		      "Inserted '_YUCK' into code.");
    CharBuffer_append(expressionStream, "_YUCK");
    break;
  }
}
/* End of file */
