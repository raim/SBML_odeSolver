/*
  Last changed Time-stamp: <2005-11-02 19:02:22 raim>
  $Id: sensitivity.c,v 1.1 2005/11/02 18:42:49 raimc Exp $
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
  CvodeSettings_setTime(set, 1000.0, 10);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1000);
  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);

  /* creating the odeModel */
  om = ODEModel_createFromFile("MAPK.xml");
  y = ODEModel_getVariableIndex(om, "MKKK_P");
  
  /* calling the integrator */
  ii = IntegratorInstance_create(om, set);

  printf("#time  Variable  Sensitivity Params...\n");
  printf("#time  ");
  printf("%s  ", ODEModel_getVariableName(om, y));
  for ( j=0; j<ODEModel_getNsens(om); j++ ) {
    p = ODEModel_getSensParamIndexByNum(om, j);
    printf("%s ", ODEModel_getVariableName(om, p));
    VariableIndex_free(p);
  }
  printf("\n");
  
  IntegratorInstance_dumpYSensitivities(ii, y);
  while( !IntegratorInstance_timeCourseCompleted(ii) ) {

    IntegratorInstance_integrateOneStep(ii);
    IntegratorInstance_dumpYSensitivities(ii, y);
    
  }
  
  /* IntegratorInstance_integrate(ii); */
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }


  /* print sensitivities again, but now from stored results */
  printf("### RESULTS for Sensitivity Analysis\n");

  results = IntegratorInstance_createResults(ii);
 
  for ( i=0; i<CvodeResults_getNout(results); i++ ) {
    printf("%g  ", CvodeResults_getTime(results, i));
    printf("%g  ", CvodeResults_getValue(results, y, i));
    for ( j=0; j<ODEModel_getNsens(om); j++ ) {
      p = ODEModel_getSensParamIndexByNum(om, j);
      printf("%g ", CvodeResults_getSensitivity(results, y, p, i));
      VariableIndex_free(p);
    }
    printf("\n");
  }

  VariableIndex_free(y);
  /* now we have the results and can free the inputs */
  CvodeSettings_free(set);
  CvodeResults_free(results);
  IntegratorInstance_free(ii);
  ODEModel_free(om);


  return (EXIT_SUCCESS);  
}



/* End of file */
