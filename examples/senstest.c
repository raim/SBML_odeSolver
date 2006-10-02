/*
  Last changed Time-stamp: <2006-10-02 16:51:52 raim>
  $Id: senstest.c,v 1.4 2006/10/02 14:53:35 raimc Exp $
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
 */

#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>


int
main (int argc, char *argv[]){
  int i, j;
  
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  cvodeResults_t *results;
  variableIndex_t *y, *p;

   
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, 30, 10);
  CvodeSettings_setErrors(set, 1e-9, 1e-6, 1e9);
  CvodeSettings_setMethod(set, 0, 5);
  /*   CvodeSettings_setStoreResults(set, 0); */
  CvodeSettings_setJacobian(set, 1); /* for testing only */
  /* CvodeSettings_dump(set); */
  
  /* creating the odeModel */
  om = ODEModel_createFromFile("MAPK.xml");
  ii = IntegratorInstance_create(om, set);



  printf("First try multiple integrations with default sensitivity !\n");

  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);
  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 0);

  /* reset integrator to new settings */
  IntegratorInstance_reset(ii);
  
  /* get a parameter for which we will check sensitivities */
  p = ODEModel_getVariableIndex(om, "K1");  

  
  i = 0;
  printf("Default case 1, with analytic matrices\n");
  printf("Param %s:\n", ODEModel_getVariableName(om, p));
  while ( i < 10 ) {
    while( !IntegratorInstance_timeCourseCompleted(ii) )
     if ( !IntegratorInstance_integrateOneStep(ii) )
       break;
    
   /*  IntegratorInstance_dumpData(ii); */
    IntegratorInstance_dumpPSensitivities(ii, p);

    IntegratorInstance_reset(ii);
    i++;
  }
  
  /*   IntegratorInstance_dumpPSensitivities(ii, p); */
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) )
  {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }

  printf("\n");
  /*   CvodeSettings_dump(set); */
  char *sensIDs[3];  
  sensIDs[0] = "MAPK_PP";
  sensIDs[1] = "K1";
  sensIDs[2] = "MKKK_P";
  CvodeSettings_setSensParams(set, sensIDs, 3);

  IntegratorInstance_reset(ii);
  
  while( !IntegratorInstance_timeCourseCompleted(ii) )
    if ( !IntegratorInstance_integrateOneStep(ii) )
      break;

  printf("Selected parameter case 1, analytic matrices.\n");
  y = ODEModel_getVariableIndex(om, "K1");   
  printf("Param %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpPSensitivities(ii, y);
  VariableIndex_free(y);
  
  
  y = ODEModel_getVariableIndex(om, "MAPK_PP");   
  printf("Variable %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpPSensitivities(ii, y);
  VariableIndex_free(y);
  
  y = ODEModel_getVariableIndex(om, "MKKK");
  printf("Sens. for variable %s:\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpYSensitivities(ii, y);
  VariableIndex_free(y);

 /*  IntegratorInstance_dumpData(ii); */
  /* deactivate matrices */
  CvodeSettings_setJacobian(set, 0);
  char *sensIDs2[4];  
  sensIDs2[3] = "V1";
  sensIDs2[0] = "K1";
  sensIDs2[1] = "MAPK_PP";
  sensIDs2[2] = "MKKK";
  CvodeSettings_setSensParams(set, sensIDs2, 4);   
  IntegratorInstance_reset(ii);
  IntegratorInstance_integrate(ii);

  /*   IntegratorInstance_dumpData(ii); */
  printf("\nSelected parameter case 2, internal approximation of matrices.\n");
  
  y = ODEModel_getVariableIndex(om, "MAPK_PP");   
  printf("Param %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpPSensitivities(ii, y);  
  VariableIndex_free(y);
  
  y = ODEModel_getVariableIndex(om, "K1");
  printf("Param %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpPSensitivities(ii, y);
  VariableIndex_free(y);

  y = ODEModel_getVariableIndex(om, "V1");
  printf("Param %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpPSensitivities(ii, y);
  VariableIndex_free(y);

  y = ODEModel_getVariableIndex(om, "MKKK_P");   
  printf("Param %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpPSensitivities(ii, y);  
  VariableIndex_free(y);

  y = ODEModel_getVariableIndex(om, "MKKK");   
  printf("Sens. for variable : %s\n", ODEModel_getVariableName(om, y));
  IntegratorInstance_dumpYSensitivities(ii, y);
  VariableIndex_free(y);  

  /* switch to default sens. again and run iteration */
  printf("\n\nDefault case 2, switching between approx. and analytic\n");
  printf("\nSwitching back to analytic matrices and default sens.\n\n"); 
  printf("Param %s:\n", ODEModel_getVariableName(om, p));
  CvodeSettings_setJacobian(set, 1);
  CvodeSettings_setSensParams(set, NULL, 4);
  IntegratorInstance_reset(ii);  
  i = 0;
  while ( i < 10 ) {
    /* IntegratorInstance_dumpPSensitivities(ii, p); */

    j = 0;
    while( !IntegratorInstance_timeCourseCompleted(ii) ) {
      if ( !IntegratorInstance_integrateOneStep(ii) ) 
	break;
      
/*       IntegratorInstance_dumpPSensitivities(ii, p); */
      j++;
    }
    /*  IntegratorInstance_dumpData(ii); */
    IntegratorInstance_dumpPSensitivities(ii, p);

    if(i==5 )
    {
      printf("\nNow again with internal approximation of matrices: \n\n"); 
      /* deactivate matrices */
      CvodeSettings_setJacobian(set, 0);
    }
    IntegratorInstance_reset(ii);
    i++;
  }
  
  /*   VariableIndex_free(y); */
  VariableIndex_free(p);
  /* now we have the results and can free the inputs */
  IntegratorInstance_free(ii);
  CvodeSettings_free(set);
  ODEModel_free(om);

  return (EXIT_SUCCESS);  
}



/* End of file */
