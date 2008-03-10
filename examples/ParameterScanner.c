/*
  Last changed Time-stamp: <2008-03-10 16:37:42 raim>
  $Id: ParameterScanner.c,v 1.16 2008/03/10 19:24:47 raimc Exp $
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
#include <math.h>
#include <stdlib.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

void DumpState(
    integratorInstance_t *ii, variableIndex_t *p, variableIndex_t *v)
{
    printf(
        " %g %g %g\n", 
        IntegratorInstance_getTime(ii),
        IntegratorInstance_getVariableValue(ii, p),
        IntegratorInstance_getVariableValue(ii, v));
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
    int i ;
    cvodeSettings_t *settings = CvodeSettings_create();
    variableIndex_t *speciesVI, *parameterVI;
    integratorInstance_t *integratorInstance;
    char *modelStr, *parameterStr, *speciesStr;
    double parameter, timeStepLength, parameterEnd, parameterStepSize,
        errorTolerance, relativeErrorTolerance;
    int numberOfTimeSteps, maximumIntegrationSteps;
    odeModel_t *model ;
        
    if (argc < 9)
    {
        fprintf(
            stderr,
            "usage %s sbml-model-file time-steps time-step-length parameter parameter-start parameter-end parameter-step species [error-tolerance] [relative-error-tolerance] [maximum integration steps]\n",
            argv[0]);

        exit(0);
    }

    modelStr = argv[1];
    numberOfTimeSteps = atoi(argv[2]);
    timeStepLength = atof(argv[3]);
    parameterStr = argv[4];
    parameter = atof(argv[5]);
    parameterEnd = atof(argv[6]);
    parameterStepSize = atof(argv[7]);
    speciesStr = argv[8];

    if (argc > 9)
        errorTolerance = atof(argv[9]);
    else
        errorTolerance = 1e-9;

    if (argc > 10)
        relativeErrorTolerance = atof(argv[10]);
    else
        relativeErrorTolerance = 1e-4;

    if (argc > 11)
        maximumIntegrationSteps = atoi(argv[11]);
    else
        maximumIntegrationSteps = 500;

    model = ODEModel_createFromFile(modelStr);
    RETURN_ON_ERRORS_WITH(1);

    speciesVI = ODEModel_getVariableIndex(model, speciesStr);
    parameterVI = ODEModel_getVariableIndex(model, parameterStr);
    RETURN_ON_ERRORS_WITH(1);
    
    CvodeSettings_setIndefinitely(settings, 1);    /* run without a defined end time */
    CvodeSettings_setTime(settings, timeStepLength, 1);          /*  end time is the step size - Indefinitely == 1 */
    CvodeSettings_setError(settings, errorTolerance);         /* absolute tolerance in Cvode integration */
    CvodeSettings_setRError(settings, relativeErrorTolerance);        /* relative tolerance in Cvode integration */
    CvodeSettings_setMxstep(settings, maximumIntegrationSteps);        /* maximum step number for CVode integration */
    CvodeSettings_setHaltOnEvent(settings, 0);      /* doesn't stop integration upon an event */
    CvodeSettings_setHaltOnSteadyState(settings, 0);      /* doesn't stop integration upon a steady state */
    CvodeSettings_setJacobian(settings, 1);      /* Toggle use of Jacobian ASTs or approximation */
    CvodeSettings_setStoreResults(settings, 0);     /* don't Store time course history */
    CvodeSettings_setCompileFunctions(settings, 1); /* compile model */ 

    
    integratorInstance = IntegratorInstance_create(model, settings);
    
    printf("set xlabel 'time'\n");
    printf("set ylabel '%s'\n", parameterStr);
    printf("splot '-' using 1:2:3 title '%s' with lines\n", speciesStr);
        
    for (; parameter <= parameterEnd; parameter += parameterStepSize)
    {
        int error = 0 ;

        IntegratorInstance_reset( integratorInstance);
        RETURN_ON_ERRORS_WITH(1);

        IntegratorInstance_setVariableValue(integratorInstance,
					    parameterVI, parameter); 
        DumpState(integratorInstance, parameterVI, speciesVI);

        for (i=0; i != numberOfTimeSteps && !error; i++)
        {
            IntegratorInstance_integrateOneStep(integratorInstance);
            
            if (SolverError_getNum(ERROR_ERROR_TYPE) ||
		SolverError_getNum(FATAL_ERROR_TYPE))
            {
                printf("Parameter = %g\n", parameter);
                DumpErrors();
                error = 1;
            }
            else
                DumpState(integratorInstance, parameterVI, speciesVI);
        }
        
        printf("\n");
    }

    printf("end\n");
    IntegratorInstance_free(integratorInstance);
    VariableIndex_free(parameterVI);
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
