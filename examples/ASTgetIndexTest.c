/*
  Last changed Time-stamp: <2008-09-24 14:40:15 raim>
  $Id: ASTgetIndexTest.c,v 1.7 2008/09/24 14:10:36 raimc Exp $
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
    int nvalues = 6;
    char *names[nvalues];
    names[0] = "A";
    names[1] = "x";
    names[2] = "B";
    names[3] = "y";
    names[4] = "C";
    names[5] = "D";

    /* index the original AST */
    new = indexAST(old, nvalues, names);
    /* print by looping through AST */
    printIndex(new);


    formula = SBML_formulaToString(old);
    printf("old : %s\n", formula);
    free(formula);
    formula = SBML_formulaToString(new);
    printf("new : %s\n", formula);
    
    free(formula);

    /* retrieve List of indices */
    List_t *index = List_create();
    ASTNode_getIndices(new, index);

    for ( i=0; i<List_size(index); i++ )
    {
      int *k;
      k = List_get(index,i);
      printf("IDX of %s is %d\n", names[*k], *k);
      free(k);
    }
    List_free(index);
 
    /* retrieve boolean array of indices */

    int *indexBool = ASTNode_getIndexArray(new, nvalues);

    for ( i=0; i<nvalues; i++ )
    {
      printf("symbol %s occurs in equation? %s\n",
	     names[i], indexBool[i] ? "yes" : "no"); 
    }
    
    ASTNode_free(old);
    ASTNode_free(new);    
    free(indexBool);

    odeModel_t *om = ODEModel_createFromFile("../ruleorder/rules.xml");
/*     ODEModel_topologicalRuleSort(om); */
    ODEModel_free(om);

    return(EXIT_SUCCESS);
}
