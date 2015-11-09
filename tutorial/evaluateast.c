/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Last changed Time-stamp: <2007-09-28 15:27:26 raim>
  $Id: evaluateast.c,v 1.1 2007/09/28 13:29:25 raimc Exp $
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
#include <sbmlsolver/processAST.h>
#include <sbmlsolver/solverError.h>

int main (void)
{
  cvodeSettings_t *options = CvodeSettings_createWithTime(10000, 1000);
  odeModel_t *odemodel = ODEModel_createFromFile("MAPK.xml");
  integratorInstance_t *ii = IntegratorInstance_create(odemodel, options);

  ASTNode_t *f = SBML_parseFormula("MAPK_PP");  
  ASTNode_t *f2 = SBML_parseFormula("MAPK + MAPK_P");  
  ASTNode_t *f3 = SBML_parseFormula("MAPK + MAPK_P + MAPK_PP");  
  cvodeData_t *data = IntegratorInstance_getData(ii);
  
  while( ! IntegratorInstance_timeCourseCompleted(ii) )
    if ( IntegratorInstance_integrateOneStep(ii ) )
    {
      printf("  active MAPK concentration at time %g:\t%7.3f\n",
	     IntegratorInstance_getTime(ii), evaluateAST(f, data));
      printf("inactive MAPK concentration at time %g:\t%7.3f\n",
	     IntegratorInstance_getTime(ii), evaluateAST(f2, data));
      printf("                                    total:\t%7.3f\n\n",
	     evaluateAST(f3, data));
    }
    else
      break;
  SolverError_dump();
  IntegratorInstance_free(ii);
  ODEModel_free(odemodel);
  CvodeSettings_free(options);
  ASTNode_free(f);
  ASTNode_free(f2);
  ASTNode_free(f3);
  
  return (EXIT_SUCCESS);  
}


/* End of file */
