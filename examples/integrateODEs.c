/*
  Last changed Time-stamp: <2006-10-14 07:07:07 raim>
  $Id: integrateODEs.c,v 1.2 2006/10/14 05:10:27 raimc Exp $
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
 *     Rainer Machne
 *
 * Contributor(s):
 */

#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>


int
main (int argc, char *argv[])
{
  int neq, nass, nconst;
  char *x[3];
  double x0[3];
  char *ode[2];
  ASTNode_t *ast[2];
  
  double time, rtol, atol;
  double printstep;

  /* SBML model containing events */
  Model_t *events;
  
  /* SOSlib types */
  odeModel_t *om;
  cvodeSettings_t *set;
  integratorInstance_t *ii;
  variableIndex_t *vi;

  /* parsing input ODEs and settings */
  /* time and error settings */
  time = 0.5;
  rtol = atol = 1.0E-9;
  printstep = 10;
  /* variables */
  neq = 2;
  x[0] = "C_cy"; 
  x0[0] = 0.0;
  ode[0] = "((150.0 * (3.8 - (p * D_cy) - (p * C_cy)) * (1.0 - (p * C_cy))) - (9.0 * C_cy))";
  x[1] = "D_cy";
  x0[1] = 9.0;
  ode[1] = "(3.8 - (3.0 * D_cy) - (p * D_cy) - (p * C_cy))";
  /* parameters */
  nconst = 1;
  x[2] = "p";
  x0[2] = 0.2;
  /* assignments */
  nass = 0;
  /* SBML model containing events */
  events = NULL;
  
  /* creating ODE ASTs from strings */
  ast[0] = SBML_parseFormula(ode[0]);
  ast[1] = SBML_parseFormula(ode[1]);

  /* end of input parsing */
  
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_createWithTime(time, printstep);
  CvodeSettings_setErrors(set, atol, rtol, 1e4);
  CvodeSettings_setStoreResults(set, 0);

  /* activating default sensitivity */
  CvodeSettings_setSensitivity(set, 1);
  
  /* creating odeModel */
  /* `events' could be an SBML model containing events */
  om = ODEModel_createFromODEs(ast, neq, nass, nconst, x, x0, events);

  /* creating integrator */
  ii = IntegratorInstance_create(om,set);

  /* integrate */
  printf("integrating:\n");
  while( !IntegratorInstance_timeCourseCompleted(ii) )
  {
    if ( !IntegratorInstance_integrateOneStep(ii) )
    {
      SolverError_dump();
      break;
    }
    else
      IntegratorInstance_dumpData(ii);    
  }

  /* Specific data interfaces: */
  /* names are stored in the odeModel and values 
     are independently stored in the integrator: you can
     get different values from different integrator instances
     built from the same model */
  vi = ODEModel_getVariableIndex(om, "C_cy");
  printf("\nVariable %s has final value of %g at time %g\n\n",
	 ODEModel_getVariableName(om, vi),
	 IntegratorInstance_getVariableValue(ii, vi),
	 IntegratorInstance_getTime(ii));
  VariableIndex_free(vi);

  vi = ODEModel_getVariableIndex(om, "p");
  printf("Sensitivies to %s:\n",  ODEModel_getVariableName(om, vi));
  IntegratorInstance_dumpPSensitivities(ii, vi);
  printf("\n\n");
  VariableIndex_free(vi);

  /* finished, free stuff */
  /* free ASTs */
  ASTNode_free(ast[0]);
  ASTNode_free(ast[1]);
  /* free solver structures */
  IntegratorInstance_free(ii);
  ODEModel_free(om);  
  CvodeSettings_free(set);
  
  return (EXIT_SUCCESS);  
}



/* End of file */
