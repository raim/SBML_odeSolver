/*
  Last changed Time-stamp: <2007-09-13 18:10:06 raim>
  $Id: sensitivity.c,v 1.10 2007/09/20 01:16:12 raimc Exp $
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
  odeSense_t *os;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  cvodeResults_t *results;
  variableIndex_t *y, *p;

   
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, 300.0, 10);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1e9);
  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);
  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 0);
  CvodeSettings_setJacobian(set, 1); /* for testing only */
  /* CvodeSettings_dump(set); */

  CvodeSettings_setCompileFunctions(set, 1);

  
  /* creating the odeModel */
  om = ODEModel_createFromFile("MAPK.xml");
  /* get a parameter for which we will check sensitivities */
  p = ODEModel_getVariableIndex(om, "K1");
  /* calling the integrator */
  ii = IntegratorInstance_create(om, set);

  printf("### Printing Sensitivities to %s (%g) on the fly:\n",
	 ODEModel_getVariableName(om, p),
	 IntegratorInstance_getVariableValue(ii, p));
  
  printf("#time  ");
  IntegratorInstance_dumpNames(ii);

  IntegratorInstance_dumpPSensitivities(ii, p);

  while( !IntegratorInstance_timeCourseCompleted(ii) ) {
     if ( !IntegratorInstance_integrateOneStep(ii) ) {
       break;
     }
     IntegratorInstance_dumpPSensitivities(ii, p);
  }
  VariableIndex_free(p);
  
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }


  y = ODEModel_getVariableIndex(om, "MAPK_P");

  os = IntegratorInstance_getSensitivityModel(ii);
  
  printf("\nLet's look at a specific ODE variable:\n");
  /* print sensitivities again, but now from stored results */
  printf("### RESULTS for Sensitivity Analysis for one ODE variable\n");
  printf("#time  Variable  Sensitivity Params...\n");
  printf("#time  ");
  printf("%s  ", ODEModel_getVariableName(om, y));
  for ( j=0; j<ODESense_getNsens(os); j++ ) {
    p = ODESense_getSensParamIndexByNum(os, j);
    printf("%s ", ODEModel_getVariableName(om, p));
    VariableIndex_free(p);
  }
  printf("\n");

  results = IntegratorInstance_createResults(ii);
 
  for ( i=0; i<CvodeResults_getNout(results); i++ ) {
    printf("%g  ", CvodeResults_getTime(results, i));
    printf("%g  ", CvodeResults_getValue(results, y, i));
    for ( j=0; j<ODESense_getNsens(os); j++ ) {
      p = ODESense_getSensParamIndexByNum(os, j);
      printf("%g ", CvodeResults_getSensitivity(results, y, p, i));
      VariableIndex_free(p);
    }
    printf("\n");
  }

  drawSensitivity(ii->data, "sensitivity", "ps", 0.9);
  p = ODEModel_getVariableIndex(om, "V1");
  printf("\nWhat do sensitivities mean? Let's try out!\n\n");
  printf("... add 1 to %s:  %g + 1 = ",
	 ODEModel_getVariableName(om, p),
	 IntegratorInstance_getVariableValue(ii, p));
  
  
  CvodeSettings_setSensitivity(set, 0);
  IntegratorInstance_reset(ii);
  IntegratorInstance_setVariableValue(ii, p,
		     IntegratorInstance_getVariableValue(ii,p)+1);
  printf("%g\n", IntegratorInstance_getVariableValue(ii, p));
  
  printf("... and integrate again:\n\n");

  CvodeResults_free(results);
  IntegratorInstance_integrate(ii); 
  results = IntegratorInstance_createResults(ii);

  /* and print changed results */
  printf("#time  %s\n", ODEModel_getVariableName(om, y));
  for ( i=0; i<CvodeResults_getNout(results); i++ ) {
    printf("%g  ", CvodeResults_getTime(results, i));
    printf("%g\n", CvodeResults_getValue(results, y, i));
  }
  
  printf("\nSee the difference?\n");
  printf("Look what happens when the sensitivity changes its sign\n");
  printf("between times 180 and 210.\n\n");

  VariableIndex_free(y);
  VariableIndex_free(p);
  /* now we have the results and can free the inputs */
  IntegratorInstance_free(ii);
  CvodeSettings_free(set);
  CvodeResults_free(results);
  ODEModel_free(om);

  return (EXIT_SUCCESS);  
}



/* End of file */
