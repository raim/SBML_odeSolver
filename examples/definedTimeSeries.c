/*
  Last changed Time-stamp: <2006-03-07 16:57:44 raim>
  $Id: definedTimeSeries.c,v 1.10 2006/03/07 15:58:35 raimc Exp $
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
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>

int
main (int argc, char *argv[]){
  int i;
  char *model;

  /* libSBML types */
  SBMLDocument_t *d;
  SBMLReader_t *sr;

  /* SOSlib types */
  SBMLResults_t *results;
  cvodeSettings_t *set;

  double printstep;
  double endtime;
  
  /* parsing command-line arguments */
  if (argc < 2 ) {
    fprintf(stderr,
	    "usage %s sbml-model-file\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  model = argv[1];   

  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  d = SBMLReader_readSBML(sr, model);
  SBMLReader_free(sr);
  
  /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  /* setting endtime to 25 and printstep number to 6 */  
  printstep = 6;
  endtime = 25;
  CvodeSettings_setTime(set, endtime, printstep);
 
  /* writing predefined output times:
     IMPORTANT: can not exceed the set printstep number !!
                and first time must equal 0 !! */
  CvodeSettings_setTimeStep(set, 1, 0.5);
  for ( i=2; i<=CvodeSettings_getPrintsteps(set); i++ ) 
    CvodeSettings_setTimeStep(set, i, (i-1)*(i-1));

  /* printing integration settings */
  /* CvodeSettings_dump(set); */
  
  /* calling the SBML ODE Solver, and retrieving SBMLResults */  
  results = SBML_odeSolver(d, set);
  
  CvodeSettings_free(set);
  SBMLDocument_free(d);
  
  if ( results == NULL ) {
    printf("Integration not sucessful!\n");
    return (EXIT_FAILURE);  
  }

  /* printing results only for species*/
  SBMLResults_dumpSpecies(results);

  /* now we can also free the result structure */
  SBMLResults_free(results);

  return (EXIT_SUCCESS);  
}

/* End of file */
