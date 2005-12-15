/*
  Last changed Time-stamp: <2005-12-15 20:50:29 raim>
  $Id: odeModel.h,v 1.17 2005/12/15 19:54:06 raimc Exp $ 
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
 *     Rainer Machne     
 */

#ifndef _ODEMODEL_H_
#define _ODEMODEL_H_

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/odemodeldatatype.h"
#include "sbmlsolver/exportdefs.h"

/** The internal ODE Model as constructed in odeModel.c from an SBML
    input file, that only contains rate rules (constructed from
    reaction network in odeConstruct.c)
*/
struct odeModel
{
  SBMLDocument_t *d; /**< not-NULL only if the odeModel was directly
			  created from file */
  Model_t *m;        /**< the input SBML reaction network */
  Model_t *simple;   /**< the derived SBML with rate rules */

  /** All names, i.e. ODE variables, assigned parameters, and constant
      parameters */
  char **names; 

  int neq;    /**< number of ODEs */
  int nalg;   /**< number of algebraic rules */ 
  int nass;   /**< number of assigned variables (nass) */
  int nconst; /**< number of constant parameters */ 

  /** Assigned variables: stores species, compartments and parameters,
      that are set by an assignment rule */
  ASTNode_t **assignment;

  /** Algebraic Rules (constraints) as used for DAE systems */
  ASTNode_t **algebraic;
  
  /** The Ordinary Differential Equation System (ODE)s: f(x,p,t) = dx/dt */
  ASTNode_t **ode; 

  /** The jacobian matrix df(x)/dx of the ODE system 
      neq x neq */
  ASTNode_t ***jacob;
  /** was the jacobian matrix constructed ? */
  int jacobian;

  /* forward sensitivity analysis structure
      neq x num_param */
  int nsens;               /**< number of parameters for sens. analysis */
  int *index_sens;         /**< indexes of parameters in char **names,
			        char *value*/
  ASTNode_t ***jacob_sens; /**< sensitivity matrix: df(x)/dp  */
  int sensitivity;         /**< was the sensitivity matrix constructed ? */

  /* adjoint */

};

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct variableIndex variableIndex_t;

  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromFile(char *);
  SBML_ODESOLVER_API odeModel_t *ODEModel_createFromSBML2(SBMLDocument_t *);
  SBML_ODESOLVER_API odeModel_t *ODEModel_create(Model_t *);
  SBML_ODESOLVER_API void ODEModel_free(odeModel_t *);
  
  SBML_ODESOLVER_API int ODEModel_hasVariable(odeModel_t *, const char *);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndexByNum(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getOdeVariableIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getAssignedVariableIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getConstantIndex(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getSensParamIndexByNum(odeModel_t *, int);
  SBML_ODESOLVER_API variableIndex_t *ODEModel_getVariableIndex(odeModel_t *, const char *);
  SBML_ODESOLVER_API const char *ODEModel_getVariableName(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getOde(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getAssignment(odeModel_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getJacobianIJEntry(odeModel_t *, int i, int j);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getJacobianEntry(odeModel_t *, variableIndex_t *, variableIndex_t *);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getSensIJEntry(odeModel_t *, int i, int j);
  SBML_ODESOLVER_API const ASTNode_t *ODEModel_getSensEntry(odeModel_t *, variableIndex_t *, variableIndex_t *);
  SBML_ODESOLVER_API void VariableIndex_free(variableIndex_t *);
  SBML_ODESOLVER_API int ODEModel_getNeq(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNalg(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNsens(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumAssignments(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumConstants(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_getNumValues(odeModel_t *);
  SBML_ODESOLVER_API const Model_t *ODEModel_getModel(odeModel_t *);
  SBML_ODESOLVER_API void ODEModel_dumpNames(odeModel_t *);  
  SBML_ODESOLVER_API int ODEModel_constructJacobian(odeModel_t *);
  SBML_ODESOLVER_API int ODEModel_constructSensitivity(odeModel_t *);
  SBML_ODESOLVER_API void ODEModel_freeSensitivity(odeModel_t *);
  SBML_ODESOLVER_API ASTNode_t *ODEModel_constructDeterminant(odeModel_t *);
 
#ifdef __cplusplus
}
#endif


#endif
