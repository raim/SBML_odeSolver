/*
  Last changed Time-stamp: <2007-09-21 15:30:39 raim>
  $Id: adjsenstest.c,v 1.10 2007/09/24 09:41:54 raimc Exp $
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
#include <sbmlsolver/sensSolver.h>
#include <sbml/SBMLTypes.h>

void create_simple_v_file(char *v_file, int n_var, char **var);

int
main (int argc, char *argv[]){
  int i;
  
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  variableIndex_t *p;
  int flag;
   
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, 1000, 10);
  CvodeSettings_setErrors(set, 1e-15, 1e-8, 1e4);
  CvodeSettings_setMethod(set, 0, 5);
  /*   CvodeSettings_setStoreResults(set, 0); */
  CvodeSettings_setJacobian(set, 1); /* for testing only */
  CvodeSettings_setCompileFunctions(set, 1); /* for testing only */

 
  /* creating the odeModel */
  om = ODEModel_createFromFile("MAPK.xml");
  ii = IntegratorInstance_create(om, set);
  

  /* ACTIVATE SENSITIVITY ANALYSIS */
  CvodeSettings_setSensitivity(set, 1);
  /* 0: simultaneous 1: staggered, 2: staggered1
     see CVODES user guide for details */
  CvodeSettings_setSensMethod(set, 0);

  /* ACTIVATE ADJOINT ANALYSIS */
  CvodeSettings_setDoAdj(set);
  CvodeSettings_setAdjTime(set, 1000, 100);
  CvodeSettings_setAdjErrors(set, 1e-15, 1e-8);
  CvodeSettings_setnSaveSteps(set, 1000);

  printf("Try 3 integrations with selected parameters/ICs!\n");
  char *sensIDTest[4];  
  sensIDTest[0] = "MAPK";
  sensIDTest[2] = "MAPK_P";
  sensIDTest[1] = "K1";
  sensIDTest[3] = "Ki";
  CvodeSettings_setSensParams(set, sensIDTest, 4);

  fprintf(stderr, "\n\nReading in linear objective function from: 'MAPK.linobjfun'\n");
  fprintf(stderr, "Demonstration of forward/adjoint sensitivity (near) equivalence. \n\n");

  /* Initially, only linear objective is present */
  flag = IntegratorInstance_setLinearObjectiveFunction(ii, "MAPK.linobjfun");
  if (flag!=1)
    return(EXIT_FAILURE);  

  /* reset integrator to new settings */
  IntegratorInstance_reset(ii);
  
  /* get a parameter for which we will check sensitivities */
  p = ODEModel_getVariableIndex(om, "K1");  
   
  i = 0;
  while ( i < 4 ) {

   /*  if ( i == 2) break; */
    /* Set nonlinear objective function after 2 loops  */
    if ( i == 2)
    {
      fprintf(stderr, "\nReading in nonlinear objective now: 'MAPK.objfun'\n\n");
      flag = IntegratorInstance_setObjectiveFunction(ii, "MAPK.objfun");
      if (flag!=1)
	return(EXIT_FAILURE);
    }

    IntegratorInstance_reset(ii);
    
    while( !IntegratorInstance_timeCourseCompleted(ii) )
     if ( !IntegratorInstance_integrateOneStep(ii) )
       break;
    printf("\nIntegration time was %g\n\n",
	 IntegratorInstance_getIntegrationTime(ii));
     

    /*  IntegratorInstance_dumpData(ii); */
    printf("Param default: %s\n", ODEModel_getVariableName(om, p));
    IntegratorInstance_dumpPSensitivities(ii, p);

    flag = IntegratorInstance_CVODEQuad(ii);
    if (flag!=1)
	return(EXIT_FAILURE);

    if ( i < 2)
      fprintf(stderr, "\n### Printing Forward Sensitivities\n");
    else
      fprintf(stderr, "\n### Printing Objective Function (since nonlinear objective is present)\n");  

     
    flag = IntegratorInstance_printQuad(ii, stderr);


    if (flag!=1)
	return(EXIT_FAILURE);

     /* Now go into adjoint phase */   
    IntegratorInstance_resetAdjPhase(ii);
    /* Adjoint phase */
    while( !IntegratorInstance_timeCourseCompleted(ii) )
      if ( !IntegratorInstance_integrateOneStep(ii) )
	break;
    printf("\nIntegration time was %g\n\n",
	   IntegratorInstance_getIntegrationTime(ii));
    
    /* Print out adjoint soln */
    IntegratorInstance_dumpAdjData(ii);

    /* adjoint quadrature */
    flag = IntegratorInstance_CVODEQuad(ii);
     if (flag!=1) 
      return(EXIT_FAILURE);

    fprintf(stderr, "\n### Printing Adjoint Sensitivities: int_0^T <df/dp, psi> dt\n");
    flag = IntegratorInstance_printQuad(ii, stderr);
    if (flag!=1)
	return(EXIT_FAILURE); 


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
