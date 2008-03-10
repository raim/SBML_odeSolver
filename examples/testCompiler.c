/*
  Last changed Time-stamp: <2008-03-10 16:35:43 raim>
  $Id: testCompiler.c,v 1.7 2008/03/10 19:24:47 raimc Exp $
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
  cvodeSettings_t *settings = CvodeSettings_create();
  integratorInstance_t *ii;
  variableIndex_t *v1, *v2, *v3, *v4;
  char *modelStr;
  double endtime, relativeErrorTolerance,errorTolerance ;
  int printsteps, maximumIntegrationSteps;
  odeModel_t *model ;


  int sensi = 0;
  int nsens = 4;
  char *sensIDs[nsens];
    
  sensIDs[0] = "MAPK_PP";
  sensIDs[1] = "V1";
  sensIDs[2] = "MAPK_P";
  sensIDs[3] = "uVol";

        
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
  maximumIntegrationSteps = 50000;

  model = ODEModel_createFromFile(modelStr);
  RETURN_ON_ERRORS_WITH(1);

  v1 = ODEModel_getVariableIndex(model, sensIDs[0]);
  v2 = ODEModel_getVariableIndex(model, sensIDs[1]);
  v3 = ODEModel_getVariableIndex(model, sensIDs[2]);
  v4 = ODEModel_getVariableIndex(model, sensIDs[3]);

    
  CvodeSettings_setTime(settings, endtime, printsteps);
  CvodeSettings_setError(settings, errorTolerance);    
  CvodeSettings_setRError(settings, relativeErrorTolerance);
  CvodeSettings_setMxstep(settings, maximumIntegrationSteps);     
  CvodeSettings_setJacobian(settings, 1);
  CvodeSettings_setStoreResults(settings, 0);
  CvodeSettings_setSensitivity(settings, 1);

  if (sensi)
  {
    CvodeSettings_setSensitivity(settings, 1);
    CvodeSettings_setSensParams (settings,sensIDs, nsens );
  }
    
  CvodeSettings_setCompileFunctions(settings, 0); /* don't compile model */ 
  ii = IntegratorInstance_create(model, settings);

  printf("\n\nINTEGRATE WITHOUT COMPILATION \n");
  IntegratorInstance_integrate(ii);
  IntegratorInstance_dumpData(ii);

  IntegratorInstance_dumpPSensitivities(ii, v1);
  IntegratorInstance_dumpPSensitivities(ii, v2);
  IntegratorInstance_dumpPSensitivities(ii, v3);
  IntegratorInstance_dumpPSensitivities(ii, v4);
  
    
  DumpErrors();
  printf("Integration time was %g\n",
	 IntegratorInstance_getIntegrationTime(ii));

  /* testing combinations of difference quotient and exact calculation
   of jacobian and parametric matrices */
  CvodeSettings_setJacobian(settings, 1);
  /* CvodeSettings_setSensitivity(settings, 1); */
  /*!!! sensitivity can't be switched off! but compilation is??? */

  CvodeSettings_setCompileFunctions(settings, 1); /* compile model */
  
  IntegratorInstance_reset(ii);
    
  printf("\n\nAGAIN WITH COMPILATION \n");
  IntegratorInstance_integrate(ii);
  IntegratorInstance_dumpData(ii);

  IntegratorInstance_dumpPSensitivities(ii, v1);
  IntegratorInstance_dumpPSensitivities(ii, v2);
  IntegratorInstance_dumpPSensitivities(ii, v3);
  IntegratorInstance_dumpPSensitivities(ii, v4);


  DumpErrors();
  printf("FINISHED WITH SAME RESULTS??\n");
  printf("Integration time was %g\n",
	 IntegratorInstance_getIntegrationTime(ii));

  VariableIndex_free(v1);
  VariableIndex_free(v2);
  VariableIndex_free(v3);
  VariableIndex_free(v4);
  IntegratorInstance_free(ii);
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
