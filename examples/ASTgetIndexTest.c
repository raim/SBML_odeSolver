/*
  Last changed Time-stamp: <2008-09-22 12:25:37 raim>
  $Id: ASTgetIndexTest.c,v 1.1 2008/09/22 10:28:49 raimc Exp $
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

#include "sbml/math/FormulaParser.h"
#include "sbml/math/FormulaFormatter.h"
#include "sbmlsolver/ASTIndexNameNode.h"
#include "sbmlsolver/processAST.h"

void printIndex(ASTNode_t *f)
{
  int i;
  if (ASTNode_isSetIndex(f))
  {
    printf("index of %s is %d\n", ASTNode_getName(f), ASTNode_getIndex(f));
  }
  else
  {  
    for (i = 0; i != ASTNode_getNumChildren(f); i++)
    {
      printIndex(ASTNode_getChild(f,i));
    }
  }  
}

int main(void)
{
    int i;
    char *formula;
    ASTNode_t *old = SBML_parseFormula("(A * B )+ 1/(C+D)^2");
    ASTNode_t *new;
    char *names[4];
    names[0] = "A";
    names[1] = "B";
    names[2] = "C";
    names[3] = "D";

    new = indexAST(old, 4, names);
    printIndex(new);


    formula = SBML_formulaToString(old);
    printf("old : %s\n", formula);
    free(formula);
    formula = SBML_formulaToString(new);
    printf("new : %s\n", formula);fflush(stdout);
    
    free(formula);

    List_t *index = List_create();
    ASTNode_getIndices(new, index);

    for ( i=0; i<List_size(index); i++ )
    {
      int k = List_get(index,i);
      printf("index of %s is %d\n", names[k], k);
    }

    ASTNode_free(old);
    ASTNode_free(new);

    return(EXIT_SUCCESS);
}
