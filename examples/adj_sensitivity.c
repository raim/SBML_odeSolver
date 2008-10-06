/*
  Last changed Time-stamp: <2008-10-06 14:42:42 raim>
  $Id: adj_sensitivity.c,v 1.14 2008/10/06 12:46:51 raimc Exp $
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
#include <string.h>

#include <sbmlsolver/odeSolver.h>
#include <sbmlsolver/interpol.h>
#include <sbmlsolver/variableIndex.h>
#include <sbmlsolver/sensSolver.h>


/* --------- --------- --------- --------- --------- --------- --------- */

int
main (int argc, char *argv[]){
  int i, j;
  
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  variableIndex_t *p;
  char *sbml_file, *objfun_file, *data_file; 
  int flag;
  double *dp;
  double tout; 


  /* read command line */
  if (argc != 4) {
    fprintf(stderr, "usage: %s sbml-file objfun-file data-file\n", argv[0]);
    fprintf(stderr, "eg: ./adj_sensitivity repressilator.xml repressilator.objfun repressilator_data.txt\n");
    exit (EXIT_FAILURE);
  } 
  else { 
    sbml_file      = argv[1];
    objfun_file    = argv[2];
    data_file      = argv[3];
  }
  
  fprintf(stdout, "\nThis example performs forward and adjoint sensitivity analysis for data mis-match functional\n");
  fprintf(stdout, "J(x) = int_0^T | x - x_data |^2 dt. The two methods should give (up-to-numerics) equivalent results. \n");

  /* creating the odeModel */
  om = ODEModel_createFromFile(sbml_file);

  /* time interval (of data) */
  tout = 100;

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setCompileFunctions(set, 1);
  CvodeSettings_setTime(set, tout, 5);
  CvodeSettings_setErrors(set, 1e-9, 1e-9, 1e7);

  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);

  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 1);
  CvodeSettings_setJacobian(set, 1);
 
  /* ACTIVATE ADJOINT ANALYSIS */
  CvodeSettings_setDoAdj(set);
  
  /* Do the time settings analogous to forward phase, but only reversed */
  CvodeSettings_setAdjTime(set, tout, 5);
  CvodeSettings_setAdjErrors(set, 1e-5, 1e-5);
  CvodeSettings_setnSaveSteps(set, 2);
    
  /* get the last parameter (for which we will check sensitivities) */
  p = ODEModel_getConstantIndex(om, om->nconst-1);

  /* initialize the integrator */
  ii = IntegratorInstance_create(om, set);
 
  flag = IntegratorInstance_setObjectiveFunction(ii, objfun_file);
  if (flag!=1)
  {
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }


  /* read observations */
  flag = IntegratorInstance_readTimeSeriesData(ii, data_file);
  if (flag!=1)
	return(EXIT_FAILURE);

  printf("### Printing Sensitivities to %s (%g) on the fly:\n",
	 ODEModel_getVariableName(om, p),
	 IntegratorInstance_getVariableValue(ii, p));
  
  printf("#time  ");
  IntegratorInstance_dumpNames(ii);
  IntegratorInstance_dumpPSensitivities(ii, p);
  
  /* forward integration (incl. sensitivity analysis) */
  while( !IntegratorInstance_timeCourseCompleted(ii) ) {
    if ( !IntegratorInstance_integrateOneStep(ii) ) {
      break;
    }
    IntegratorInstance_dumpPSensitivities(ii, p);
  }

  /* forward quadrature */
  flag = IntegratorInstance_CVODEQuad(ii);
  if (flag!=1)
    return(EXIT_FAILURE);

  fprintf(stdout, "\n### Printing Forward Sensitivities: int_0^T <x-x_delta, x_sens> dt \n");
  flag = IntegratorInstance_printQuad(ii, stdout);
    if (flag!=1)
	return(EXIT_FAILURE); 

  /* directional sensitivities */
  dp = space(ii->results->nsens * sizeof(double));
  for(i=0; i<ii->results->nsens; i++)
    dp[i] = 0.0;
  dp[ii->results->nsens-1] = 1.0; /* the last parameter */

  fprintf(stdout, "### Now computing and printing out directional sensitivities \nwith ");
  for(i=0; i<ii->results->nsens; i++)
    fprintf(stdout," dp[%d] = %.2g ", i, dp[i]);
   fprintf(stdout, "\n");

  printf("#time  ");
  IntegratorInstance_dumpNames(ii);

  CvodeResults_computeDirectional(ii->results, dp);
  for(j=0; j<ii->results->nout+1; j++){ 
    fprintf(stdout, "%g  ",ii->results->time[j] );
    for(i=0; i<ii->results->neq; i++)
      fprintf(stdout, " %.8g ", ii->results->directional[i][j]);
    fprintf(stdout, "\n");
  }

  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }


  
  /* Adjoint Integration */
  fprintf(stdout, "\n### Commencing adjoint integration:\n");
 
  IntegratorInstance_resetAdjPhase(ii);

  printf("#time  ");
  IntegratorInstance_dumpNames(ii);
  IntegratorInstance_dumpAdjData(ii);

  /*  Now the adjoint solver is called in the integration steps  */
  while( !IntegratorInstance_timeCourseCompleted(ii) ) {
    if ( !IntegratorInstance_integrateOneStep(ii) ) {
      fatal(stderr, " ERROR in Integrate One Step \n");
      break;
    }
    IntegratorInstance_dumpAdjData(ii);
  }
  
  /* adjoint quadrature */
  flag = IntegratorInstance_CVODEQuad(ii);
  if (flag!=1) return(EXIT_FAILURE);
  
  fprintf(stdout, "\n### Printing Adjoint Sensitivities: int_0^T <df/dp, psi> dt   \n");
  flag = IntegratorInstance_printQuad(ii, stdout);
    if (flag!=1)
	return(EXIT_FAILURE);
    
  /* now we have the results and can free the inputs */
  VariableIndex_free(p);
  IntegratorInstance_free(ii);
  CvodeSettings_free(set);
  ODEModel_free(om);
  free(dp);
  return (EXIT_SUCCESS);  
}


/* End of file */
