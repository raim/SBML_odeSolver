/*
  Last changed Time-stamp: <2006-08-30 15:29:21 raim>
  $Id: senstest.c,v 1.1 2006/09/27 13:37:33 raimc Exp $
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
  CvodeSettings_setTime(set, 3.0, 10);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1e9);
  CvodeSettings_setMethod(set, 0, 5);
  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);
  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 0);
/*   CvodeSettings_setStoreResults(set, 0); */
  CvodeSettings_setJacobian(set, 1); /* for testing only */
  /* CvodeSettings_dump(set); */
  
  /* creating the odeModel */
  om = ODEModel_createFromFile("MAPKonlyK1.xml");
  /* get a parameter for which we will check sensitivities */
  p = ODEModel_getVariableIndex(om, "K1");
  /* calling the integrator */
  ii = IntegratorInstance_create(om, set);

  printf("### Printing Sensitivities to %s (%g) on the fly:\n",
	 ODEModel_getVariableName(om, p),
	 IntegratorInstance_getVariableValue(ii, p));
  
  printf("#time  ");
  IntegratorInstance_dumpNames(ii);

  /* IntegratorInstance_dumpPSensitivities(ii, p); */

  i = 0;
  while ( i < 10 ) {
    /* IntegratorInstance_dumpPSensitivities(ii, p); */

    j = 0;
    while( !IntegratorInstance_timeCourseCompleted(ii) ) {
      if ( !IntegratorInstance_integrateOneStep(ii) ) {
	break;
      }
      /* IntegratorInstance_dumpPSensitivities(ii, p); */
      j++;
    }
    IntegratorInstance_dumpData(ii);
    IntegratorInstance_dumpPSensitivities(ii, p);
    IntegratorInstance_reset(ii);
    i++;
  }
  
  VariableIndex_free(p);
  
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) )
  {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }


  y = ODEModel_getVariableIndex(om, "MAPK_P"); 
  CvodeSettings_dump(set);

  VariableIndex_free(y);
  /* now we have the results and can free the inputs */
  IntegratorInstance_free(ii);
  CvodeSettings_free(set);
  ODEModel_free(om);

  return (EXIT_SUCCESS);  
}



/* End of file */
