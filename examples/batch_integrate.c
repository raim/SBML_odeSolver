/*
  Last changed Time-stamp: <2005-12-12 15:25:35 raim>
  $Id: batch_integrate.c,v 1.18 2005/12/12 14:40:53 raimc Exp $
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
#include <string.h>

#include "../src/sbmlsolver/odeSolver.h"


int
main (int argc, char *argv[]){
  int i, j;
  char *model, *parameter, *reaction;
  double start, end, steps, value;
  double time ;
  double printstep;

  /* libSBML types */
  SBMLDocument_t *d;
  SBMLReader_t *sr;

  /* SOSlib types */
  cvodeSettings_t *set;
  varySettings_t *vs;
  SBMLResultsMatrix_t *resM;
  SBMLResults_t *results;
 
  /* parsing command-line arguments */
  if (argc < 8 ) {
    fprintf(stderr,
	    "usage %s sbml-model-file simulation-time time-steps"
	    " start-value end-value step-number parameter-id"
	    " [optional reaction-id]\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  model = argv[1];
  time = atof(argv[2]);
  printstep = atoi(argv[3]);
  
  parameter = argv[7];
  start = atof(argv[4]);
  end = atof(argv[5]);
  steps = atoi(argv[6]);
  
  if ( argc > 8 ) {
      reaction = argv[8];
  }
  else{
    reaction = NULL;
  }
  
  printf("### Varying parameter %s (reaction %s) from %f to %f in %f steps\n",
	 parameter, reaction, start, end, steps);
  
  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);  

  /* Setting SBML ODE Solver integration parameters with default values */
  set = CvodeSettings_create();
  /* resetting the values we need */
  CvodeSettings_setTime(set, time, printstep);
  CvodeSettings_setErrors(set, 1e-18, 1e-10, 10000);
  CvodeSettings_setSwitches(set, 1, 0, 1, 1, 1, 0, 0); 
  CvodeSettings_setSteadyState(set, 1); 

  /* Setting SBML Ode Solver batch integration parameters */
  vs = VarySettings_allocate(1, steps+1);
  VarySettings_addParameter(vs, parameter, reaction, start, end);
  VarySettings_dump(vs);

  /* calling the SBML ODE Solver Batch function,
     and retrieving SBMLResults */
  resM = SBML_odeSolverBatch(d, set, vs);

  if ( resM == NULL ) {
    printf("### Parameter variation not succesful!\n");
    return(0);
  }
    /* we don't need these anymore */
  CvodeSettings_free(set);  
  SBMLDocument_free(d);
  VarySettings_free(vs);

  results = resM->results[0][0];

  for ( i=0; i<resM->i; i++ ) {
    for ( j=0; j<resM->j; j++ ) {
      results = SBMLResultsMatrix_getResults(resM, i, j);
      printf("### RESULTS Parameter %d, Step %d \n", i+1, j+1);
      /* printing results only for species*/
      SBMLResults_dumpSpecies(results);
    }
  }

  /* SolverError_dumpAndClearErrors(); */
  SBMLResultsMatrix_free(resM);
  return (EXIT_SUCCESS);  
}


/* End of file */
