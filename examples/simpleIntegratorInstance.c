/*
  Last changed Time-stamp: <2007-12-06 19:48:05 raim>
  $Id: simpleIntegratorInstance.c,v 1.17 2008/03/10 19:24:47 raimc Exp $
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
 *     Rainer Machne
 */

#include <stdio.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

int doIt(void)
{
    int i ;
    cvodeSettings_t *settings, *set2;
    variableIndex_t *s1, *s2;
    integratorInstance_t *integratorInstance;

    odeModel_t *model =
      ODEModel_createFromFile("basic-model1-forward-l2.xml");
    
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
       tolerance and maximal internal step, respectively */
    CvodeSettings_setErrors(settings, 1e-20, 1e-14, 500);
    
    /* Setting Integration Switches */
    CvodeSettings_setMethod(settings, 1, 12);
    CvodeSettings_setJacobian(settings, 1);
    CvodeSettings_setIndefinitely(settings, 1);
    CvodeSettings_setHaltOnEvent(settings, 0);
    CvodeSettings_setHaltOnSteadyState(settings, 0);
    CvodeSettings_setStoreResults(settings, 0);
    CvodeSettings_setSensitivity(settings, 0);

    /* first integration run */
    printf("\nFIRST INTEGRATION RUN WITH:\n");
    CvodeSettings_dump(settings);
    integratorInstance = IntegratorInstance_create(model, settings);
    RETURN_ON_ERRORS_WITH(1);
    IntegratorInstance_setInitialTime(integratorInstance, .05);
    /* print variable (ODE, assignments) and constant names */
    printf("#time  ");
    IntegratorInstance_dumpNames(integratorInstance);
    /* print initial conditions and parameters */
    IntegratorInstance_dumpData(integratorInstance);
    
    for (i=0; i != 12; i++)
    {
        /* setting the next time step, uncomment the following
	   function call to see its effect */
     /* IntegratorInstance_setNextTimeStep(integratorInstance, 0.2*(i+1)); */

        IntegratorInstance_integrateOneStep(integratorInstance);
        RETURN_ON_ERRORS_WITH(1);
	/* print current data */
	IntegratorInstance_dumpData(integratorInstance);
    }

    /* now, let's try again, with different settings */

    set2 = CvodeSettings_create();
    /* as Indefinitely will be set to 0, a finite integration
       to time 1.2 in 6 steps will be run */
    CvodeSettings_setTime(set2, 1.2, 6);
    CvodeSettings_setErrors(set2, 1e-16, 1e-14, 500);
    /* switches can be set all together, same order as above */
    CvodeSettings_setSwitches(set2, 1, 0, 0, 0, 0, 0, 0);
    
    printf("\nNOW, LET'S TRY AGAIN WITH DIFFERENT SETTINGS:\n");
    CvodeSettings_dump(set2);
    
    IntegratorInstance_set(integratorInstance, set2);
    IntegratorInstance_setInitialTime(integratorInstance, .5);

    IntegratorInstance_dumpData(integratorInstance);  

    while( !IntegratorInstance_timeCourseCompleted(integratorInstance) ) {

        IntegratorInstance_integrateOneStep(integratorInstance);
        RETURN_ON_ERRORS_WITH(1);

 	IntegratorInstance_dumpData(integratorInstance);
    }
    
    printf("\n\nFINISHED SUCCESSFULLY!\n");
    printf("Please, note the different values e.g. at time 1.2.\n");
    printf("The values for the first run a more exact, due to the much\n");
    printf("lower error tolerances. The error tolerances have to be\n");
    printf("adapted to the ranges of each model!!\n\n");

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
