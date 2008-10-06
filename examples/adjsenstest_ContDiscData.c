/*
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
 *     James Lu
 *
 * Contributor(s):
 */

#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>
#include <sbmlsolver/sensSolver.h>
#include <sbml/SBMLTypes.h>


int
main (int argc, char *argv[]){
  
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  int flag, RunIndex, RunIndexOuter;
  char *DataFileNames[4], *ObjFuncFileNames[4];
  variableIndex_t *vi;  
  int NRuns = 4;

  DataFileNames[0] = "MAPK_10pt.dat";
  DataFileNames[1] = "MAPK_100pt.dat";
  DataFileNames[2] = "MAPK_1000pt.dat";
  DataFileNames[3] = "MAPK_10000pt.dat";

  ObjFuncFileNames[0] = "MAPK_10pt.objfun";
  ObjFuncFileNames[1] = "MAPK_100pt.objfun";
  ObjFuncFileNames[2] = "MAPK_1000pt.objfun";
  ObjFuncFileNames[3] = "MAPK_10000pt.objfun"; 

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();

  CvodeSettings_setCompileFunctions(set, 1);

  /* use time series data as discrete observations and set corresponding print steps */ 
  CvodeSettings_setDiscreteObservation(set);
  
  CvodeSettings_setErrors(set, 1e-8, 1e-8, 1e6);
  CvodeSettings_setMethod(set, 0, 5);
  CvodeSettings_setJacobian(set, 1);
  CvodeSettings_setStoreResults(set, 1);

  CvodeSettings_setCompileFunctions(set, 0); /* do not compile RHS functions */
  CvodeSettings_setSensitivity(set, 0);
  CvodeSettings_setSensMethod(set, 0);

  /* ACTIVATE ADJOINT ANALYSIS */
  CvodeSettings_setDoAdj(set);
  CvodeSettings_setAdjErrors(set, 1e-8, 1e-8);
  CvodeSettings_setnSaveSteps(set, 1e5);

  char *sensIDTest[4];  
  sensIDTest[0] = "MAPK";
  sensIDTest[1] = "MAPK_P";
  sensIDTest[2] = "K1";
  sensIDTest[3] = "Ki";
  printf("Sensitivity analysis for 2 ICs: %s, %s\n2 parameters: %s, %s\n", 
	 sensIDTest[0], sensIDTest[1], sensIDTest[2], sensIDTest[3]); 
  CvodeSettings_setSensParams(set, sensIDTest, 4);

  om = ODEModel_createFromFile("MAPK.xml");  
  /* Set parameter Ki from original value of 10 to 9.2 */
  vi = ODEModel_getVariableIndex(om, "K1");
  if (vi == NULL){
      fprintf(stderr, "vi == NULL \n");
      return FALSE;
   }
 
  ii = IntegratorInstance_create(om, set);
 

  for (RunIndexOuter=0; RunIndexOuter<NRuns*2; RunIndexOuter++)
  {
    fprintf(stdout, "\n\n");

    if (RunIndexOuter >=NRuns )
    {   
        CvodeSettings_unsetDiscreteObservation(set);
	RunIndex = RunIndexOuter-NRuns;

        if (RunIndexOuter == 1)
	  CvodeSettings_setSensitivity(set, 0);

        if (RunIndexOuter == NRuns)
	{
         IntegratorInstance_printStatistics(ii, stdout);
         printf("\n\n==== NOW TREATING DATA AS BEING CONTINUOUS ====\n\n\n\n");
	}
    }
    else
      RunIndex = RunIndexOuter;


    CvodeSettings_setForwAdjTimeSeriesFromData(set, DataFileNames[RunIndex], 3);   
    IntegratorInstance_reset(ii);
    IntegratorInstance_setVariableValue(ii, vi, 9.2);
   
    if (ii->opt->observation_data_type == 1)
    { /* discrete observation */
      fprintf(stdout, "Reading in objective '%s' ... ",    ObjFuncFileNames[RunIndex]);
      flag = IntegratorInstance_setObjectiveFunction(ii,   ObjFuncFileNames[RunIndex]);
      if (flag!=1)
	return(EXIT_FAILURE);
    }
    else
    {  /* continuous observation */
      fprintf(stdout, "Reading in objective '%s' ... ",  "MAPK_withData.objfun");
      flag = IntegratorInstance_setObjectiveFunction(ii, "MAPK_withData.objfun");
      if (flag!=1)
	return(EXIT_FAILURE);
    }

    fprintf(stdout, "and data '%s'\n", DataFileNames[RunIndex] );

    flag = IntegratorInstance_readTimeSeriesData(ii, DataFileNames[RunIndex]);

     
    if (flag!=1)
      return(EXIT_FAILURE);
    
    while( !IntegratorInstance_timeCourseCompleted(ii) ){
      if ( !IntegratorInstance_integrateOneStep(ii) )	
	break;
    }


    flag = IntegratorInstance_CVODEQuad(ii);
    if (flag!=1)
      return(EXIT_FAILURE);
    
    printf("\nForward integration time was %g\n",
	   IntegratorInstance_getIntegrationTime(ii));

    
    fprintf(stdout, "### Printing Objective Value:\n"); 
    flag = IntegratorInstance_printQuad(ii, stdout);
    if (flag!=1)
      return(EXIT_FAILURE);
    
    
     
    /* Now go into adjoint phase */     
    IntegratorInstance_resetAdjPhase(ii); 
    
    /* Adjoint phase */
    /* Print out adjoint soln */
    while( !IntegratorInstance_timeCourseCompleted(ii) ){
      /*  if (RunIndex == 0) */
      /* 	 IntegratorInstance_dumpAdjData(ii); */
      if ( !IntegratorInstance_integrateOneStep(ii) )
      { 
	fprintf(stderr, "Error in integrating one step!\n");
	break;
      }     
    }
    
    
    /* adjoint quadrature */
    flag = IntegratorInstance_CVODEQuad(ii);
    if (flag!=1) 
      return(EXIT_FAILURE);
    
    printf("\nAdjoint integration time was %g\n", IntegratorInstance_getIntegrationTime(ii));
    
    fprintf(stdout, "### Printing Adjoint Sensitivities:\n");
    flag = IntegratorInstance_printQuad(ii, stdout);
    if (flag!=1)
      return(EXIT_FAILURE);     
 
  }

  IntegratorInstance_printStatistics(ii, stdout);

  /* now we have the results and can free the inputs */
  IntegratorInstance_free(ii); 
  ODEModel_free(om); 
  CvodeSettings_free(set);
  VariableIndex_free(vi);
  

  return (EXIT_SUCCESS);  
}
/* End of file */
