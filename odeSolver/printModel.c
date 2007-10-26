/*
  Last changed Time-stamp: <2007-10-24 11:30:21 raim>
  $Id: printModel.c,v 1.22 2007/10/26 17:52:29 raimc Exp $
*/
/* 
 *
 * This application is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This application  is distributed in the hope that it will be useful, but
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
 *     Rainer Machne
 *
 * Contributor(s):
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 

/* Header Files from the SBML Library libsbml */
#include <sbml/SBMLTypes.h>
#include <sbml/util/util.h> /* only for util_trim */

/* System specific definitions,
   created by configure script */
#ifndef WIN32
#include "../src/sbmlsolver/config.h"
#endif

/* Own Header Files */
#include "../src/sbmlsolver/util.h"
#include "../src/sbmlsolver/cvodeData.h"
#include "../src/sbmlsolver/modelSimplify.h"
#include "../src/sbmlsolver/processAST.h"

#include "options.h"
#include "printModel.h"

/*
  XMGrace Library dependent functions,
  printing results of simulation to XMGrace
*/
#if USE_GRACE
/* Header Files for XMGrace */
#include <grace_np.h>
/* functions */
static void grace_error(const char *msg);
static int printXMGConcentrationTimeCourse(cvodeData_t *data);
static int printXMGOdeTimeCourse(cvodeData_t *data);
static int printXMGReactionTimeCourse(cvodeData_t *data);
static int printXMGJacobianTimeCourse(cvodeData_t *data);
static int printXMGLegend(cvodeData_t *data, int nvalues);
static int openXMGrace(cvodeData_t *data);
static int closeXMGrace(cvodeData_t *data, char *name);
#endif

void printModel(Model_t *m, FILE *f)
{
  fprintf(f, "\n");
  fprintf(f, "Model Statistics:\n");
  fprintf(f, " Model id:     %s\n",
	 Model_isSetId(m) ? Model_getId(m) : "(not set)");
  fprintf(f, " Model name:   %s\n",
	 Model_isSetName(m) ? Model_getName(m) : "(not set)"); 
  fprintf(f, "\n");
  fprintf(f, " Compartments: %d\n",  Model_getNumCompartments(m)); 
  fprintf(f, " Species:      %d\n",  Model_getNumSpecies(m));
  fprintf(f, " Reactions:    %d\n",  Model_getNumReactions(m));
  fprintf(f, " Rules:        %d\n",  Model_getNumRules(m));
  fprintf(f, " Events:       %d\n",  Model_getNumEvents(m));
  fprintf(f, " Functions:    %d\n",  Model_getNumFunctionDefinitions(m) ); 
  fprintf(f, "\n");
}


void printSpecies(Model_t *m, FILE *f)
{
  int i, j;
  Species_t *s;  
  Compartment_t *c;

  fprintf(f, "\n");
  fprintf(f, "# Initial Conditions for Species and Compartments:\n");
  for ( i=0; i<Model_getNumCompartments(m); i++ )
  {
    if ( i== 0 ) fprintf(f, "# Compartments:\n");
    c = Model_getCompartment(m,i);
    if(Compartment_isSetId(c))
      fprintf(f, "%s ", Compartment_getId(c));
    if(Compartment_isSetName(c))
      fprintf(f, "(%s) ", Compartment_getName(c));
    if ( Compartment_isSetVolume(c) )
     fprintf(f, "= %g; ", Compartment_getSize(c));
    fprintf(f, "%s", Compartment_getConstant(c) ? "" : "variable; ");
    if(Compartment_isSetOutside(c))
      fprintf(f, "outside %s; ", Compartment_getOutside(c));
   /*  fprintf(f, "\n"); */
    fprintf(f, "dimensions %d; ", Compartment_getSpatialDimensions(c));

    if(Compartment_isSetUnits(c))
      fprintf(f, "[%s]; ", Compartment_getUnits(c));
    fprintf(f, "\n");
    
    fprintf(f, "# Species concentrations in `compartment' %s\n",
	   Compartment_getId(c));
    for(j=0;j<Model_getNumSpecies(m);j++){
      s = Model_getSpecies(m,j);      
      if(strcmp(Species_getCompartment(s), Compartment_getId(c))==0){  

	fprintf(f, "%s ", Species_getId(s));
	if(Species_isSetName(s))
	  fprintf(f, "(%s) ", Species_getName(s));
	
	if ( Species_isSetInitialAmount(s) )
	{
	  if ( Compartment_getSpatialDimensions(c) != 0 &&
	       !Species_getHasOnlySubstanceUnits(s) )
	  fprintf(f, "= %g/%g; ",
		 Species_getInitialAmount(s),
		  Compartment_getSize(c));
	  else 
	    fprintf(f, "= %g; ", Species_getInitialAmount(s));
	}
	else if ( Species_isSetInitialConcentration(s) )
	  fprintf(f, "= %g; ", Species_getInitialConcentration(s));
	else
	  fprintf(f, "# no initial value;");	
	fprintf(f, "%s", Species_getBoundaryCondition(s) ? "boundary;" : "");
	fprintf(f, "%s", Species_getConstant(s) ? "constant;" : "");
	if(Species_isSetCharge(s))
	  fprintf(f, "charge = %d; ", Species_getCharge(s));

	fprintf(f, "\n");
      }     
    }
    fprintf(f, "\n");  
  }  
}

