/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Last changed Time-stamp: <2007-09-28 14:51:37 raim>
  $Id: setvariablevalue.c,v 1.1 2007/09/28 13:14:16 raimc Exp $
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
 *     Rainer Machne, Camille Stephan-Otto Attolini
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
  int i;
  cvodeSettings_t *options = CvodeSettings_createWithTime(0.1, 1000);  
  odeModel_t *odemodel = ODEModel_createFromFile("Goodwin_SomoS.xml");

  variableIndex_t *vi1 = ODEModel_getVariableIndex(odemodel, "mama");
  variableIndex_t *vi2 = ODEModel_getVariableIndex(odemodel, "fgf8");
  variableIndex_t *vi3 = ODEModel_getVariableIndex(odemodel, "FGF8");

  CvodeSettings_setIndefinitely(options, 1); /* integrate forever! */
  CvodeSettings_setTStop(options, 1);        /* RECOMMENDED! */
  CvodeSettings_setErrors(options, 1e-5, 1e-3, 1e4);
  
  integratorInstance_t *ii_1 = IntegratorInstance_create(odemodel, options);
  integratorInstance_t *ii_2 = IntegratorInstance_create(odemodel, options);

  /* change a parameter, the `mama cell flag`
     in one of the integrators */
  IntegratorInstance_setVariableValue(ii_1, vi1, 0);
  
  for( i=0; i<3000; i++ )
  {
    /* now use the value from one integrator, to set
       the value in the other integrator */
    IntegratorInstance_setVariableValue(ii_2, vi3,
	IntegratorInstance_getVariableValue(ii_1, vi2));

    /* integrate both integrators */
    IntegratorInstance_integrateOneStep(ii_1);
    IntegratorInstance_integrateOneStep(ii_2);
    /* write out all data, first column is time */
    IntegratorInstance_dumpData(ii_2);    
  }
  
  SolverError_dump();
  VariableIndex_free(vi1);
  VariableIndex_free(vi2);
  VariableIndex_free(vi3);
  IntegratorInstance_free(ii_1);
  IntegratorInstance_free(ii_2);  
  ODEModel_free(odemodel);
  CvodeSettings_free(options);
  return (EXIT_SUCCESS);  
}


/* End of file */
