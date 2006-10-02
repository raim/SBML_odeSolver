/*
  Last changed Time-stamp: <2006-10-02 17:29:35 raim>
  $Id: printODEModel.c,v 1.5 2006/10/02 15:30:24 raimc Exp $
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

int main(void)
{
    int i;
    char *formula;
    variableIndex_t *vi = NULL;

    odeModel_t *model =
      ODEModel_createFromFile("basic-model1-forward-l2.xml");


    /* Get some information from constructed odeModel */
    printf("\n\n");
    printf("ODE Model Statistics:\n");
    printf("Number of ODEs:               %d\n",
	   ODEModel_getNeq(model));
    printf("Number of Assignments:        %d\n",
	   ODEModel_getNumAssignments(model));
    printf("Number of Constants:          %d\n",
	   ODEModel_getNumConstants(model));
    printf("                            ____\n");
    printf("Total number of values:       %d\n",
	   ODEModel_getNumValues(model));
    
    printf("\n");
    printf("ODEs:\n");
    for ( i=0; i<ODEModel_getNeq(model); i++ ){
      vi = ODEModel_getOdeVariableIndex(model, i);
      formula = SBML_formulaToString(ODEModel_getOde(model, vi));
      printf("d[%s]/dt = %s \n", ODEModel_getVariableName(model, vi), formula);
      free(formula);
      VariableIndex_free(vi);
    }
    printf("Assigned Variables:\n");
    for ( i=0; i<ODEModel_getNumAssignments(model); i++ ){
      vi = ODEModel_getAssignedVariableIndex(model, i);
      formula = SBML_formulaToString(ODEModel_getAssignment(model, vi));
      printf("%s = %s \n", ODEModel_getVariableName(model, vi), formula);
      free(formula);
      VariableIndex_free(vi);
    }
    printf("Constants:\n");
    for ( i=0; i<ODEModel_getNumConstants(model); i++ ){
      vi = ODEModel_getConstantIndex(model, i);
      printf("%s\n", ODEModel_getVariableName(model, vi));
      VariableIndex_free(vi);
    }
    printf("\n\n");

    
    ODEModel_free(model);
    return 1;
}

/* End of file */