void printReactions(Model_t *m, FILE *f)
{
  
  int i,j,k;
  Reaction_t *r;
  SpeciesReference_t *sref;
  KineticLaw_t *kl;
  Rule_t *rl;
  Event_t *e;
  EventAssignment_t *ea;
  Parameter_t *p;
  FunctionDefinition_t *fd;
  SBMLTypeCode_t type;
  const ASTNode_t *math;

  math = NULL;
  
  fprintf(f, "\n");
  for(i=0;i<Model_getNumParameters(m);i++){
    if(i==0)
      fprintf(f, "# Global parameters:\n");
    p = Model_getParameter(m,i);
    if(Parameter_isSetId(p))
      fprintf(f, "%s ",  Parameter_getId(p));
    if(Parameter_isSetName(p))
      fprintf(f, "(%s) ", Parameter_getName(p));
    if(Parameter_isSetValue(p))
      fprintf(f, "= %g; ", Parameter_getValue(p));
    if(Parameter_isSetUnits(p))
      fprintf(f, "[%s]; ", Parameter_getUnits(p));
    if(!Parameter_getConstant(p))
      fprintf(f, "(variable);");
    fprintf(f, "\n");
    
    if ( i==Model_getNumParameters(m)-1 )
      fprintf(f, "\n");
  }

  fprintf(f, "# Reactions:\n");
  for ( i=0; i<Model_getNumReactions(m); i++ ) {    
    r = Model_getReaction(m,i);
  
    fprintf(f, "%s: %s",
	   Reaction_isSetName(r) ? Reaction_getName(r) : Reaction_getId(r),
	   Reaction_getFast(r) ? "(fast)" : "");
    for ( k=0; k<Reaction_getNumReactants(r); k++ ) {
      sref = Reaction_getReactant(r,k);

      if ( SpeciesReference_isSetStoichiometryMath(sref) )	
	fprintf(f, "%s ",
		SBML_formulaToString(StoichiometryMath_getMath(SpeciesReference_getStoichiometryMath(sref))));
      else 
	if ( SpeciesReference_getStoichiometry(sref) != 1. )
	  fprintf(f, "%g ",  SpeciesReference_getStoichiometry(sref));
	
      fprintf(f, "%s", SpeciesReference_getSpecies(sref));
      if(k+1<Reaction_getNumReactants(r))
	fprintf(f, "%s", " + ");
    }
    
    fprintf(f, "%s", Reaction_getReversible(r) ? " <-> " : " -> ");
    for ( k=0; k<Reaction_getNumProducts(r); k++ ) {
      sref = Reaction_getProduct(r,k);
      if ( SpeciesReference_isSetStoichiometryMath(sref) )
	fprintf(f, "%s ",
	       SBML_formulaToString(StoichiometryMath_getMath(SpeciesReference_getStoichiometryMath(sref))));
      else
	if ( SpeciesReference_getStoichiometry(sref) != 1. )
	  fprintf(f, "%g ", SpeciesReference_getStoichiometry(sref));
      
      fprintf(f, "%s", SpeciesReference_getSpecies(sref));
      if(k+1<Reaction_getNumProducts(r))
	fprintf(f, "%s", " + ");
    }
    fprintf(f, ";  ");
    if(Reaction_isSetKineticLaw(r)){
      kl = Reaction_getKineticLaw(r);
      math = KineticLaw_getMath(kl);
      fprintf(f, "%s;", SBML_formulaToString(math));
      for(k=0;k<KineticLaw_getNumParameters(kl);k++){
	
	p = KineticLaw_getParameter(kl,k);
	fprintf(f, " %s",  Parameter_getId(p));
	if(Parameter_isSetName(p))
	  fprintf(f, " (%s)", Parameter_getName(p));
	if(Parameter_isSetValue(p))
	  fprintf(f, " = %g", Parameter_getValue(p));
	if(Parameter_isSetUnits(p))
	  fprintf(f, " [%s]", Parameter_getUnits(p));
	if ( !Parameter_getConstant(p) )
	  fprintf(f, " (variable)");
	fprintf(f, ";");
      }
     /*  fprintf(f, "\n"); */
    }else
      fprintf(f, "#   no rate law is set for this reaction.");
    fprintf(f, "\n");
  }
    
  for ( i=0; i<Model_getNumRules(m); i++ )
  {
    rl = Model_getRule(m,i);
    if ( i == 0 )
      fprintf(f, "# Rules:\n");
    type = SBase_getTypeCode((SBase_t *)rl);
     
    if ( type == SBML_RATE_RULE ) 
      fprintf(f, " rateRule:       d%s/dt = ", Rule_getVariable(rl));
    if ( type == SBML_ALGEBRAIC_RULE ) 
      fprintf(f, " algebraicRule:       0 = ");
    if ( type == SBML_ASSIGNMENT_RULE ) 
      fprintf(f, " assignmentRule: %s = ", Rule_getVariable(rl));

    if ( Rule_isSetMath(rl) )
      fprintf(f, "%s\n", SBML_formulaToString(Rule_getMath(rl)));
	     
  }
  fprintf(f, "\n");

  for ( i=0; i<Model_getNumEvents(m); i++ )
  {
    if ( i==0 )
      fprintf(f, "# Events:\n");
    
    e = Model_getEvent(m,i);
    if ( Event_isSetId(e) )
      fprintf(f, "%s: ", Event_getId(e));
    if ( Event_isSetName(e) )
      fprintf(f, "(%s) ", Event_getName(e));   
    if ( Event_isSetTrigger(e) )
    {
      math = Trigger_getMath(Event_getTrigger(e));
      fprintf(f, "trigger: %s\n", SBML_formulaToString(math));
    }
    if ( Event_isSetDelay(e) )
      fprintf(f, "delay: %s;\n",
	      SBML_formulaToString(Delay_getMath(Event_getDelay(e))));
    if ( Event_isSetTimeUnits(e) )
      fprintf(f, "time Units: %s;\n", Event_getTimeUnits(e));
    for ( k=0; k<Event_getNumEventAssignments(e); k++ )
    {      
      ea = Event_getEventAssignment(e,k);
      if(EventAssignment_isSetVariable(ea))
	fprintf(f, "  event:  %s = %s;\n",
	       EventAssignment_getVariable(ea),
	       EventAssignment_isSetMath(ea) ?
	       SBML_formulaToString(EventAssignment_getMath(ea)) :
	       "# no math set;\n");
    }

    if ( i == Model_getNumEvents(m)-1 )
       fprintf(f, "\n"); 
  }  


  for ( i=0; i<Model_getNumFunctionDefinitions(m); i++ ) {

    if ( i==0 ) fprintf(f, "# Functions:\n");

    fd = Model_getFunctionDefinition(m,i);
    if ( FunctionDefinition_isSetName(fd) )
      fprintf(f, "%s: ", FunctionDefinition_getName(fd));
    if(FunctionDefinition_isSetId(fd) && FunctionDefinition_isSetMath(fd)){
      fprintf(f, "%s( ", FunctionDefinition_getId(fd));
      math = FunctionDefinition_getMath(fd);
	for(j=0;j<ASTNode_getNumChildren(math)-1;j++){
	  fprintf(f, "%s", SBML_formulaToString(ASTNode_getChild(math, j)));
	  if(j<ASTNode_getNumChildren(math)-2)
	    fprintf(f, ", ");
	  if(j==ASTNode_getNumChildren(math)-2)
	    fprintf(f, ") = ");
	}
      fprintf(f, "%s;", SBML_formulaToString(ASTNode_getRightChild(math)));
    }
    fprintf(f, "\n");
  } 
  
}


