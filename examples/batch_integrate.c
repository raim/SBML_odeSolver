/*
  Last changed Time-stamp: <2008-03-10 20:13:28 raim>
  $Id: batch_integrate.c,v 1.22 2008/03/10 19:24:29 raimc Exp $
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
  char *model, *parameter1, *parameter2, *reaction2;
  double start1, end1, steps1;
  double start2, end2, steps2;
  double time ;
  double printstep;
  double values[2];

  /* libSBML types */
  SBMLDocument_t *d;
  SBMLReader_t *sr;

  /* SOSlib types */
  cvodeSettings_t *set;
  varySettings_t *vs;
  SBMLResultsArray_t *resM;
  SBMLResults_t *results;
 
  /* parsing command-line arguments */
  if (argc < 8 ) {
    fprintf(stderr,
	    "usage %s sbml-model-file simulation-time time-steps"
	    " start-value1 end-value1 step-number1 parameter-id1"
	    " start-value2 end-value2 step-number2 parameter-id2"
	    " [optional reaction-id for param2]\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  model = argv[1];
  time = atof(argv[2]);
  printstep = atoi(argv[3]);
  
  start1 = atof(argv[4]);
  end1 = atof(argv[5]);
  steps1 = atoi(argv[6]);
  parameter1 = argv[7];
   

  printf("### Varying parameter %s from %f to %f in %f steps\n",
	 parameter1, start1, end1, steps1);

  start2 = atof(argv[8]);
  end2 = atof(argv[9]);
  steps2 = atoi(argv[10]);
  parameter2 = argv[11];
  
  if ( argc > 12 ) 
    reaction2 = argv[12];
  else
    reaction2 = NULL;
  
  printf("### Varying parameter %s (reaction %s) from %f to %f in %f steps\n",
	 parameter2, reaction2, start2, end2, steps2);
  
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
  CvodeSettings_setHaltOnSteadyState(set, 1); 

  /* Setting SBML Ode Solver batch integration parameters */
  vs = VarySettings_allocate(2, steps1*steps2);
  
  VarySettings_addParameter(vs, parameter1, NULL);
  VarySettings_addParameter(vs, parameter2, reaction2);

  for ( i=0; i<steps1; i++ )
  {
    values[0] = start1 + i*(end1-start1)/steps1;
    for ( j=0; j<steps2; j++ )
    {
      values[1] = start2 + j*(end2-start2)/steps2;
      VarySettings_addDesignPoint(vs, values);    
    }
  }
  VarySettings_dump(vs);


  /* calling the SBML ODE Solver Batch function,
     and retrieving SBMLResults */
  resM = SBML_odeSolverBatch(d, set, vs);

  if ( resM == NULL )
  {
    printf("### Parameter variation not succesful!\n");
    SolverError_dumpAndClearErrors();
    CvodeSettings_free(set);  
    SBMLDocument_free(d);
    VarySettings_free(vs);
    return(0);
  }
  
  /* we don't need these anymore */
  CvodeSettings_free(set);  
  SBMLDocument_free(d);

  for ( i=0; i<resM->size; i++ )
  {
    results = SBMLResultsArray_getResults(resM, i);
    printf("### Parameters: "); 
    for ( j=0; j<2; j++ )
      printf("%s=%f, ",
	     VarySettings_getName(vs, j),
	     VarySettings_getValue(vs, i, j));
    printf("\n");
    /* printing results only for species*/
    SBMLResults_dumpSpecies(results);
    printf("\n");
  }
  
  /* SolverError_dumpAndClearErrors(); */
  SBMLResultsArray_free(resM);
  VarySettings_free(vs);
  return (EXIT_SUCCESS);  
}


/* End of file */
