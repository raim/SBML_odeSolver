/*
  Last changed Time-stamp: <2007-09-20 01:54:22 raim>
  $Id: bistability.c,v 1.2 2007/09/20 01:16:12 raimc Exp $
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
 *     Andrew Finney
 *
 * Contributor(s):
 */
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

void DumpState(
	       integratorInstance_t *ii, variableIndex_t *s,  variableIndex_t *u, variableIndex_t *a/* , variableIndex_t *a2 */)
{
  printf(
	 " %g %g %g\n", 
	 IntegratorInstance_getVariableValue(ii, s),
	 IntegratorInstance_getVariableValue(ii, u),
	 IntegratorInstance_getVariableValue(ii, a)/* +IntegratorInstance_getVariableValue(ii, a2) */);
}

void DumpErrors()
{
  char *errors = SolverError_dumpToString();

  fprintf(stderr, errors);
  SolverError_freeDumpString(errors);
  SolverError_clear();
}

int doit(int argc, char *argv[])
{
  double i, j ;
  cvodeSettings_t *settings = CvodeSettings_create();
  variableIndex_t *speciesVI, *parameterVI, *parameter2VI;
  integratorInstance_t *integratorInstance;
  char *modelStr, *parameterStr, *parameter2Str, *speciesStr, *observable[1];
  double parameter, parameterEnd, parameterStepSize,
    parameter2, parameter2End, parameter2StepSize,
    errorTolerance, relativeErrorTolerance;
  int maximumIntegrationSteps;
  odeModel_t *model ;
        
  if (argc < 11)
  {
    fprintf(
            stderr,
            "usage %s sbml-model variable parameter1 parameter1-start parameter1-end parameter1-step parameter1 parameter2-start parameter2-end parameter2-step [error-tolerance] [relative-error-tolerance] [maximum integration steps]\n",
            argv[0]);

    exit(0);
  }

  modelStr = argv[1];
  speciesStr = argv[2];

  parameterStr = argv[3];
  parameter = atof(argv[4]);
  parameterEnd = atof(argv[5]);
  parameterStepSize = atof(argv[6]);
  
  parameter2Str = argv[7];
  parameter2 = atof(argv[8]);
  parameter2End = atof(argv[9]);
  parameter2StepSize = atof(argv[10]);

  if (argc > 11)
    errorTolerance = atof(argv[11]);
  else
    errorTolerance = 1e-6;

  if (argc > 12)
    relativeErrorTolerance = atof(argv[12]);
  else
    relativeErrorTolerance = 1e-4;

  if (argc > 13)
    maximumIntegrationSteps = atoi(argv[13]);
  else
    maximumIntegrationSteps = 1e9;

  observable[0] = "v1s";
  model = ODEModel_createFromFileWithObservables(modelStr, observable);
  RETURN_ON_ERRORS_WITH(1);

  speciesVI = ODEModel_getVariableIndex(model, speciesStr);
/*   species2VI = ODEModel_getVariableIndex(model, "A2"); */
  parameterVI = ODEModel_getVariableIndex(model, parameterStr);
  parameter2VI = ODEModel_getVariableIndex(model, parameter2Str);
  RETURN_ON_ERRORS_WITH(1);
    
  CvodeSettings_setIndefinitely(settings, 0);
  /* run without a defined end time */
  CvodeSettings_setTime(settings, 100000, 1000);
  /*  end time is the step size - Indefinitely == 1 */
  CvodeSettings_setError(settings, errorTolerance);
  /* absolute tolerance in Cvode integration */
  CvodeSettings_setRError(settings, relativeErrorTolerance);        /* relative tolerance in Cvode integration */
  CvodeSettings_setMxstep(settings, maximumIntegrationSteps);        /* maximum step number for CVode integration */
  CvodeSettings_setHaltOnEvent(settings, 0);      /* doesn't stop integration upon an event */
  CvodeSettings_setSteadyStateThreshold(settings, 1e-9);  
  CvodeSettings_setHaltOnSteadyState(settings, 0);      /* doesn't stop integration upon a steady state */
  CvodeSettings_setJacobian(settings, 1);      /* Toggle use of Jacobian ASTs or approximation */
  CvodeSettings_setStoreResults(settings, 0);     /* don't Store time course history */
  CvodeSettings_setCompileFunctions(settings, 0); /* compile model */ 

    
  integratorInstance = IntegratorInstance_create(model, settings);
    
  printf("set xlabel '%s'\n", ODEModel_getVariableName(model, parameterVI));
  printf("set ylabel '%s'\n", ODEModel_getVariableName(model, parameter2VI));
  printf("splot '-' using 1:2:3 title '%s' with points pointtype 1 pointsize 1 palette\n",
	 ODEModel_getVariableName(model, speciesVI) );
        
  int error = 0 ;
  for (i = parameter;
       i <= parameterEnd;
       i += parameterStepSize)
  {
    for (j = parameter2;
	 j <= parameter2End;
	 j += parameter2StepSize)
    {
      /* add fourth parameter here */

      IntegratorInstance_reset( integratorInstance);
      RETURN_ON_ERRORS_WITH(1);

      IntegratorInstance_setVariableValue(integratorInstance,
					  parameterVI, i);
      IntegratorInstance_setVariableValue(integratorInstance,
					  parameter2VI, j);
      while(!IntegratorInstance_checkSteadyState(integratorInstance) &&
	    IntegratorInstance_getTime(integratorInstance) < 1e4 )
      {
	IntegratorInstance_integrateOneStep(integratorInstance);


	  
	if (SolverError_getNum(ERROR_ERROR_TYPE) ||
	    SolverError_getNum(FATAL_ERROR_TYPE))
	{
	  printf("ERROR at parameter 1 = %g, parameter 2 = %g\n", i, j);
	  DumpErrors();
	  error++;	  
	}

      }
/*       DumpState(integratorInstance, parameterVI, parameter2VI, speciesVI, species2VI); */
      DumpState(integratorInstance, parameterVI, parameter2VI, speciesVI);
    }
    /* printf("\n"); */

  }
      
    printf("end\n");
    if ( error ) printf("\t%d errors occured\n", error);
    IntegratorInstance_free(integratorInstance);
    VariableIndex_free(parameterVI);
    VariableIndex_free(parameter2VI);
    VariableIndex_free(speciesVI);
    ODEModel_free(model);
    CvodeSettings_free(settings);

    return 0;
  }

  int main (int argc, char *argv[])
  {
    int result = doit(argc, argv);
    DumpErrors();

    return result;
  }