void printODEsToSBML(Model_t *ode, FILE *f)
{      
  SBMLDocument_t *d;
  char *model;
  d = SBMLDocument_create();
  SBMLDocument_setModel(d, ode);
  model = writeSBMLToString(d);
  fprintf(f, "%s", model);
  free(model);
}


void printODEs(odeModel_t *model, FILE *f)
{  
  int i, nvalues;
  char *formel;

  nvalues = model->neq+model->nass + model->nconst;
  fprintf(f, "\n");
  fprintf(f, "# Derived system of Ordinary Differential Equations (ODEs):\n");
  
  fprintf(f, "# Parameters:\n");
  for ( i=model->neq+model->nass; i<nvalues; i++ ) 
    fprintf(f, "%s, ", model->names[i]);
  printf("\n");
  fprintf(f, "# Assigned Parameters:\n");
  for ( i=0; i<model->nass; i++ ) {
    formel = SBML_formulaToString(model->assignment[i]);
    fprintf(f, "%d: %s =  %s;\n", i+1, model->names[model->neq+i], formel);
    free(formel);
  }  
  for ( i=0; i<model->neq; i++ ) {
    if ( i == 0 ) {
      fprintf(f, "# ODEs:\n");
    }
    formel = SBML_formulaToString(model->ode[i]);
    fprintf(f, "%d: d%s/dt =  %s;\n", i+1, model->names[i], formel);
    free(formel);
  }
  fprintf(f, "\n");

  /* change in behaviour AMF 27th June 05 
  if ( data->errors>0 ) {
    fprintf(f, "# %d ODEs could not be constructed."
	   " Try to add or correct kinetic laws!\n",
	   data->errors); 
  }*/
  return;
}

void printJacobian(odeModel_t *om, FILE *f)
{    
  int i, j;
  if ( om == NULL ) {
    fprintf(stderr, "No odeModel available.\n");
    return;
  }

  if ( om->jacob == NULL ) {
    fprintf(stderr, "Jacobian Matrix has not been constructed.\n");
    return;
  }

  fprintf(f, "\n");
  fprintf(f, "# Jacobian Matrix:\n");
  for ( i=0; i<om->neq; i++ ) {
    fprintf(f, "# %s: \n", om->names[i]);
    for ( j=0; j<om->neq; j++ ) {
      fprintf(f, "  (d[%s]/dt)/d[%s] = %s;\n", 
	     om->names[i], om->names[j], 
	     SBML_formulaToString(om->jacob[i][j]));
    }
  }
  fprintf(f, "\n");
  fprintf(stderr, "Use option -j to avoid printing the"
	  " jacobian matrix expressions.\n");
  return;
}

/* The following functions print results of integration */

void printDeterminantTimeCourse(cvodeData_t *data, ASTNode_t *det, FILE *f)
{
  int i,j;
  cvodeResults_t *results;

  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }
  
  if ( Opt.PrintMessage )
    fprintf(stderr, "\nPrinting time course of det(j).\n\n");
  
  results = data->results;
  fprintf(f, "#t det(j)\n");
  fprintf(f, "##DETERMINANT OF THE JACOBIAN MATRIX\n");
  for ( i = 0; i<=results->nout; i++ ) {
    fprintf(f, "%g ", results->time[i]);
    data->currenttime = results->time[i];
    for ( j=0; j<data->model->neq; j++ ) {
      data->value[j] = results->value[j][i];
    }
    fprintf(f, "%g\n", evaluateAST(det, data));
  }
  fprintf(f, "##DETERMINANT OF THE JACOBIAN MATRIX\n");
  fprintf(f, "#t det(j)\n");
  fflush(f);
}


