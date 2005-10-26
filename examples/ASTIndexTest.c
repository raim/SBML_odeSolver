/*
  Last changed Time-stamp: <2005-10-26 17:52:40 raim>
  $Id: ASTIndexTest.c,v 1.4 2005/10/26 15:53:16 raimc Exp $
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
 *     Andrew Finney
 *
 * Contributor(s):
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sbml/math/FormulaParser.h"
#include "sbml/math/FormulaFormatter.h"
#include "sbmlsolver/ASTIndexNameNode.h"

int main(void)
{
    int i;
    char *formula;
    ASTNode_t *top = SBML_parseFormula("A * B");
    ASTNode_t *temp = ASTNode_create();

    for (i = 0; i != ASTNode_getNumChildren(top); i++)
    {
        ASTNode_t *node = ASTNode_getChild(top, i);
        ASTNode_t *replacement = ASTNode_createIndexName();

        assert(!ASTNode_isIndexName(node));
        assert(!ASTNode_isSetIndex(node));

        ASTNode_setName(replacement, ASTNode_getName(node));
        ASTNode_setIndex(replacement, i);
        ASTNode_addChild(temp, replacement);
    }

    ASTNode_swapChildren(temp, top);
    ASTNode_free(temp);

    for (i = 0; i != ASTNode_getNumChildren(top); i++)
    {
        ASTNode_t *node = ASTNode_getChild(top, i);
        
        assert(ASTNode_isIndexName(node));
        assert(ASTNode_isSetIndex(node));
        printf("index of %s is %d\n", ASTNode_getName(node), ASTNode_getIndex(node));
    }

    formula = SBML_formulaToString(top);
    printf("top : %s\n", formula);
    free(formula);

    ASTNode_free(top);

    return(EXIT_SUCCESS);
}
