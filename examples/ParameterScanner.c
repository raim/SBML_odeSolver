#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

void DumpState(
    integratorInstance_t *ii, double parameter, variableIndex_t *v)
{
    printf(
        " %g %g %g\n", 
        IntegratorInstance_getTime(ii),
        parameter,
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
    int i;
    cvodeSettings_t settings;
    variableIndex_t *speciesVI, *parameterVI;
    integratorInstance_t *integratorInstance;
    char *modelStr, *parameterStr, *speciesStr;
    double parameter, timeStepLength, parameterEnd, parameterStepSize,
        errorTolerance, relativeErrorTolerance;
    int numberOfTimeSteps, maximumIntegrationSteps;
    char *parameterArray[2];
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

    parameterArray[0] = parameterStr ;
    parameterArray[1] = NULL ;

    model = ODEModel_createWithSelectiveReplacement(modelStr, parameterArray);
    RETURN_ON_ERRORS_WITH(1);

    speciesVI = ODEModel_getVariableIndex(model, speciesStr);
    parameterVI = ODEModel_getVariableIndex(model, parameterStr);
    RETURN_ON_ERRORS_WITH(1);
    
    settings.Time = timeStepLength;          /*  the step size - Indefinitely == 1 */
    settings.Error = errorTolerance;         /* absolute tolerance in Cvode integration */
    settings.RError = relativeErrorTolerance;        /* relative tolerance in Cvode integration */
    settings.Mxstep = maximumIntegrationSteps;        /* maximum step number for CVode integration */
    settings.Indefinitely = 1;     /* run without a defined end time, Time field contains step duration, ignore PrintStep field*/
    settings.HaltOnEvent = 0;      /* doesn't stops integration upon an event */
    settings.SteadyState = 0;      /* doesn't stop integration upon a steady state */
    settings.UseJacobian = 1;      /* Toggle use of Jacobian ASTs or approximation */
    settings.StoreResults = 0;     /* don't Store time course history */
    settings.EnableVariableChanges = 1; /* allow modification of variables between integration steps. */

    printf("set xlabel 'time'\n");
    printf("set ylabel '%s'\n", parameterStr);
    printf("splot '-' using 1:2:3 title '%s' with lines\n", speciesStr);
        
    for (; parameter <= parameterEnd; parameter += parameterStepSize)
    {
        int error = 0 ;

        integratorInstance = IntegratorInstance_create(model, &settings);
        RETURN_ON_ERRORS_WITH(1);

        IntegratorInstance_setVariableValue(integratorInstance, parameterVI, parameter);
        DumpState(integratorInstance, parameter, speciesVI);

        for (i=0; i != numberOfTimeSteps && !error; i++)
        {
            IntegratorInstance_integrateOneStep(integratorInstance);
            
            if (SolverError_getNum(ERROR_ERROR_TYPE) || SolverError_getNum(FATAL_ERROR_TYPE))
            {
                printf("Parameter = %g\n", parameter);
                DumpErrors();
                error = 1;
            }
            else
                DumpState(integratorInstance, parameter, speciesVI);
        }
        IntegratorInstance_free(integratorInstance);
        printf("\n");
    }

    printf("end\n");
    VariableIndex_free(parameterVI);
    VariableIndex_free(speciesVI);
    ODEModel_free(model);

    return 0;
}

int main (int argc, char *argv[])
{
    int result = doit(argc, argv);
    DumpErrors();

    return result;
}
