/*
  Last changed Time-stamp: <2005-11-03 11:03:51 raim>
  $Id: TwinIntegratorInstance.c,v 1.4 2006/03/09 17:23:49 afinney Exp $
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

#include "sbmlsolver/integratorInstance.h"

void DumpState(
    integratorInstance_t *iia,
    variableIndex_t *v1, variableIndex_t *v2,
    integratorInstance_t *iib,
    variableIndex_t *v3, variableIndex_t *v4)
{
    printf(
        " %g %g %g %g %g\n", 
        IntegratorInstance_getTime(iia),
        IntegratorInstance_getVariableValue(iia, v1),
        IntegratorInstance_getVariableValue(iia, v2),
        IntegratorInstance_getVariableValue(iib, v3),
        IntegratorInstance_getVariableValue(iib, v4));
}

int main (int argc, char *argv[])
{
    int i ;
    odeModel_t *modelA = ODEModel_createFromFile("events-2-events-1-assignment-l2.xml");
    odeModel_t *modelB = ODEModel_createFromFile("events-1-event-1-assignment-l2.xml");
    variableIndex_t *s1a = ODEModel_getVariableIndex(modelA, "S1");
    variableIndex_t *s2a = ODEModel_getVariableIndex(modelA, "S2");
    variableIndex_t *s1b = ODEModel_getVariableIndex(modelB, "S1");
    variableIndex_t *s2b = ODEModel_getVariableIndex(modelB, "S2");
    integratorInstance_t *integratorInstanceA;
    integratorInstance_t *integratorInstanceB;
    
    cvodeSettings_t *settings ;

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
    CvodeSettings_setSwitches(settings, 1, 1, 0, 0, 0, 0, 0);

    CvodeSettings_setCompileFunctions(settings, 1);

    integratorInstanceA = IntegratorInstance_create(modelA, settings);
    integratorInstanceB = IntegratorInstance_create(modelB, settings);

    DumpState(integratorInstanceA, s1a, s2a, integratorInstanceB, s1b, s2b);

    for (i=0; i != 500; i++)
    {
        if (!IntegratorInstance_integrateOneStep(integratorInstanceA))
            return 1;

        if (!IntegratorInstance_integrateOneStep(integratorInstanceB))
            return 1;

        DumpState(integratorInstanceA, s1a, s2a, integratorInstanceB, s1b, s2b);
    }

    IntegratorInstance_free(integratorInstanceA);
    IntegratorInstance_free(integratorInstanceB);
    VariableIndex_free(s1a);
    VariableIndex_free(s2a);
    VariableIndex_free(s1b);
    VariableIndex_free(s2b);
    ODEModel_free(modelA);
    ODEModel_free(modelB);
    CvodeSettings_free(settings);

    return 0;
}
