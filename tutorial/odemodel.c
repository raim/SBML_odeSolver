/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Last changed Time-stamp: <2007-09-29 20:54:18 raim>
  $Id: odemodel.c,v 1.3 2007/09/29 18:57:31 raimc Exp $
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

int main (void)
{
  odeModel_t *odemodel = ODEModel_createFromFile("MAPK.xml");
  variableIndex_t *vi = ODEModel_getVariableIndex(odemodel, "MAPK_PP");
  
  const ASTNode_t *ode = ODEModel_getOde(odemodel, vi);
  char *equation = SBML_formulaToString(ode); 
  printf("d(%s)/dt = %s\n", ODEModel_getVariableName(odemodel, vi), equation);

  ODEModel_free(odemodel);
  VariableIndex_free(vi);
  free(equation);
  
  return (EXIT_SUCCESS);  
}



/* End of file */
