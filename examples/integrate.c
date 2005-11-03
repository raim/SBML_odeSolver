/*
  Last changed Time-stamp: <2005-11-03 13:34:58 raim>
  $Id: integrate.c,v 1.10 2005/11/03 12:36:24 raimc Exp $
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
main (int argc, char *argv[]){
  int i, j;
  char model[256];
  char sens[256];
  double time = 0.0;
  double printstep;
  
  SBMLDocument_t *d;
  SBMLReader_t *sr;
  Model_t *m;
  
  SBMLResults_t *results;
  timeCourse_t *tc;
  cvodeSettings_t *set;

   
  sscanf(argv[1], "%s", model);
  sscanf(argv[2], "%lf", &time);
  sscanf(argv[3], "%lf", &printstep);
  sscanf(argv[4], "%s", sens);

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);

  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, time, printstep);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1000);

  /* set sensitivity with default method */
  if ( strcmp(sens,"y") == 0 )
    CvodeSettings_setSensitivity(set, 1);
  
  /* calling the SBML ODE Solver which returns SBMLResults */  
  results = SBML_odeSolver(d, set);
  
  if ( SolverError_getNum(FATAL_ERROR_TYPE) ) {
    printf("Integration not sucessful!\n");
    SolverError_dumpAndClearErrors();
    return(EXIT_FAILURE);
  }

  /* now we have the results and can free the inputs */
  CvodeSettings_free(set);
  SBMLDocument_free(d);

  /* print results */
  printf("### RESULTS \n");  
  SBMLResults_dump(results);
  SolverError_dumpAndClearErrors();
  /* now we can also free the result structure */
  SBMLResults_free(results);

  return (EXIT_SUCCESS);  
}



/* End of file */
