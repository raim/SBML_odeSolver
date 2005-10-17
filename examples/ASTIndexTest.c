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
