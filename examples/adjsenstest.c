/*
  Last changed Time-stamp: <2006-10-01 14:31:31 raim>
  $Id: adjsenstest.c,v 1.1 2006/10/03 14:45:43 jamescclu Exp $
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

void create_simple_v_file(char *v_file, int n_var, char **var);

int
main (int argc, char *argv[]){
  int i, j;
  
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  cvodeResults_t *results;
  variableIndex_t *y, *p;
  int flag;
   
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

  /* simply create a v_file for the case  */
  /* where the objective function is: x -> int_{[0, T]} v(t) x(t) dt  */
  create_simple_v_file("adjsenstest_vector_v.txt", om->neq, om->names);
  /* read v_vector */ 
  IntegratorInstance_setLinearObjectiveFunction(ii, "adjsenstest_vector_v.txt");

  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);
  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 0);

  /* ACTIVATE ADJOINT ANALYSIS */
  CvodeSettings_setDoAdj(set);
  CvodeSettings_setAdjTime(set, 30, 10);
  CvodeSettings_setAdjErrors(set, 1e-9, 1e-6);
  CvodeSettings_setnSaveSteps(set, 100);

  printf("Try 3 integrations with selected parameters/ICs!\n");
  char *sensIDTest[3];  
  sensIDTest[0] = "MAPK_PP";
  sensIDTest[1] = "K1";
  sensIDTest[2] = "MKKK_P";
  CvodeSettings_setSensParams(set, sensIDTest, 3);

  /* reset integrator to new settings */
  IntegratorInstance_reset(ii);
  
  /* get a parameter for which we will check sensitivities */
  p = ODEModel_getVariableIndex(om, "K1");  
   
  i = 0;
  while ( i < 3 ) {
    IntegratorInstance_reset(ii);

    while( !IntegratorInstance_timeCourseCompleted(ii) )
     if ( !IntegratorInstance_integrateOneStep(ii) )
       break;
    
    /*  IntegratorInstance_dumpData(ii); */
    printf("Param default: %s\n", ODEModel_getVariableName(om, p));
    IntegratorInstance_dumpPSensitivities(ii, p);

    /* forward quadrature */
    flag = IntegratorInstance_CVODEQuad(ii);
    if (flag!=1)
      return(EXIT_FAILURE);
    fprintf(stderr, "### Printing Forward Sensitivities: int_0^T <x-x_delta, x_sens> dt \n");
    for(j=0;j<om->nsens;j++)
      fprintf(stderr, "dJ/dp_%d = %0.10g ", i, NV_Ith_S(ii->solver->q, j));
    fprintf(stderr, "\n \n");

    /* Now go into adjoint phase */   
    CvodeSettings_setAdjPhase(ii->opt); 
    IntegratorInstance_resetAdjPhase(ii); 
    /* Adjoint phase */
    while( !IntegratorInstance_timeCourseCompleted(ii) )
     if ( !IntegratorInstance_integrateOneStep(ii) )
       break;

    /* Print out adjoint soln */
    IntegratorInstance_dumpAdjData(ii);
    /* adjoint quadrature */
    flag = IntegratorInstance_CVODEQuad(ii);
    if (flag!=1) return(EXIT_FAILURE);
    fprintf(stderr, "### Printing Adjoint Sensitivities: int_0^T <df/dp, psi> dt   \n");
    for(j=0;j<om->nsens;j++)
      fprintf(stderr, "dJ/dp_%d = %0.10g ", j, NV_Ith_S(ii->solver->qA, j));
    
    CvodeSettings_unsetAdjPhase(ii->opt); 
    fprintf(stderr, "\n############# DONE RUN NUMBER %d  #############\n", i); 
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


/* --------- --------- --------- --------- --------- --------- --------- */
void create_simple_v_file(char *v_file, int n_var, char **var){

  FILE *fp;
  int i;
  
  fp = fopen(v_file, "w");
    
  for(i=0; i<n_var; i++){
    fprintf(fp, "%s ", var[i]); 
    fprintf(fp, "%s \n", var[i]);
  }

  fclose(fp);

}

/* End of file */
