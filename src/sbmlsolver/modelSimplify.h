/*
  Last changed Time-stamp: <2005-10-26 17:26:30 raim>
  $Id: modelSimplify.h,v 1.5 2005/10/26 15:32:13 raimc Exp $
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

#ifndef _MODEL_H_
#define _MODEL_H_

#include "sbmlsolver/exportdefs.h"

SBML_ODESOLVER_API void AST_replaceNameByFormula(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceNameByName(ASTNode_t *math, const char *name, const char *newname);
SBML_ODESOLVER_API void AST_replaceNameByValue(ASTNode_t *math, const char *name, double x);
SBML_ODESOLVER_API void AST_replaceNameByParameters(ASTNode_t *math, ListOf_t* lp);
SBML_ODESOLVER_API void AST_replaceFunctionDefinition(ASTNode_t *math, const char *name, const ASTNode_t *formula);
SBML_ODESOLVER_API void AST_replaceConstants(Model_t *m, ASTNode_t *math);

#endif

/* End of file */
