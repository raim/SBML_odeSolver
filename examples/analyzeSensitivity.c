/*
  Last changed Time-stamp: <2005-11-17 13:27:42 raim>
  $Id: analyzeSensitivity.c,v 1.3 2005/11/17 13:01:50 raimc Exp $
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


static printSensiMatrix(odeModel_t *odeModel, cvodeData_t *data)
{

  int i, j;
  const ASTNode_t *f  = NULL;
  char *formula;

  /* first, get the number of equations of the ODE system */
  int neq = ODEModel_getNeq(odeModel);
  
  printf("i\\j ");
  for ( j=0; j<ODEModel_getNsens(odeModel); j++ )
    printf("%d   ", j);
  printf("\n");
      
  for ( i=0; i<ODEModel_getNeq(odeModel); i++ ) {
    printf("%d   ", i);
    for ( j=0; j<ODEModel_getNsens(odeModel); j++ ) {
      f = ODEModel_getSensIJEntry(odeModel, i, j);
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
    int i, j, neq;
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

 
    if ( ODEModel_constructSensitivity(odeModel) &&
	 ODEModel_getNeq(odeModel) ) {
      printf("\n\nSuccessfully constructed the parametric matrix S ");
      printf("as used for CVODES sensitivity analysis\n\n");
      printf("We might be interested in the `sparsity' of S,\n");
      printf("... we can just evaluate the parametric entries:\n\n");
      
      /* we need cvodeData for evaluating formulas */
      data = CvodeData_create(odeModel);
      printf("... how many parametric entries: %d\n\n",
	     ODEModel_getNsens(odeModel));
      
      /* now take a look at a all entries of S: */
      
      for ( i=0; i<ODEModel_getNeq(odeModel); i++ ) {
	
	vi = ODEModel_getOdeVariableIndex(odeModel, i);
	
	printf("\nODE VARIABLE %d: %s\n", i+1,
	       ODEModel_getVariableName(odeModel, vi));
	
	f = ODEModel_getOde(odeModel, vi);
	formula = SBML_formulaToString(f);

	printf("dY/dt =  %s \n", formula);
	free(formula);
      
	for ( j=0; j<ODEModel_getNsens(odeModel); j++ ) {
	  vj = ODEModel_getSensParamIndexByNum(odeModel, j);
	  printf("  Parameter %d: %s   ", j,
		 ODEModel_getVariableName(odeModel, vj));
	  f = ODEModel_getSensIJEntry(odeModel, i, j);
	  formula = SBML_formulaToString(f); 
	  printf("  S[%d][%d] = %s \n", i, j, formula);
	  free(formula);
	  VariableIndex_free(vj);
	}
	VariableIndex_free(vi);
      }

      printf("\n\n");      
      
      printf("Sensitivity: parametric matrix with initial conditions:\n");
      printSensiMatrix(odeModel, data);
      /* we must free this cvodeData structure */ 
      CvodeData_free(data);
      
      printf("Thx and good bye!\n");
    }
    else {
      SolverError_dumpAndClearErrors();
      printf("Sorry, for this system we couldn't generate the Parametric.\n");
      printf("Sensitivity can still be run with internal approximation.\n");
    }
      
    
    ODEModel_free(odeModel);
    return 1;
}


/* End of file */
