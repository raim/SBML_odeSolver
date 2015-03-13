/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#ifndef SBMLSOLVER_ASTINDEXNAMENODE_H_
#define SBMLSOLVER_ASTINDEXNAMENODE_H_

#include <sbml/math/ASTNode.h>

#include <sbmlsolver/exportdefs.h>

BEGIN_C_DECLS

/* creates a new AST node with an index field */
SBML_ODESOLVER_API ASTNode_t *ASTNode_createIndexName(void);

/* returns 1 if the node is indexed */
SBML_ODESOLVER_API int ASTNode_isIndexName(const ASTNode_t *);

/* assumes node is index node */ 
SBML_ODESOLVER_API unsigned int ASTNode_getIndex(const ASTNode_t *);

/* returns 0 if node isn't index or if index is not set yet */
SBML_ODESOLVER_API unsigned int ASTNode_isSetIndex(const ASTNode_t *);

/* assumes node is index node */
SBML_ODESOLVER_API void ASTNode_setIndex(ASTNode_t *, unsigned int); 

/* returns 0 if node isn't index or if data is not set yet */
SBML_ODESOLVER_API unsigned int ASTNode_isSetData(const ASTNode_t *);

/* assumes node is index node, and then sets data  */
SBML_ODESOLVER_API void ASTNode_setData(ASTNode_t *); 




END_C_DECLS

#endif
