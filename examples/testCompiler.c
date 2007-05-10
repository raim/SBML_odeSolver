/*
  Last changed Time-stamp: <2007-05-10 23:19:21 raim>
  $Id: testCompiler.c,v 1.1 2007/05/10 21:21:09 raimc Exp $
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
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"



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
    integratorInstance_t *integratorInstance;
    char *modelStr, *parameterStr, *speciesStr;
    double endtime, relativeErrorTolerance,errorTolerance ;
    int printsteps, maximumIntegrationSteps;
    odeModel_t *model ;
        
    if (argc < 3)
    {
        fprintf(
            stderr,
            "usage %s sbml-model-file end-time print-steps\n",
            argv[0]);

        exit(0);
    }

    modelStr = argv[1];
    endtime = atoi(argv[2]);
    printsteps = atof(argv[3]);

    errorTolerance = 1e-20;
    relativeErrorTolerance = 1e-10;
    maximumIntegrationSteps = 500;

    model = ODEModel_createFromFile(modelStr);
    RETURN_ON_ERRORS_WITH(1);

    
    CvodeSettings_setTime(settings, endtime, printsteps);
    CvodeSettings_setError(settings, errorTolerance);    
    CvodeSettings_setRError(settings, relativeErrorTolerance);
    CvodeSettings_setMxstep(settings, maximumIntegrationSteps);     
    CvodeSettings_setJacobian(settings, 1);
    CvodeSettings_setStoreResults(settings, 0);

    
    CvodeSettings_setCompileFunctions(settings, 1); /* compile model */ 
    integratorInstance = IntegratorInstance_create(model, settings);

    printf("START WITH COMPILATION \n");
    IntegratorInstance_integrate(integratorInstance);
    IntegratorInstance_dumpData(integratorInstance);
    DumpErrors;
    printf("Integration time was %g\n",
	   IntegratorInstance_getIntegrationTime(integratorInstance));

    CvodeSettings_setCompileFunctions(settings, 0); /* don't compile model */ 
    IntegratorInstance_reset(integratorInstance);
    
    printf("\n\nAGAIN WITHOUT COMPILATION \n");
    IntegratorInstance_integrate(integratorInstance);
    IntegratorInstance_dumpData(integratorInstance);
    DumpErrors;
    printf("FINISHED WITH SAME RESULTS??\n");
    printf("Integration time was %g\n",
	   IntegratorInstance_getIntegrationTime(integratorInstance));
    
    IntegratorInstance_free(integratorInstance);
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
