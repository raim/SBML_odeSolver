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
    cvodeSettings_t *settings, *set2;
    variableIndex_t *s1, *s2;
    integratorInstance_t *integratorInstance;

    odeModel_t *model = ODEModel_createFromFile("basic-model1-forward-l2.xml", 1);
    RETURN_ON_ERRORS_WITH(1)

    s1 = ODEModel_getVariableIndex(model, "S1");
    s2 = ODEModel_getVariableIndex(model, "S2");

    /* Creating settings with default values */
    settings = CvodeSettings_create();
    
    /* Setting end time to .1, number of time steps to 1 and NULL
       instead of an optional predefined time series (double *); due
       to Indefinitely == 1, Printstep 5 will be ignored and Time =
       0.1 will be used as timestep for infinite integration */
    CvodeSettings_setTime(settings, .1, 5);

    /* Setting Cvode Parameters: absolute tolerance, relative
       tolerance and maximal internal step */
    CvodeSettings_setErrors(settings, 1e-18, 1e-14, 500);
    
    /* Setting Integration Switches:
       
    settings->UseJacobian = 1;  toggle use of Jacobian ASTs (1) or
                                internal approximation  (0)
    settings->Indefinitely = 1; run without a defined end time, Time
			        field contains step duration, ignore
			        PrintStep field
    settings->HaltOnEvent = 0;  doesn't stops integration upon an event
    settings->SteadyState = 0;  don't stop integration upon a steady state 
    settings->StoreResults = 0; don't Store time course history
    */

    CvodeSettings_setSwitches(settings, 1, 1, 0, 0, 0);


    integratorInstance = IntegratorInstance_create(model, settings);
    RETURN_ON_ERRORS_WITH(1);

    DumpState(integratorInstance, s1, s2);
    
    for (i=0; i != 12; i++)
    {

        IntegratorInstance_integrateOneStep(integratorInstance);
        RETURN_ON_ERRORS_WITH(1);

        DumpState(integratorInstance, s1, s2);
    }

    /* now, let's try again, with different settings */
    printf("now, let's try again, with different settings:\n");
    set2 = CvodeSettings_create();
    CvodeSettings_setTime(set2, .1, 5);
    CvodeSettings_setErrors(set2, 1e-18, 1e-14, 500);
    CvodeSettings_setSwitches(set2, 1, 0, 0, 0, 0);

    IntegratorInstance_set(integratorInstance, set2);
    

    while( !IntegratorInstance_timeCourseCompleted(integratorInstance) ) {

        IntegratorInstance_integrateOneStep(integratorInstance);
        RETURN_ON_ERRORS_WITH(1);

        DumpState(integratorInstance, s1, s2);
    }
    

    IntegratorInstance_free(integratorInstance);
    ODEModel_free(model);
    VariableIndex_free(s1);
    VariableIndex_free(s2);
    CvodeSettings_free(settings);
    CvodeSettings_free(set2);
    
    return 0;
}

int main (int argc, char *argv[])
{
    int result = doIt();

    SolverError_dump();

    return result ;
}
