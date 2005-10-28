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
    cvodeSettings_t *settings ;
    variableIndex_t *s1, *s2;
    integratorInstance_t *integratorInstanceA;
    integratorInstance_t *integratorInstanceB;

    odeModel_t *model = ODEModel_createFromFile("c:\\models\\events-2-events-1-assignment-l2.xml");
    RETURN_ON_ERRORS_WITH(1);

    assert(ODEModel_hasVariable(model, "S1"));
    assert(ODEModel_hasVariable(model, "S1"));
    assert(!ODEModel_hasVariable(model, "foobar"));

    s1 = ODEModel_getVariableIndex(model, "S1");
    s2 = ODEModel_getVariableIndex(model, "S2");
    
    /* Creating settings with default values */
    settings = CvodeSettings_create();

    
    /* Setting end time to .1, number of time steps to 1 and NULL
       instead of an optional predefined time series (double *); due
       to Indefinitely == 1, Printstep 1 will be ignored and Time =
       0.1 will be used as timestep for infinite integration */
    CvodeSettings_setTime(settings, .01, 1);

    /* Setting Cvode Parameters: absolute and relative tolerances and
       maximal internal step */
    CvodeSettings_setErrors(settings, 1e-18, 1e-14, 500);

    
    /* Setting Integration Switches: see documentation or
       example simpleIntegratorInstance.c for details on
       the passed values */
    CvodeSettings_setSwitches(settings, 1, 1, 0, 0, 0, 0);

    integratorInstanceA = IntegratorInstance_create(model, settings);
    RETURN_ON_ERRORS_WITH(1);

    integratorInstanceB = IntegratorInstance_create(model, settings);
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
    CvodeSettings_free(settings);

    return 0;
}

int main (int argc, char *argv[])
{
    int result = doit();

    SolverError_dump();

    return result;
}