/*
  Last changed Time-stamp: <2006-06-12 11:03:17 raim>
  $Id: ASTIndexNameNode.cpp,v 1.5 2006/06/12 10:25:55 raimc Exp $
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

#include "sbmlsolver/ASTIndexNameNode.h"

class ASTIndexNameNode :
public ASTNode
{
 public:
  ASTIndexNameNode();
  virtual ~ASTIndexNameNode(void);

  unsigned int getIndex() const { return index; }
  unsigned int isSetIndex() const { return indexSet; }
  void setIndex(unsigned int i) { index = i; indexSet = 1; }

  unsigned int isSetData() const { return dataSet; }
  void setData() { dataSet = 1; } 

 private:
  unsigned int index ;
  int indexSet ;
 
  /* used for differentiating whether the name refers
     to a variable for which "data" is available   */
  int dataSet;
};

ASTIndexNameNode::ASTIndexNameNode() : ASTNode(AST_NAME), index(0), dataSet(0)
{
}

ASTIndexNameNode::~ASTIndexNameNode(void)
{
}

// C interface
/** Creates a SOSlib specific variant of AST_NAME type ASTNode
    that can store an index (ASTIndexNameNode).

    This index corresponds to the index of the variable in a
    value array, allowing fast access to values for evaluation.
*/
ASTNode_t *ASTNode_createIndexName(void)
{
  return new ASTIndexNameNode();
}

/** Returns the index of an indexed AST_NAME node (ASTIndexNameNode)
 */
unsigned int ASTNode_getIndex(const ASTNode_t *node)
{
  return static_cast<const ASTIndexNameNode*>(node)->getIndex();
}

/** Sets the index of an indexed AST_NAME node (ASTIndexNameNode)
 */
void ASTNode_setIndex(ASTNode_t *node, unsigned int index)
{
  static_cast<ASTIndexNameNode*>(node)->setIndex(index);
}

/*! \addtogroup simplifyAST
  @{
*/

/** Returns true (1) if the ASTNode is an ASTIndexNameNode
 */
SBML_ODESOLVER_API int ASTNode_isIndexName(const ASTNode_t *node)
{
  return dynamic_cast<const ASTIndexNameNode*>(node) != 0;
}

/** Returns true (1) if the an indexed ASTNode (ASTIndexNameNode) has
    it's index set
*/
SBML_ODESOLVER_API unsigned int ASTNode_isSetIndex(const ASTNode_t *node)
{
  return ASTNode_isIndexName(node) && static_cast<const ASTIndexNameNode*>(node)->isSetIndex();
}




/** Returns true (1) if the an indexed ASTNode (ASTIndexNameNode) has
    its data set
*/
SBML_ODESOLVER_API unsigned int ASTNode_isSetData(const ASTNode_t *node)
{
  return ASTNode_isIndexName(node) && static_cast<const ASTIndexNameNode*>(node)->isSetData();
}

/** Sets the data of an indexed AST_NAME node (ASTIndexNameNode)
 */
void ASTNode_setData(ASTNode_t *node)
{
  static_cast<ASTIndexNameNode*>(node)->setData();
}


/** @} */
