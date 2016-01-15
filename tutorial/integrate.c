/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Last changed Time-stamp: <2007-09-28 14:01:29 raim>
  $Id: integrate.c,v 1.1 2007/09/28 13:14:16 raimc Exp $
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>
#include <sbmlsolver/solverError.h>


int main (void)
{
  SBMLReader_t *sr = SBMLReader_create();
  SBMLDocument_t *d = SBMLReader_readSBML(sr, "MAPK-l2.xml");
  Model_t *sbml = SBMLDocument_getModel(d);
  SBMLReader_free(sr);

  int i;
  Species_t *species = Model_getSpeciesById(sbml, "MAPK_PP");
  
  cvodeSettings_t *options = CvodeSettings_createWithTime(10000, 1000);
  /* WARNING: sbml model must be SBML Level 2 */
  SBMLResults_t *results = Model_odeSolver(sbml, options);
  if ( results != NULL )
  {
    timeCourse_t *timecourse = Species_getTimeCourse(species, results);
    for ( i=0; i<TimeCourse_getNumValues(timecourse); i++ )
      printf("%g\n", TimeCourse_getValue(timecourse, i));
  }
  SolverError_dumpAndClearErrors();
  SBMLResults_free(results);
  CvodeSettings_free(options);

  SBMLDocument_free(d);

  return (EXIT_SUCCESS);  
}



/* End of file */