void printJacobianTimeCourse(cvodeData_t *data, FILE *f)
{
  int i, j, k;
  cvodeResults_t *results;


  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }


#if USE_GRACE
  if ( Opt.Xmgrace == 1 ) {
    printXMGJacobianTimeCourse(data);
    return;
  }
#endif  
  
  results = data->results;
  if ( Opt.PrintMessage )
    fprintf(stderr, "Printing time course of the jacobian matrix.\n\n");
  
  fprintf(f, "#t ");
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      fprintf(f, "%s ", data->model->names[j]);
    }
  }
  fprintf(f, "\n");
  fprintf(f, "##JACOBIAN MATRIX VALUES\n");
  for ( k = 0; k<=results->nout; k++ ) {
    fprintf(f, "%g ", results->time[k]);
    data->currenttime = results->time[k];
    for ( i=0; i<data->model->neq; i++ ) {
      data->value[i] = results->value[i][k];
    }
    for ( i=0; i<data->model->neq; i++ ) {
      for ( j=0; j<data->model->neq; j++ ) {	    	   
	fprintf(f, "%g ", evaluateAST(data->model->jacob[i][j], data));
      }
    }
    fprintf(f, "\n");
  }
  fprintf(f, "##JACOBIAN MATRIX VALUES\n");
  fprintf(f, "#t ");				      
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      fprintf(f, "%s ", data->model->names[j]);
    }
  }
  fprintf(f, "\n");
  fflush(f);

#if !USE_GRACE

  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr,
	    "odeSolver has been compiled without XMGRACE functionality.\n");
    fprintf(stderr,
	    "The requested data have been printed to stdout instead.\n");
  }

#endif
  
}

void printOdeTimeCourse(cvodeData_t *data, FILE *f)
{  
  int i,j;
  cvodeResults_t *results;

  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }
  
#if USE_GRACE
  if ( Opt.Xmgrace == 1 ) {
    printXMGOdeTimeCourse(data);
    return;
  }
#endif
  
  results = data->results;
  if ( Opt.PrintMessage )
    fprintf(stderr, "\nPrinting time course of the ODEs.\n\n");
    
  fprintf(f, "#t ");
  for (i=0; i<data->model->neq; i++ ) {
    fprintf(f, "%s ", data->model->names[i]);
  }
  fprintf(f, "\n");
  fprintf(f, "##ODE VALUES\n");
  for ( i=0; i<=results->nout; ++i ) { 
    fprintf(f, "%g ", results->time[i]);
    data->currenttime = results->time[i];
    for ( j=0; j<data->model->neq; j++ ) {
      data->value[j] = results->value[j][i];
      fprintf(f, "%g ", evaluateAST(data->model->ode[j],data));
    }
    fprintf(f, "\n");
  }
  fprintf(f, "##ODE VALUES\n");
  fprintf(f, "#t ");
  for ( i=0; i<data->model->neq; i++ ) {
    fprintf(f, "%s ", data->model->names[i]);
  }
  fprintf(f, "\n");
  fflush(f);

  
#if !USE_GRACE

  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr,
	    "odeSolver has been compiled without XMGRACE functionality.\n");
    fprintf(stderr,
	    "The requested data have been printed to stdout instead.\n");
  }

#endif
  
}

void printReactionTimeCourse(cvodeData_t *data, Model_t *m, FILE *f)
{
  int i, j;
  cvodeResults_t *results;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;

  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }

#if USE_GRACE
  if ( Opt.Xmgrace == 1 ) {
    printXMGReactionTimeCourse(data);
    return;
  }
#endif  
  
  results = data->results;
  if ( Opt.PrintMessage )
    fprintf(stderr,
	    "\nPrinting time course of the reactions (kinetic laws).\n\n");

  if(!(kls =
       (ASTNode_t **)calloc(Model_getNumReactions(m),
			    sizeof(ASTNode_t *)))) {
    fprintf(stderr, "failed!\n");
  }

  fprintf(f, "#t ");
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    kl = Reaction_getKineticLaw(r);
    kls[i] = copyAST(KineticLaw_getMath(kl));
    AST_replaceNameByParameters(kls[i], KineticLaw_getListOfParameters(kl));
    AST_replaceConstants(m, kls[i]);
    fprintf(f, "%s ", Reaction_getId(r));
  }
  fprintf(f, "\n");
  fprintf(f, "##REACTION RATES\n"); 
  for ( i=0; i<=results->nout; ++i ) {
    fprintf(f, "%g ", results->time[i]);
    data->currenttime = results->time[i];
    for ( j=0; j<data->model->neq; j++ ) {
      data->value[j] = results->value[j][i];
    }
    for ( j=0; j<Model_getNumReactions(m); j++ ) {      
      
      fprintf(f, "%g ", evaluateAST(kls[j], data));
    }
    fprintf(f, "\n");
  }
  fprintf(f, "##REACTION RATES\n"); 
  fprintf(f, "#t ");
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    fprintf(f, "%s ", Reaction_getId(r));
    ASTNode_free(kls[i]);
  }
  free(kls);
  fprintf(f, "\n");
  fflush(f);
  
