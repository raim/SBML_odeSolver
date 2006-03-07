/*
  Last changed Time-stamp: <2006-03-07 16:56:51 raim>
  $Id: adj_sensitivity.c,v 1.4 2006/03/07 15:58:35 raimc Exp $
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



void create_v_file(char *v_file, int n_var, char **var, int flag);
ASTNode_t **read_v_file(char *v_file, int n_var, char **var);

/* --------- --------- --------- --------- --------- --------- --------- */

int
main (int argc, char *argv[]){
  int i, j;
  
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  variableIndex_t *p;
  char *sbml_file, *v_file, *data_file; 
  time_series_t *ts;
  int flag;
  double *dp;
  double tout; 

  /* read command line */
  if (argc != 4) {
    fprintf(stderr, "usage: %s sbml-file v-file data-file\n", argv[0]);
    exit (EXIT_FAILURE);
  } 
  else { 
    sbml_file = argv[1];
    v_file    = argv[2];
    data_file = argv[3];
  }
   
  fprintf(stderr, "\n \n This example performs forward and adjoint sensitivity analysis for data matching functional J(x) = int_0^T | x - x_delta |^2 dt. \n The two methods should give equivalent results. \n \n");


  /* creating the odeModel */
  om = ODEModel_createFromFile(sbml_file);

  /* default: create "Landweber file" */
  create_v_file(v_file, om->neq, om->names, 0);
  /* read v_vector */
  om->vector_v = read_v_file(v_file, om->neq, om->names);

  /* read observations */
  om->time_series = read_data(data_file, om->neq, om->names);
  ts = om->time_series;
  
  /* print data */
  /* print_data(ts); */

  /* test data */
  /* test_interpol(ts); */

  /* free data */
  /* free_data(ts); */

  /* time interval (of data) */
  tout = ts->time[ts->n_time-1];

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, tout, 10);
  CvodeSettings_setErrors(set, 1e-5, 1e-5, 1e7);

  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);

  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 1);
  CvodeSettings_setJacobian(set, 1);
 
  /* ACTIVATE ADJOINT ANALYSIS */
  CvodeSettings_setDoAdj(set);
  
  /* Do the time settings analogous to forward phase, but only reversed */
  CvodeSettings_setAdjTime(set, tout, 10);
  CvodeSettings_setAdjErrors(set, 1e-5, 1e-5);
  CvodeSettings_setnSaveSteps(set, 1000);
   
  /* set this as a default elsewhere !!! */
  om->n_adj_sens = om->nconst;
 
  /* get the last parameter (for which we will check sensitivities) */
  p = ODEModel_getConstantIndex(om, om->nconst-1);

  /* initialize the integrator */
  ii = IntegratorInstance_create(om, set);

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

 /*  VariableIndex_free(p); */

  /* forward quadrature */
  flag = IntegratorInstance_CVODEQuad(ii);
  if (flag==1)
    return(EXIT_FAILURE);

  fprintf(stderr, "\n### Printing Forward Sensitivities: int_0^T <x-x_delta, x_sens> dt \n");
  for(i=0;i<om->nconst;i++)
    fprintf(stderr, "dJ/dp_%d = %0.10g ", i, NV_Ith_S(ii->solver->q, i));
  fprintf(stderr, "\n \n");

  /* directional sensitivities */
  dp = space(ii->results->nsens * sizeof(double));
  for(i=0; i<ii->results->nsens; i++)
    dp[i] = 0.0;
  dp[ii->results->nsens-1] = 1.0; /* the last parameter */

  fprintf(stderr, "### Now computing and printing out directional sensitivities \nwith ");
  for(i=0; i<ii->results->nsens; i++)
    fprintf(stderr," dp[%d] = %.2g ", i, dp[i]);
   fprintf(stderr, "\n");

  printf("#time  ");
  IntegratorInstance_dumpNames(ii);

  CvodeResults_computeDirectional(ii->results, dp);
  for(j=0; j<ii->results->nout+1; j++){ 
    fprintf(stderr, "%g  ",ii->results->time[j] );
    for(i=0; i<ii->results->neq; i++)
      fprintf(stderr, " %.8g ", ii->results->directional[i][j]);
    fprintf(stderr, "\n");
  }

  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }

  /* Adjoint Integration */
  CvodeSettings_setAdjPhase(set);
   
  fprintf(stderr, "\n### Commencing adjoint integration:\n");
  /* For adjoint phase, resetting the isValid flag, 
     and create the necessary adjoint structures */
  ii->isValid = 0;
  IntegratorInstance_set(ii, set);
  
 
  printf("#time  ");
  IntegratorInstance_dumpNames(ii);
  ii->data->currenttime =  ii->solver->t0;
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
  if (flag==1) return(EXIT_FAILURE);
  
  fprintf(stderr, "\n### Printing Adjoint Sensitivities: int_0^T <df/dp, psi> dt   \n");
  for(i=0;i<om->n_adj_sens;i++)
    fprintf(stderr, "dJ/dp_%d = %0.10g ", i, NV_Ith_S(ii->solver->qA, i));
  fprintf(stderr, "\n \n");

  /* now we have the results and can free the inputs */
  IntegratorInstance_free(ii);
  CvodeSettings_free(set);
  ODEModel_free(om);

  return (EXIT_SUCCESS);  
}

/* --------- --------- --------- --------- --------- --------- --------- */

void create_v_file(char *v_file, int n_var, char **var, int flag){

  FILE *fp;
  int i;
  
  fp = fopen(v_file, "w");
    
  for(i=0; i<n_var; i++){
    fprintf(fp, "%s ", var[i]);
    if (flag == 0) /* Landweber */
      fprintf(fp, "%s - %s_data\n", var[i], var[i]);
    else           /* only data */
      fprintf(fp, "%s_data\n", var[i]);
  }

  fclose(fp);

}

/* --------- --------- --------- --------- --------- --------- --------- */

ASTNode_t **read_v_file(char *v_file, int n_var, char **var) {

  FILE *fp;
  char *line, *token;
  int i;
  ASTNode_t **vector_v, *tempAST;
  vector_v = space(n_var * sizeof(ASTNode_t *));

  if ((fp = fopen(v_file, "r")) == NULL)
    fatal(stderr, "read_v_file(): file not found");

  /* loop over lines */
  for (i=0; (line = get_line(fp)) != NULL; i++){
    /* read column 0 */
    token = strtok(line, " ");
    /* skip empty lines and comment lines */
    if (token == NULL || *token == '#'){
      free(line);
      i--;
      continue;
    }
    /* check variable order */
    if ( i == n_var )
      fatal(stderr, "read_v_file(): inconsistent number of variables (>)");
    if ( strcmp(token, var[i]) != 0 )
      fatal(stderr, "read_v_file(): inconsistent variable order");
    /* read column 1 */
    token = strtok(NULL, "");
    /* fprintf(stderr, "%s: %s\n", var[i], token); */
    tempAST = SBML_parseFormula(token);

    vector_v[i] = indexAST(tempAST, n_var, var);
   /*  fprintf(stderr, "vector_v[%d] = %s \n", i, SBML_formulaToString(vector_v[i])); */   
    ASTNode_free(tempAST);
    free(line);
  }


  if (i < n_var)
    fatal(stderr, "read_v_file(): inconsistent number of variables (<)");

  return vector_v;

}

/* End of file */
