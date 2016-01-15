/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sbmlsolver/ASTIndexNameNode.h"

#include <stdio.h>

/*
 * The index is stored in the user data with an ASTNode.
 * The LSB of the user data is on, meaning "this is an indexed ASTNode".
 * Its 2nd bit is for the flag indicating "the index has been stored".
 * Its 3rd bit is for "data is available".
 * The rest of the field contains the actual index value (shifted by 3).
 */
#define GET_USER_DATA(node) ((unsigned int)ASTNode_getUserData(node));

#define IS_INDEX_NAME(ud) (((ud) & 1u) == 1u)
#define HAS_INDEX(ud)     (((ud) & 2u) == 2u)
#define HAS_DATA(ud)      (((ud) & 4u) == 4u)

#define GET_INDEX(ud) ((ud)>>3)

/** Creates an AST_NAME type ASTNode with the indexed flag
    as its user data.
*/
ASTNode_t *ASTNode_createIndexName(void)
{
  ASTNode_t *node;
  node = ASTNode_createWithType(AST_NAME);
  ASTNode_setUserData(node, (void *)1u);
  return node;
}

static void report_argument_error(void)
{
  fprintf(stderr, "error: given node is not an indexed AST_NAME\n");
}

/** Returns the index of an indexed AST_NAME node (ASTIndexNameNode)
 */
unsigned int ASTNode_getIndex(const ASTNode_t *node)
{
  unsigned int ud;
  if (ASTNode_getType(node) != AST_NAME) {
    report_argument_error();
    return 0;
  }
  ud = GET_USER_DATA(node);
  if (!IS_INDEX_NAME(ud)) {
    report_argument_error();
    return 0;
  }
  return GET_INDEX(ud);
}

/** Sets the index of an indexed AST_NAME node
 */
void ASTNode_setIndex(ASTNode_t *node, unsigned int index)
{
  unsigned int ud;
  if (ASTNode_getType(node) != AST_NAME) {
    report_argument_error();
    return;
  }
  ud = GET_USER_DATA(node);
  if (!IS_INDEX_NAME(ud)) {
    report_argument_error();
    return;
  }
  ASTNode_setUserData(node, (void *)((index<<3)|(ud&4u)|3u));
}

/*! \addtogroup simplifyAST
  @{
*/

/** Returns true (1) if the ASTNode is an indexed AST_NAME node
 */
SBML_ODESOLVER_API int ASTNode_isIndexName(const ASTNode_t *node)
{
  unsigned int ud;
  if (ASTNode_getType(node) != AST_NAME) return 0;
  ud = GET_USER_DATA(node);
  return IS_INDEX_NAME(ud);
}

/** Returns true (1) if the an indexed ASTNode has
    its index set
*/
SBML_ODESOLVER_API unsigned int ASTNode_isSetIndex(const ASTNode_t *node)
{
  unsigned int ud;
  if (ASTNode_getType(node) != AST_NAME) return 0;
  ud = GET_USER_DATA(node);
  return IS_INDEX_NAME(ud) && HAS_INDEX(ud);
}




/** Returns true (1) if the indexed ASTNode has
    its data set

    used for differentiating whether the name refers
    to a variable for which "data" is available
*/
SBML_ODESOLVER_API unsigned int ASTNode_isSetData(const ASTNode_t *node)
{
  unsigned int ud;
  if (ASTNode_getType(node) != AST_NAME) return 0;
  ud = GET_USER_DATA(node);
  return IS_INDEX_NAME(ud) && HAS_DATA(ud);
}

/** Sets the data of an indexed AST_NAME node
 */
void ASTNode_setData(ASTNode_t *node)
{
  unsigned int ud;
  if (ASTNode_getType(node) != AST_NAME) {
    report_argument_error();
    return;
  }
  ud = GET_USER_DATA(node);
  if (!IS_INDEX_NAME(ud)) {
    report_argument_error();
    return;
  }
  ASTNode_setUserData(node, (void *)(ud|4u));
}


/** @} */
