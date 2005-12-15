/*
  Last changed Time-stamp: <2005-12-15 17:11:40 raim>
  $Id: ASTIndexNameNode.cpp,v 1.2 2005/12/15 16:33:54 raimc Exp $
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
 *
 */
/*! \addtogroup processAST  
@{
*/

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

/** @} */
