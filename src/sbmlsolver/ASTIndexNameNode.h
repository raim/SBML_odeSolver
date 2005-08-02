#ifndef _ASTINDEXNAMENODE_H_
#define _ASTINDEXNAMENODE_H_

#include "sbml/math/ASTNode.h"

#include "sbmlsolver/exportdefs.h"

#ifdef __cplusplus

class ASTIndexNameNode :
    public ASTNode
{
public:
    ASTIndexNameNode();
    virtual ~ASTIndexNameNode(void);

    unsigned int getIndex() const { return index; }
    unsigned int isSetIndex() const { return indexSet; }
    void setIndex(unsigned int i) { index = i; indexSet = 1; }

private:
    unsigned int index ;
    int indexSet ;
};

#endif /* __cplusplus */

BEGIN_C_DECLS

SBML_ODESOLVER_API ASTNode_t *ASTNode_createIndexName(void);
SBML_ODESOLVER_API int ASTNode_isIndexName(ASTNode_t *);

/* assumes node is index node */ 
SBML_ODESOLVER_API unsigned int ASTNode_getIndex(ASTNode_t *); 

/* returns 0 if node isn't index or if index is not set yet */
SBML_ODESOLVER_API unsigned int ASTNode_isSetIndex(ASTNode_t *);

/* assumes node is index node */
SBML_ODESOLVER_API void ASTNode_setIndex(ASTNode_t *, unsigned int); 

END_C_DECLS

#endif /* _ASTINDEXNAMENODE_H_ */
