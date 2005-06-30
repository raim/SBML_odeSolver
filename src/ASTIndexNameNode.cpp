#include "sbmlsolver/ASTIndexNameNode.h"

ASTIndexNameNode::ASTIndexNameNode() : ASTNode(AST_NAME), index(0)
{
}

ASTIndexNameNode::~ASTIndexNameNode(void)
{
}

// C interface

ASTNode_t *ASTNode_createIndexName(void)
{
    return new ASTIndexNameNode();
}

unsigned int ASTNode_getIndex(ASTNode_t *node)
{
    return static_cast<ASTIndexNameNode*>(node)->getIndex();
}

void ASTNode_setIndex(ASTNode_t *node, unsigned int index)
{
    static_cast<ASTIndexNameNode*>(node)->setIndex(index);
}

SBML_ODESOLVER_API int ASTNode_isIndexName(ASTNode_t *node)
{
    return dynamic_cast<ASTIndexNameNode*>(node) != 0;
}

SBML_ODESOLVER_API unsigned int ASTNode_isSetIndex(ASTNode_t *node)
{
    return ASTNode_isIndexName(node) && static_cast<ASTIndexNameNode*>(node)->isSetIndex();
}
