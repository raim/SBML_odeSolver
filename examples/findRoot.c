/*
  Last changed Time-stamp: <2006-03-07 16:52:51 raim>
  $Id: findRoot.c,v 1.3 2006/03/07 15:58:35 raimc Exp $
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
#include <sbmlsolver/nullSolver.h>


int
main (int argc, char *argv[]){

  char model[256];
  
  SBMLDocument_t *d;
  SBMLReader_t *sr;
  Model_t *m;

  cvodeSettings_t *set;
  odeModel_t *om;
  integratorInstance_t *ii;
  
   
  sscanf(argv[1], "%s", model);

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);
  m = SBMLDocument_getModel(d);

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();

  om = ODEModel_create(m);
  ii = IntegratorInstance_create(om, set);
  /**!!! the KINSOL implementation in SOSlib is experimental,
         and can't be used in software but feel free to experiment.
	 Source code is in src/nullSolver.c !!!**/
  IntegratorInstance_nullSolver(ii);
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }

  /* now we have the results and can free the inputs */
  /* IntegratorInstance_free(ii); */
  CvodeSettings_free(set);
  ODEModel_free(om);
  SBMLDocument_free(d);

  /* print results */
  SolverError_dumpAndClearErrors();

  return (EXIT_SUCCESS);  
}



/* End of file */
