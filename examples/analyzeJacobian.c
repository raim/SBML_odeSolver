/*
  Last changed Time-stamp: <2006-04-19 15:06:16 raim>
  $Id: analyzeJacobian.c,v 1.6 2006/04/19 13:12:12 raimc Exp $
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
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "sbml/SBMLTypes.h"
#include "sbmlsolver/odeSolver.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/solverError.h"


static void printJacobian(odeModel_t *odeModel, cvodeData_t *data)
{

  int i, j;
  const ASTNode_t *f  = NULL;

  printf("i\\j ");
  for ( j=0; j<ODEModel_getNeq(odeModel); j++ )
    printf("%d   ", j);
  printf("\n");
      
  for ( i=0; i<ODEModel_getNeq(odeModel); i++ ) {
    printf("%d   ", i);
    for ( j=0; j<ODEModel_getNeq(odeModel); j++ ) {
      f = ODEModel_getJacobianIJEntry(odeModel, i, j);
      /* now let's see wether the entry is positive or negative at
	 this point */
      if ( evaluateAST((ASTNode_t *)f, data) < 0 )
	printf("-   ");
      if ( evaluateAST((ASTNode_t *)f, data) > 0 )
	printf("+   ");	  
      if ( evaluateAST((ASTNode_t *)f, data) == 0 )
	printf("0   ");
    }
    printf("\n\n");
  }
  printf("\n");
}


int main(void)
{
    char *formula;
    variableIndex_t *vi = NULL;
    variableIndex_t *vj = NULL;
    const ASTNode_t *f  = NULL;
    cvodeData_t *data   = NULL;
    cvodeSettings_t *set;
    integratorInstance_t *iI;

    /* first load an SBML model and directly construct the
       internal odeModel from it */
    odeModel_t *odeModel = ODEModel_createFromFile("MAPK.xml");

 
    if ( ODEModel_constructJacobian(odeModel) && ODEModel_getNeq(odeModel) ) {
      printf("\n\nSuccessfully constructed the jacobian matrix J\n\n");
      printf("We might be interested in the `sparsity' of J,\n");
      printf("... we can just evaluate the jacobian entries:\n\n");
      
      /* we need cvodeData for evaluating formulas */
      data = CvodeData_create(odeModel);
      
      printf("Jacobian with initial conditions:\n");
      printJacobian(odeModel, data);
      /* we must free this cvodeData structure */ 
      CvodeData_free(data);
	
      printf("Does it change after integration?\n\n");
      
      /* creating settings and integrate */
      set = CvodeSettings_create();
      CvodeSettings_setTime(set, 1000, 1);
      iI = IntegratorInstance_create(odeModel, set);
      IntegratorInstance_integrate(iI);
      
      /* get cvodeData from integratorInstance, it contains
         the values at the last time point,
	 and just do the same as above */
      
      data = IntegratorInstance_getData(iI);
      printf("Jacobian at time %g:\n", IntegratorInstance_getTime(iI));
      printJacobian(odeModel, data);
      printf("J[6,4] changed its sign. Let's take a look at the equations:\n\n");
     
      vi = ODEModel_getOdeVariableIndex(odeModel, 6);
      vj = ODEModel_getOdeVariableIndex(odeModel, 4);      

      f = ODEModel_getOde(odeModel, vi);
      formula = SBML_formulaToString(f);
      
      printf("The ODE d%s/dt = \n%s \n\n",
	     ODEModel_getVariableName(odeModel, vi),
	     formula);
      free(formula);      

      f = ODEModel_getJacobianEntry(odeModel, vi, vj);
      formula = SBML_formulaToString(f); 
      printf("The jacobian entry (d%s/dt)/d%s = \n%s \n\n",
	     ODEModel_getVariableName(odeModel, vi),
	     ODEModel_getVariableName(odeModel, vj),
	     formula);
      free(formula);
      VariableIndex_free(vi);
      VariableIndex_free(vj);
      
      printf("MAPK_P is both a substrate and a product of MKK_PP ");
      printf("in different reactions.\nTherefor the corresponding ");
      printf("entry in the jacobian can change its sign, depending ");
      printf("on concentrations!\n");
      /* finally draw a `species interaction graph' of the jacobian' */
      drawJacoby(data, "jacobian", "gif");      
      printf("Take a look at jacobian interaction graph in");
      printf("file jacobian_jm.gif that has just been constructed.\n");
      printf("If you have compiled w/o graphviz, you just have a textfile");
      printf(" jacobian.dot\n");
      printf("Thx and good bye!\n");
      
       /* note that this cvodeData MUST NOT be freed, it stays with and
         will be freed together with integratorInstance */
      IntegratorInstance_free(iI);
      CvodeSettings_free(set);
    }
    else {
      SolverError_dumpAndClearErrors();
      printf("Sorry, for this system we couldn't generate the Jacobian.\n");
      printf("Integration can still be run with internal approximation.\n");
    }
      
    
    ODEModel_free(odeModel);
    return 1;
}


/* End of file */
