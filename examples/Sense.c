/*
  Last changed Time-stamp: <2008-03-10 16:38:11 raim>
  $Id: Sense.c,v 1.5 2008/03/10 19:24:47 raimc Exp $
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
#include <ctype.h>
#include <string.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

void DumpErrors()
{
    char *errors = SolverError_dumpToString();

    fprintf(stderr, errors);
    SolverError_freeDumpString(errors);
    SolverError_clear();
}

void printGraph(
    cvodeResults_t *results,
    int parameterMode,
    int numOfSymbols,
    char *symbol,
    char **symbols,
    variableIndex_t *symbolVI,
    variableIndex_t **symbolsVI) 
{
    int i, j ;

    printf("set xlabel 'time'\n");

    printf(
        "set ylabel 'Sensitivity of %s to %s'\n",
        parameterMode ? "Variable" : symbol,
        parameterMode ? symbol : "Parameter");

    printf("plot ");    
    for (i = 0; i != numOfSymbols; i++)
    {
       printf("'-' using 1:2 title '%s' with lines", symbols[i]);
       if (i != numOfSymbols - 1)
           printf(", ");
    }
    printf("\n");


    for (j= 0; j != numOfSymbols; j++)
    {
        printf("\n#time %s\n", symbols[j]);

        for (i = 0; i != CvodeResults_getNout(results); i++)
        {
            printf("%g", CvodeResults_getTime(results, i));
            printf(
                " %g",
                CvodeResults_getSensitivity(
                    results,
                    parameterMode ? symbolsVI[j] : symbolVI,
                    parameterMode ? symbolVI : symbolsVI[j],
                    i));
            printf("\n");
        }
        printf("e\n");
    }
}

int doit(int argc, char *argv[])
{
    int i, parameterMode ;
    cvodeSettings_t *settings = CvodeSettings_create();
    variableIndex_t *symbolVI, **symbolsVI;
    integratorInstance_t *integratorInstance;
    char *modelStr, *symbol, **symbols;
    double timeStepLength, errorTolerance, relativeErrorTolerance;
    int numOfSymbols = 0, numberOfTimeSteps, maximumIntegrationSteps;
    odeModel_t *model ;
    cvodeResults_t *results;
        
    if (argc > 4)
    {
        if (strcmp("p", argv[4]) == 0) 
            parameterMode = 1 ;
        else if (strcmp("v", argv[4]) == 0)
            parameterMode = 0 ;
        else
            parameterMode = -1;
    }

    if (argc < 6 || parameterMode == -1)
    {
        fprintf(
            stderr,
            "usage %s sbml-model-file time-steps time-step-length mode symbol [symbols] [error-tolerance] [relative-error-tolerance] [maximum integration steps]\nmode is either 'p' or 'v'\n",
            argv[0]);

        exit(0);
    }

    modelStr = argv[1];
    numberOfTimeSteps = atoi(argv[2]);
    timeStepLength = atof(argv[3]);
    symbol = argv[5];

    model = ODEModel_createFromFile(modelStr);
    RETURN_ON_ERRORS_WITH(1);

    symbolVI = ODEModel_getVariableIndex(model, symbol);
    RETURN_ON_ERRORS_WITH(1);
    
    while (argc > 6 + numOfSymbols && !isdigit(argv[6 + numOfSymbols][0]))
        numOfSymbols++ ;

    symbolsVI = (variableIndex_t **)calloc(sizeof(variableIndex_t *), numOfSymbols);
    symbols = (char **)calloc(sizeof(char *), numOfSymbols);

    for (i = 0; i != numOfSymbols; i++)
    {
        symbols[i] = argv[6 + i];
        symbolsVI[i] = ODEModel_getVariableIndex(model, symbols[i]);
        RETURN_ON_ERRORS_WITH(1);
    }

    if (argc > 6 + numOfSymbols)
        errorTolerance = atof(argv[6 + numOfSymbols]);
    else
        errorTolerance = 1e-9;

    if (argc > 7 + numOfSymbols)
        relativeErrorTolerance = atof(argv[7 + numOfSymbols]);
    else
        relativeErrorTolerance = 1e-4;

    if (argc > 8 + numOfSymbols)
        maximumIntegrationSteps = atoi(argv[8 + numOfSymbols]);
    else
        maximumIntegrationSteps = 500;

    CvodeSettings_setTime(settings, timeStepLength * numberOfTimeSteps, numberOfTimeSteps);          
    CvodeSettings_setError(settings, errorTolerance);         /* absolute tolerance in Cvode integration */
    CvodeSettings_setRError(settings, relativeErrorTolerance);        /* relative tolerance in Cvode integration */
    CvodeSettings_setMxstep(settings, maximumIntegrationSteps);        /* maximum step number for CVode integration */
    CvodeSettings_setHaltOnEvent(settings, 0);      /* doesn't stop integration upon an event */
    CvodeSettings_setHaltOnSteadyState(settings, 0);      /* doesn't stop integration upon a steady state */
    CvodeSettings_setJacobian(settings, 1);      /* Toggle use of Jacobian ASTs or approximation */
    CvodeSettings_setStoreResults(settings, 1);     /* don't Store time course history */
    CvodeSettings_setSensitivity(settings, 1); /* switch on sensitivity */
    CvodeSettings_setSensMethod(settings, 0);
    CvodeSettings_setCompileFunctions(settings, 1);

    integratorInstance = IntegratorInstance_create(model, settings);
    RETURN_ON_ERRORS_WITH(1);

    while (!IntegratorInstance_timeCourseCompleted(integratorInstance))
    {
        IntegratorInstance_integrateOneStep(integratorInstance);
        RETURN_ON_ERRORS_WITH(1);
    }

    results = IntegratorInstance_createResults(integratorInstance);

    printGraph(results, parameterMode, numOfSymbols, symbol, symbols, symbolVI, symbolsVI);

    for (i=0; i != numOfSymbols; i++)
        VariableIndex_free(symbolsVI[i]);

    free(symbols);
    free(symbolsVI);
    VariableIndex_free(symbolVI);
    CvodeResults_free(results);
    IntegratorInstance_free(integratorInstance);
    CvodeSettings_free(settings);
    ODEModel_free(model);

    return 0;
}

int main (int argc, char *argv[])
{
    int result = doit(argc, argv);
    DumpErrors();

    return result;
}
