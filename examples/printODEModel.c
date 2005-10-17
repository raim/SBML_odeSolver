/*
  Last changed Time-stamp: <2005-10-17 17:36:40 raim>
  $Id: printODEModel.c,v 1.2 2005/10/17 16:08:37 raimc Exp $
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
      ODEModel_createFromFile("basic-model1-forward-l2.xml", 1);


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
      formula = SBML_formulaToString(ODEModel_getOde(model, vi));
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
