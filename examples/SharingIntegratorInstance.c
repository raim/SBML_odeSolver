#include <stdio.h>
#include <assert.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

void DumpState(
    integratorInstance_t *iia,
    integratorInstance_t *iib,
    variableIndex_t *v1, variableIndex_t *v2
    )
{
    printf(
        " %g %g %g %g %g\n", 
        IntegratorInstance_getTime(iia),
        IntegratorInstance_getVariableValue(iia, v1),
        IntegratorInstance_getVariableValue(iia, v2),
        IntegratorInstance_getVariableValue(iib, v1),
        IntegratorInstance_getVariableValue(iib, v2));
}

int doit(void)
{
    int i ;
    CvodeSettings settings ;
    variableIndex_t *s1, *s2;
    integratorInstance_t *integratorInstanceA;
    integratorInstance_t *integratorInstanceB;

    odeModel_t *model = ODEModel_create("c:\\models\\events-2-events-1-assignment-l2.xml");
    RETURN_ON_ERRORS_WITH(1);

    assert(ODEModel_hasVariable(model, "S1"));
    assert(ODEModel_hasVariable(model, "S1"));
    assert(!ODEModel_hasVariable(model, "foobar"));

    s1 = ODEModel_getVariableIndex(model, "S1");
    s2 = ODEModel_getVariableIndex(model, "S2");
    
    settings.Time = 0.01;          /*  the step size - Indefinitely == 1 */
    settings.Error = 1e-9;         /* absolute tolerance in Cvode integration */
    settings.RError = 1e-4;        /* relative tolerance in Cvode integration */
    settings.Mxstep = 500;        /* maximum step number for CVode integration */
    settings.Indefinitely = 1;     /* run without a defined end time, Time field contains step duration, ignore PrintStep field*/
    settings.PrintMessage = 0;     /* don't Print messages */
    settings.PrintOnTheFly = 0;    /* don't Print species concentration during integration */
    settings.HaltOnEvent = 0;      /* doesn't stops integration upon an event */
    settings.SteadyState = 0;      /* doesn't stop integration upon a steady state */
    settings.UseJacobian = 1;      /* Toggle use of Jacobian ASTs or approximation */
    settings.StoreResults = 0;     /* don't Store time course history */
    settings.EnableVariableChanges = 0; /* optimize excution without allowing modification of variables
                                            between integration steps. */

    integratorInstanceA = IntegratorInstance_create(model, &settings);
    RETURN_ON_ERRORS_WITH(1);

    integratorInstanceB = IntegratorInstance_create(model, &settings);
    RETURN_ON_ERRORS_WITH(1);

    DumpState(integratorInstanceA, integratorInstanceB, s1, s2);

    for (i=0; i != 500; i++)
    {
        IntegratorInstance_integrateOneStep(integratorInstanceA);
        RETURN_ON_ERRORS_WITH(1);

        IntegratorInstance_integrateOneStep(integratorInstanceB);
        RETURN_ON_ERRORS_WITH(1);

        DumpState(integratorInstanceA, integratorInstanceB, s1, s2);

    }

    IntegratorInstance_free(integratorInstanceA);
    IntegratorInstance_free(integratorInstanceB);
    VariableIndex_free(s1);
    VariableIndex_free(s2);
    ODEModel_free(model);

    return 0;
}

int main (int argc, char *argv[])
{
    int result = doit();

    SolverError_dump();

    return result;
}