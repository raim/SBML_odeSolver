/*
  Last changed Time-stamp: <2008-03-10 16:37:28 raim>
  $Id: ChangingParameterIntegrator.c,v 1.8 2008/03/10 19:24:47 raimc Exp $
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
#include "sbmlsolver/solverError.h"

void DumpState(
    integratorInstance_t *iia,
    variableIndex_t *v1, variableIndex_t *v2
    )
{
    printf(
        " %g %g %g\n", 
        IntegratorInstance_getTime(iia),
        IntegratorInstance_getVariableValue(iia, v1),
        IntegratorInstance_getVariableValue(iia, v2));
}

int doit(void)
{
    int i ;
    cvodeSettings_t *settings ;
    variableIndex_t *s1, *s2, *k1;
    integratorInstance_t *integratorInstanceA;
    odeModel_t *model = ODEModel_createFromFile("basic.xml");
    RETURN_ON_ERRORS_WITH(1);

    s1 = ODEModel_getVariableIndex(model, "S1");
    s2 = ODEModel_getVariableIndex(model, "S2");
    k1 = ODEModel_getVariableIndex(model, "k_1");
    RETURN_ON_ERRORS_WITH(1);
    
    /* Creating settings with default values */
    settings = CvodeSettings_create();

    
    /* Setting end time to .1, number of time steps to 1 and NULL
       instead of an optional predefined time series (double *); due
       to Indefinitely == 1, Printstep 1 will be ignored and Time =
       0.1 will be used as timestep for infinite integration */
    CvodeSettings_setTime(settings, .1, 1);

    /* Setting Cvode Parameters: absolute and relative tolerances and
       maximal internal step */
    CvodeSettings_setErrors(settings, 1e-18, 1e-14, 500);

    
    /* Setting Integration Switches: see documentation or
       example simpleIntegratorInstance.c for details on
       the passed values */
    CvodeSettings_setSwitches(settings, 1, 1, 0, 0, 0, 0, 0);

    CvodeSettings_setCompileFunctions(settings, 1); /* compile model */

    integratorInstanceA = IntegratorInstance_create(model, settings);
    RETURN_ON_ERRORS_WITH(1);

    DumpState(integratorInstanceA, s1, s2);

    for (i=0; i != 50; i++)
    {
        IntegratorInstance_integrateOneStep(integratorInstanceA);
        RETURN_ON_ERRORS_WITH(1);

        if (IntegratorInstance_getVariableValue(integratorInstanceA, s1) < 7.5e-16)
            IntegratorInstance_setVariableValue(integratorInstanceA, k1, 2.0);

        DumpState(integratorInstanceA,  s1, s2);

    }

    IntegratorInstance_free(integratorInstanceA);
    VariableIndex_free(s1);
    VariableIndex_free(s2);
    ODEModel_free(model);
    CvodeSettings_free(settings);

    return 0;
}

int main (int argc, char *argv[])
{
    int result = doit();
    char *errors = SolverError_dumpToString();

    fprintf(stderr, errors);
    SolverError_freeDumpString(errors);

    return result;
}
