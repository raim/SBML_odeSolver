#include <stdio.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

void DumpState(integratorInstance_t *ii, variableIndex_t *v1, variableIndex_t *v2)
{
    printf(
        " %g %g %g\n", 
        IntegratorInstance_getTime(ii),
        IntegratorInstance_getVariableValue(ii, v1),
        IntegratorInstance_getVariableValue(ii, v2));
}

int doIt(void)
{
    int i ;
    cvodeSettings_t settings;
    variableIndex_t *s1, *s2;
    integratorInstance_t *integratorInstance;

    odeModel_t *model = ODEModel_create("c:\\models\\basic-model1-forward-l2.xml");
    RETURN_ON_ERRORS_WITH(1)

    s1 = ODEModel_getVariableIndex(model, "S1");
    s2 = ODEModel_getVariableIndex(model, "S2");

    settings.Time = 0.1;          /*  the step size - Indefinitely == 1 */
    settings.Error = 1e-20;         /* absolute tolerance in Cvode integration */
    settings.RError = 1e-20;        /* relative tolerance in Cvode integration */
    settings.Mxstep = 500;        /* maximum step number for CVode integration */
    settings.Indefinitely = 1;     /* run without a defined end time, Time field contains step duration, ignore PrintStep field*/

    settings.HaltOnEvent = 0;      /* doesn't stops integration upon an event */
    settings.SteadyState = 0;      /* doesn't stop integration upon a steady state */
    settings.UseJacobian = 1;      /* Toggle use of Jacobian ASTs or approximation */
    settings.StoreResults = 0;     /* don't Store time course history */
    settings.EnableVariableChanges = 1; /* optimize excution without allowing modification of variables
                                            between integration steps. */

    integratorInstance = IntegratorInstance_create(model, &settings);
    RETURN_ON_ERRORS_WITH(1);

    DumpState(integratorInstance, s1, s2);
    
    for (i=0; i != 50; i++)
    {
        IntegratorInstance_integrateOneStep(integratorInstance);
        RETURN_ON_ERRORS_WITH(1);

        DumpState(integratorInstance, s1, s2);
    }

    IntegratorInstance_free(integratorInstance);
    VariableIndex_free(s1);
    VariableIndex_free(s2);
    ODEModel_free(model);

    return 0;
}

int main (int argc, char *argv[])
{
    int result = doIt();

    SolverError_dump();

    return result ;
}