#if !USE_GRACE
  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr,
	    "odeSolver has been compiled without XMGRACE functionality.\n");
    fprintf(stderr,
	    "The requested data have been printed to stdout instead.\n");
  }
#endif
  
}

void printConcentrationTimeCourse(cvodeData_t *data, FILE *f)
{  
  int i,j, k;
  cvodeResults_t *results;
  odeModel_t *om;
  odeSense_t *os;

  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }

#if USE_GRACE
  if ( Opt.Xmgrace == 1 ) {
    printXMGConcentrationTimeCourse(data);
    return;
  }
#endif  
  
  results = data->results;
  om = data->model;
  os = data->os;

  if ( Opt.PrintMessage ) {
    fprintf(stderr,
	    "\nPrinting time course of all variable values");
    if ( Opt.Sensitivity  && results->sensitivity != NULL )
      fprintf(stderr, "\nand sensitivities of ODE variables.\n\n");
    else
      fprintf(stderr, ".\n\n");
  }
  
  /* print sensitivities of calculated */
  if ( Opt.Sensitivity  && results->sensitivity != NULL ) {
    fprintf(f, "#t ");
    for( j=0; j<om->neq; j++ )
      for ( k=0; k<os->nsens; k++ )
	fprintf(f, "d%s/d%s ", om->names[j], om->names[os->index_sens[k]]);

    fprintf(f, "\n");
    fprintf(f, "##SENSITIVITIES\n");
    
    for ( i=0; i<=results->nout; ++i ) {
      fprintf(f, "%g ", results->time[i]);      
      for ( j=0; j<om->neq; j++ ) 
        for ( k=0; k<os->nsens; k++ ) 
	  fprintf(f, "%g ", results->sensitivity[j][k][i]);
      fprintf(f, "\n");
    }
    
    fprintf(f, "\n");
    fprintf(f, "##SENSITIVITIES\n");
    fprintf(f, "#t ");
    
    for( j=0; j<om->neq; j++ ) 
      for ( k=0; k<os->nsens; k++ )
	fprintf(f, "d%s/d%s ", om->names[j], om->names[os->index_sens[k]]);
    
  }
  /* print concentrations */
  else {
    fprintf(f, "#t ");
    for( i=0; i<data->nvalues; i++)
        if (om->observablesArray[i])
            fprintf(f, "%s ", om->names[i]);
    
    fprintf(f, "\n");    
    fprintf(f, "##CONCENTRATIONS\n");
    for ( i=0; i<=results->nout; ++i ) {
      fprintf(f, "%g ", results->time[i]);
      
      for ( j=0; j<om->neq; j++ ) 
        if (om->observablesArray[j])
	        fprintf(f, "%g ", results->value[j][i]);

      for ( j=0; j<om->nass; j++ ) 
        if (om->observablesArray[om->neq+j])
	        fprintf(f, "%g ", results->value[om->neq+j][i]);
      
      for ( j=0; j<om->nconst; j++ )
        if (om->observablesArray[om->neq+om->nass+j])
	        fprintf(f, "%g ", results->value[om->neq+om->nass+j][i]);

      fprintf(f, "\n");
    }
    fprintf(f, "##CONCENTRATIONS\n");
    fprintf(f, "#t ");
    for( i=0; i<data->nvalues; i++ )
      if (om->observablesArray[i])
        fprintf(f, "%s ", om->names[i]);

    fprintf(f, "\n");
  }
  
  fprintf(f, "\n\n");
  
  fflush(f);

#if !USE_GRACE

  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr,
	    "odeSolver has been compiled without XMGRACE functionality.\n");
    fprintf(stderr,
	    "The requested data have been printed to stdout instead.\n");
  }

#endif
  
}


/*
  The following functions print results of the integration
  to XMGrace.
*/

/*
  The function printPhase(data) asks the user to enter 2 species of
  the integrated system and prints a phase diagram to XMGrace.
*/

