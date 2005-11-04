/*
  Last changed Time-stamp: <2005-11-04 10:31:43 raim>
  $Id: odeModel.h,v 1.16 2005/11/04 10:39:15 raimc Exp $ 
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

struct odeModel
{
  SBMLDocument_t *d; /* only if directly created from file */
  Model_t *m;
  Model_t *simple;

  /* All names, i.e. ODE variables, assigned parameters, and constant
     parameters */
  char **names; 

  /* number of ODEs (neq), assigned parameters (nass), and constant
     parameters (nconst) */
  int neq;
  int nalg;
  int nass;
  int nconst;  

  /* Assigned variables: stores species, compartments and parameters,
     that are set by an assignment rule */
  ASTNode_t **assignment;

  /* algebraic rules */
  ASTNode_t **algebraic;
  
  /* The main data: number, names, and equation ASTs
     of the ODEs. The value array is used to write and read the
     current value of the ODE variables during simulation. */
  ASTNode_t **ode; 

  /* The jacobian matrix (d[X]/dt)/d[Y] of the ODE system */
  /* neq x neq */
  ASTNode_t ***jacob;
  /* was the model the jacobian constructed ? */
  int jacobian;

  /* forward sensitivity analysis */
  /* neq x num_param */
  int nsens;    /* number of parameters for sens. analysis */
  int *index_sens; /* indexes of parameters in char **names, char *value*/
  ASTNode_t ***jacob_sens; /*  (d[Y]/dt)/dP  */
  int sensitivity;  /* a flag for success */

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
