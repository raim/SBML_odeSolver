#include <stdio.h>
#include <assert.h>

#include "sbml/Math/FormulaParser.h"
#include "sbml/Math/FormulaFormatter.h"
#include "sbmlsolver/ASTIndexNameNode.h"

void main(void)
{
    int i;
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

    printf("top : %s\n", SBML_formulaToString(top));

    ASTNode_free(top);
}