void printPhase(cvodeData_t *data)
{

#if !USE_GRACE

  fprintf(stderr,
	  "odeSolver has been compiled without XMGRACE functionality.\n");
  fprintf(stderr,
	  "Phase diagrams can only be printed to XMGrace at the moment.\n");

#else
  
  int i,j;
  double maxY;
  double minY;
  double maxX;
  char *x;
  double xvalue;
  char *y;
  double yvalue;

  cvodeResults_t *results;

  maxY = 1.0;
  maxX = 1.0;
  minY = 0.0;
  
  if ( data==NULL || data->results==NULL ) {
    Warn(stderr,
	 "No data available to print! Please integrate model first!\n");
    return;
  }

  results = data->results;

  if ( openXMGrace(data) > 0 ) {
    fprintf(stderr,
	    "Error: Couldn't open XMGrace\n");
    return;
  }

  GracePrintf("world xmax %g", 1.25*maxX);
  GracePrintf("world ymax %g", 1.25*maxY);

  GracePrintf("xaxis tick major %g", (1.25*maxX)/12.5);
  /*     GracePrintf("xaxis tick minor %d", (int) data->currenttime/100); */
  GracePrintf("yaxis tick major %g", (1.25*maxY)/12.5 );

  if ( Model_isSetName(data->model->m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getName(data->model->m),
		"phase diagram");
  else if  ( Model_isSetId(data->model->m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getId(data->model->m),
		"phase diagram");
  else
    GracePrintf("subtitle \"model has no name, %s/id\"", "phase diagram");
      
  GracePrintf("xaxis label \"species 1\"");
  GracePrintf("yaxis label \"species 2\"");

  printf("Please enter the IDs and NOT the NAMES of species!\n");
  printf("In interactive mode press 'c' to see ID/NAME pairs.\n\n");
  printf("Please enter the ID of the species for the x axis: ");
  x = get_line(stdin);
  x = util_trim(x);
  GracePrintf("xaxis label \"%s\"", x);
  GracePrintf("redraw");
  
  printf("Please enter the ID of the species for the y axis: ");
  y = get_line(stdin);
  y = util_trim(y);
  GracePrintf("yaxis label \"%s\"", y);
  GracePrintf("redraw");
  
  /* check if species exist */
  xvalue = 1;
  yvalue = 1;
  for ( j=0; j<data->model->neq; j++ ) {
    if ( !strcmp(x, data->model->names[j]) ) {
      xvalue = 0;
      GracePrintf("xaxis label \"%s\"", data->model->names[j]);
    }
    if ( !strcmp(y, data->model->names[j]) ) {
      yvalue = 0;
      GracePrintf("yaxis label \"%s\"", data->model->names[j]);
    }
  }
  if ( xvalue || yvalue ) {
    fprintf(stderr, "One of the entered species does not exist.\n");
    GraceClose();
    fprintf(stderr, "XMGrace subprocess closed. Please try again");
    free(x);
    free(y);
    return;
  }

  fprintf(stderr, "Printing phase diagram to XMGrace!\n");

  for ( i=0; i<=results->nout; ++i ) {     
    for ( j=0; j<data->model->neq; j++ ){
      if ( !strcmp(x, data->model->names[j]) ) {
	xvalue = results->value[j][i];
      }
      if ( !strcmp(y, data->model->names[j]) ) {
	yvalue = results->value[j][i];
      }
    }
    GracePrintf("g0.s1 point %g, %g", xvalue, yvalue);

    if ( yvalue > maxY ) {
      maxY = 1.25*yvalue;
      GracePrintf("world ymax %g", maxY);
      GracePrintf("yaxis tick major %g", maxY/10);      
    }
    if ( xvalue > maxX ) {
      maxX = 1.25*xvalue;
      GracePrintf("world xmax %g", maxX);
      GracePrintf("xaxis tick major %g", maxX/10);
    }

    /*
      redrawing on each 10th step gives an impression,
      how fast the two values change within the phase
      diagram.
    */
    if ( i%10 == 0 ) {
      GracePrintf("redraw");
    }
  }

  GracePrintf("redraw");
  closeXMGrace(data, "phase");
  free(x);  
  free(y);

#endif

  return;
}


#if USE_GRACE


/*
  This function prints the time course of reaction fluxes
  to XMGrace
*/

static int printXMGReactionTimeCourse ( cvodeData_t *data )
{

  int i, j, n;
  double maxY, minY, result;
  
  Model_t *m;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;
  
  odeModel_t *om = data->model;
  cvodeResults_t *results = data->results;

  maxY = 0.0;
  minY = 0.0;

  fprintf(stderr,
	  "Printing time development of reaction fluxes to XMGrace!\n");


  if ( om->m == NULL ) {
    fprintf(stderr, "Error: No reaction model availabe\n");
    return 1;
  }
  else m = om->m;

  if ( openXMGrace(data) > 0 ) {
    fprintf(stderr,  "Error: Couldn't open XMGrace\n");
    return 1;
  }
  
  GracePrintf("yaxis label \"%s\"", "flux [substance/time]");
  if ( Model_isSetName(m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getName(m),
		"reaction flux time courses");
  else if  ( Model_isSetId(m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getId(m),
		"reaction flux time courses");
  else 
    GracePrintf("subtitle \"model has no name, %s/id\"",
		"reaction flux time courses");


  /* print legend */  
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    if ( Reaction_isSetName(r) )
      GracePrintf("g0.s%d legend  \"%s: %s \"\n", i+1,
		  Reaction_getId(r), Reaction_getName(r));
    else
      GracePrintf("g0.s%d legend  \"%s \"\n", i+1, Reaction_getId(r));      
  }  
  GracePrintf("legend 1.155, 0.85");
  GracePrintf("legend font 8");
  GracePrintf("legend char size 0.4");

  if(!(kls = (ASTNode_t **)calloc(Model_getNumReactions(m),
				  sizeof(ASTNode_t *)))) 
    fprintf(stderr, "failed!\n");
  
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    kl = Reaction_getKineticLaw(r);
    kls[i] = copyAST(KineticLaw_getMath(kl));
    AST_replaceNameByParameters(kls[i], KineticLaw_getListOfParameters(kl));
    AST_replaceConstants(m, kls[i]);
  }
  
  /* evaluate flux for each time point and print to XMGrace */
  
  for ( i=0; i<=results->nout; i++ ) {  
    n = 1;
    /* set time and variable values to values at time[k] */
    data->currenttime = results->time[i];
    for ( j=0; j<data->model->neq; j++ )
      data->value[j] = results->value[j][i];

    /* evaluate kinetic law expressions */
    for ( j=0; j<Model_getNumReactions(m); j++ ) {
      result = evaluateAST(kls[j], data);
      if ( result > maxY ) {
	maxY = result;
	GracePrintf("world ymax %g", 1.25*maxY);
      }
      if ( result < minY ) {
	minY = result;
	GracePrintf("world ymin %g", 1.25*minY);
      }
      GracePrintf("g0.s%d point %g, %g",
		  n, results->time[i], result);
      n++;
    }
  }

  GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
  GracePrintf("redraw");
  closeXMGrace(data, "flux");

  /* free temporary ASTNodes */
  for ( i=0; i<Model_getNumReactions(m); i++ ) 
    ASTNode_free(kls[i]);
  free(kls);

  return 0;
  
}


/*
  This function prints the values of the Jacobian Matrix
  for each simulated time point to XMGrace
*/

static int printXMGJacobianTimeCourse ( cvodeData_t *data )
{
  int i, j, k, n;
  double maxY;
  double minY; 
  double result;

  cvodeResults_t *results;

  maxY = 0.0;
  minY = 0.0;
  

  fprintf(stderr,
	  "Printing time development of the jacobian matrix to XMGrace!\n");

  results = data->results;

  if ( openXMGrace(data) > 0 ) {
    fprintf(stderr,
	    "Error: Couldn't open XMGrace\n");
    return 1;
  }
  
  GracePrintf("yaxis label \"%s\"", "jacobian matrix value");
  if ( Model_isSetName(data->model->m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getName(data->model->m),
		"jacobian matrix time course");
  else if  ( Model_isSetId(data->model->m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getId(data->model->m),
		"jacobian matrix time course");
  else 
    GracePrintf("subtitle \"model has no name, %s/id\"",
		"jacobian matrix time course");


  /* print legend */  
  n = 1;  
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      GracePrintf("g0.s%d legend  \"%s / %s\"\n",
		  n, data->model->names[i], data->model->names[j]);
      n++;
    }
  }  
  GracePrintf("legend 1.155, 0.85");
  GracePrintf("legend font 8");
  GracePrintf("legend char size 0.4");

  /* evaluate jacobian matrix for each time point and print to XMGrace */
  
  for ( k = 0; k<=results->nout; k++ ) {  
    n = 1;
    data->currenttime = results->time[i];
    for ( i=0; i<data->model->neq; i++ ) {
      
      /* set specie values to values at time[k] */
      data->value[i] = results->value[i][k];
      
      for ( j=0; j<data->model->neq; j++ ) {
	
	result =  evaluateAST(data->model->jacob[i][j], data);
	
	if ( result > maxY ) {	  
	  maxY = result;
	  GracePrintf("world ymax %g", 1.25*maxY);
	}
	if ( result < minY ) {
	  minY = result;
	  GracePrintf("world ymin %g", 1.25*minY);
	}
	
	GracePrintf("g0.s%d point %g, %g",
		    n, results->time[k], result);
	n++;

      }
    }
    /*
    if ( k%10 == 0 ) {
      GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
      GracePrintf("redraw");
    }
    */
  }
  GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
  GracePrintf("redraw");
  closeXMGrace(data, "jac");


  return 0;
}

/*
  This function prints the values of the ODEs
  for each simulated time point to XMGrace
*/

static int printXMGOdeTimeCourse(cvodeData_t *data)
{  
  int i, j, n;
  double maxY;
  double minY; 
  double result;

  cvodeResults_t *results;

  maxY = 0.01;
  minY = 0.0;
  

  fprintf(stderr,
	  "Printing time development of the ODEs (rates) to XMGrace!\n");

  results = data->results;

  if ( openXMGrace(data) > 0 ) {
    fprintf(stderr,
	    "Error: Couldn't open XMGrace\n");
    return 1;     
  }

  GracePrintf("yaxis label \"%s\"", "ODE values");
  if ( Model_isSetName(data->model->m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getName(data->model->m),
		  "ODEs time course");
  else if  ( Model_isSetId(data->model->m) )
    GracePrintf("subtitle \"%s, %s\"", Model_getId(data->model->m),
		  "ODEs time course");
  else 
    GracePrintf("subtitle \"model has no name/id, %s\"",
		  "ODEs time course");

  /* print legend */  
  if ( printXMGLegend(data, data->model->neq) > 0 ){
    fprintf(stderr,
	    "Warning: Couldn't print legend\n");
    return 1;
  }


  /* evaluate ODE at each time point and print to XMGrace */

  for ( i=0; i<=results->nout; ++i ) {
    n = 1;
    data->currenttime = results->time[i];
    for ( j=0; j<data->model->neq; j++ ) {
      data->value[j] = results->value[j][i];
    }
    for ( j=0; j<data->model->neq; j++ ) {
      result = evaluateAST(data->model->ode[j],data);
      if ( result > maxY ) {
	maxY = result;
	GracePrintf("world ymax %g", 1.25*maxY);
      }
      if ( result < minY ) {
	minY = result;
	GracePrintf("world ymin %g", 1.25*minY);
      }

      GracePrintf("g0.s%d point %g, %g",
		  n, results->time[i], result);
      n++;     
    }

    /*
    if ( i%10 == 0 ) {
      GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
      GracePrintf("redraw");
    }
    */
  }
  GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
  GracePrintf("redraw");
  closeXMGrace(data, "rates");

  return 0;
}


/*
  This function prints integration results to XMGrace
*/

static int printXMGConcentrationTimeCourse(cvodeData_t *data)
{
  int i,j,n;
  double maxY;
  double minY;

  cvodeResults_t *results;

  results = data->results;
  
  maxY = 1.0;
  minY = 0.0;

  fprintf(stderr, "Printing results to XMGrace!\n");
  if ( Opt.Sensitivity  && results->sensitivity != NULL )
    fprintf(stderr, "SORRY: sensitivities can not be printed to XMGrace\n");
  
  if ( openXMGrace(data) > 0 ){
    fprintf(stderr,
	    "Error: Couldn't open XMGrace\n");
    return 1;     
  }
  if ( printXMGLegend(data, data->nvalues-data->model->nconst) > 0 ){
    fprintf(stderr,
	    "Warning: Couldn't print legend\n");
    return 1;
  }
  
  for ( i=0; i<=results->nout; ++i ) {
    n=1; 
    for ( j=0; j<data->nvalues-data->model->nconst; j++ ) {
      if ( results->value[j][i] > maxY ) {
	maxY = results->value[j][i];
	GracePrintf("world ymax %g", 1.25*maxY);	
      }
      if ( results->value[j][i] < minY ) {
	minY = results->value[j][i];
	GracePrintf("world ymin %g", 1.25*minY);
      }
      GracePrintf("g0.s%d point %g, %g", 
		  n, results->time[i], results->value[j][i]);
      n++;
    }
  
    /*  if ( i%10 == 0 ) {
      GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
      GracePrintf("redraw");
      }
    */
  }
  GracePrintf("yaxis tick major %g", 1.25*(fabs(maxY)+fabs(minY))/10);
  GracePrintf("redraw");
  closeXMGrace(data, "species");

  return 0;
}


static int printXMGLegend(cvodeData_t *data, int nvalues)
{
  int i, found;
  odeModel_t *om = data->model;
  Model_t *m = om->simple;
  Species_t *s;
  Parameter_t *p;
  Compartment_t *c;

  
  for ( i=0; i<nvalues; i++ ) {
    found = 0;
    if ( (s = Model_getSpeciesById(m, om->names[i])) != NULL ) {
      if ( Species_isSetName(s) ) {
	GracePrintf("g0.s%d legend  \"%s: %s\"\n", i+1,
		    om->names[i], Species_getName(s));
	found++;
      }
    }
    else if ( (c = Model_getCompartmentById(m, om->names[i])) ) {
      if ( Compartment_isSetName(c) ) {
	GracePrintf("g0.s%d legend  \"%s: %s\"\n", i+1,
		    om->names[i], Compartment_getName(c));
	found++;
      }
    }
    else if ( (p = Model_getParameterById(m, om->names[i])) ) {
      if ( Parameter_isSetName(p) ) {
	GracePrintf("g0.s%d legend  \"%s: %s\"\n", i+1,
		    om->names[i], Parameter_getName(p));
	found++;
      }
    }
    if ( found == 0 )
      GracePrintf("g0.s%d legend  \"%s\"\n", i+1, om->names[i]);
  }


  GracePrintf("legend 1.155, 0.85");
  GracePrintf("legend font 8");
  GracePrintf("legend char size 0.4");

  return 0;
}


/* Opens XMGrace */

static int openXMGrace(cvodeData_t *data)
{
  double maxY;

  /* Open XMGrace */
  if ( !GraceIsOpen() ) {     
    GraceRegisterErrorFunction(grace_error); 
    /* Start Grace with a buffer size of 2048 and open the pipe */
    if ( GraceOpen(2048) == -1 ) {
      fprintf(stderr, "Can't run Grace. \n");
      return 1;
    }
    else if ( GraceIsOpen() ) {
      maxY = 1.0;
      /*
	"with g%d" might become useful, when printing multiple
	graphs into one XMGrace subprocess.
      */
      /* GracePrintf("with g%d", data->results->xmgrace); */
      GracePrintf("world xmax %g", data->currenttime);
      GracePrintf("world ymax %g", 1.25*maxY);
      GracePrintf("xaxis tick major %g", data->currenttime/10);
      /* GracePrintf("xaxis tick minor %d", (int) data->currenttime/100); */
      GracePrintf("yaxis tick major %g", (1.25*maxY)/12.5 );
      GracePrintf("xaxis label font 4");
      GracePrintf("xaxis label \"time\"");
      GracePrintf("xaxis ticklabel font 4");
      GracePrintf("xaxis ticklabel char size 0.7");
      GracePrintf("yaxis label font 4");
      GracePrintf("yaxis label \"concentration\"");    
      GracePrintf("yaxis ticklabel font 4");
      GracePrintf("yaxis ticklabel char size 0.7");
      if ( Model_isSetName(data->model->simple) )
	GracePrintf("subtitle \"%s\"", Model_getName(data->model->simple));
      else if  ( Model_isSetId(data->model->simple) )
	GracePrintf("subtitle \"%s\"", Model_getId(data->model->simple));
      else 
	GracePrintf("subtitle \"model has no name/id\"");
      GracePrintf("subtitle font 8");   
    }
  }
  else {
    fprintf(stderr, "Please close XMGrace first.\n");
    return 1;
  }

  return 0;
}

/* Closes the pipe to Grace and saves a grace data file
   when option -w/--write was given */

static int closeXMGrace(cvodeData_t *data, char *safename)
{
  if ( Opt.Write ) {
    fprintf(stderr, "Saving XMGrace file as \"%s_%s_t%g.agr\"\n",
	    Opt.ModelFile,
	    safename,
	    data->results->time[data->results->nout]);
    GracePrintf("saveall \"%s_%s_t%g.agr\"",
		Opt.ModelFile,
		safename,
		data->results->time[data->results->nout]);
  }

  GraceClosePipe();
  return(0);
}

static void grace_error(const char *msg)
{
  fprintf(stderr, "library message: \"%s\"\n", msg);  
}

#endif

/* End of file */
