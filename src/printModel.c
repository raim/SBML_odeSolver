/*
  Last changed Time-stamp: <2005-06-22 14:44:16 raim>
  $Id: printModel.c,v 1.4 2005/06/28 13:50:19 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Header Files from the SBML Library libsbml */
#include <sbml/SBMLTypes.h>
#include <sbml/util/util.h>

/* System specific definitions,
   created by configure script */
#include "config.h"

/* Own Header Files */
#include "sbmlsolver/util.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/processAST.h"


/*
  XMGrace Library dependent functions,
  printing results of simulation to XMGrace
*/
#if USE_GRACE
/* Header Files for XMGrace */
#include <grace_np.h>
/* functions */
static void
grace_error(const char *msg);
static int
printXMGConcentrationTimeCourse(CvodeData data);
static int
printXMGOdeTimeCourse(CvodeData data);
static int
printXMGJacobianTimeCourse(CvodeData data);
static int
printXMGLegend(CvodeData data);
static int
openXMGrace(CvodeData data);
static int
closeXMGrace(CvodeData data, char *name);
#endif

void
printModel(Model_t *m)
{
  printf("\n");
  printf("Model Statistics:\n");
  printf(" Model id:     %s\n",
	 Model_isSetId(m) ? Model_getId(m) : "(not set)");
  printf(" Model name:   %s\n",
	 Model_isSetName(m) ? Model_getName(m) : "(not set)"); 
  printf("\n");
  printf(" Compartments: %d\n",  Model_getNumCompartments(m)); 
  printf(" Species:      %d\n",  Model_getNumSpecies(m));
  printf(" Reactions:    %d\n",  Model_getNumReactions(m));
  printf(" Rules:        %d\n",  Model_getNumRules(m));
  printf(" Events:       %d\n",  Model_getNumEvents(m));
  printf(" Functions:    %d\n",  Model_getNumFunctionDefinitions(m) ); 
  printf("\n");
}


void
printSpecies(Model_t *m)
{
  int i, j;
  Species_t *s;  
  Compartment_t *c;

  printf("\n");
  printf("# Initial Conditions for Species and Compartments:\n");
  for ( i=0; i<Model_getNumCompartments(m); i++ ) {
    if ( i== 0 ) printf("# Compartments:\n");
    c = Model_getCompartment(m,i);
    if(Compartment_isSetId(c))
      printf("%s ", Compartment_getId(c));
    if(Compartment_isSetName(c))
      printf("(%s) ", Compartment_getName(c));
    if ( Compartment_isSetVolume(c) )
     printf("= %g; ", Compartment_getSize(c));
    printf("%s", Compartment_getConstant(c) ? "" : "variable; ");
    if(Compartment_isSetOutside(c))
      printf("outside %s; ", Compartment_getOutside(c));
   /*  printf("\n"); */
    printf("dimensions %d; ", Compartment_getSpatialDimensions(c));

    if(Compartment_isSetUnits(c))
      printf("[%s]; ", Compartment_getUnits(c));
    printf("\n");
    
    printf("# Species concentrations in `compartment' %s\n",
	   Compartment_getId(c));
    for(j=0;j<Model_getNumSpecies(m);j++){
      s = Model_getSpecies(m,j);      
      if(strcmp(Species_getCompartment(s), Compartment_getId(c))==0){  

	printf("%s ", Species_getId(s));
	if(Species_isSetName(s))
	  printf("(%s) ", Species_getName(s));
	
	if ( Species_isSetInitialAmount(s) )
	  printf("= %g/%g; ",
		 Species_getInitialAmount(s),
		 Compartment_getSize(c));
	else if ( Species_isSetInitialConcentration(s) )
	  printf("= %g; ", Species_getInitialConcentration(s));
	else
	  printf("# no initial value;");	
	printf("%s", Species_getBoundaryCondition(s) ? "boundary;" : "");
	printf("%s", Species_getConstant(s) ? "constant;" : "");
	if(Species_isSetCharge(s))
	  printf("charge = %d; ", Species_getCharge(s));

	printf("\n");
      }     
    }
    printf("\n");  
  }  
}

void
printReactions(Model_t *m)
{
  
  int i,j,k;
  Reaction_t *r;
  SpeciesReference_t *sref;
  KineticLaw_t *kl;
  Rule_t *rl;
  AssignmentRule_t *asr;
  AlgebraicRule_t *alr;
  RateRule_t *rr;
  Event_t *e;
  EventAssignment_t *ea;
  Parameter_t *p;
  FunctionDefinition_t *f;
  SBMLTypeCode_t type;
  const ASTNode_t *math;

  math = NULL;
  
  printf("\n");
  for(i=0;i<Model_getNumParameters(m);i++){
    if(i==0)
      printf("# Global parameters:\n");
    p = Model_getParameter(m,i);
    if(Parameter_isSetId(p))
      printf("%s ",  Parameter_getId(p));
    if(Parameter_isSetName(p))
      printf("(%s) ", Parameter_getName(p));
    if(Parameter_isSetValue(p))
      printf("= %g; ", Parameter_getValue(p));
    if(Parameter_isSetUnits(p))
      printf("[%s]; ", Parameter_getUnits(p));
    if(!Parameter_getConstant(p))
      printf("(variable);");
    printf("\n");
    
    if ( i==Model_getNumParameters(m)-1 )
      printf("\n");
  }

  printf("# Reactions:\n");
  for ( i=0; i<Model_getNumReactions(m); i++ ) {    
    r = Model_getReaction(m,i);
  
    printf("%s: %s",
	   Reaction_isSetName(r) ? Reaction_getName(r) : Reaction_getId(r),
	   Reaction_getFast(r) ? "(fast)" : "");
    for ( k=0; k<Reaction_getNumReactants(r); k++ ) {
      sref = Reaction_getReactant(r,k);

      if ( SpeciesReference_isSetStoichiometryMath(sref) )	
	printf("%s ",
	       SBML_formulaToString(\
		   SpeciesReference_getStoichiometryMath(sref)));
      else 
	if ( SpeciesReference_getStoichiometry(sref) != 1. )
	  printf("%g ",  SpeciesReference_getStoichiometry(sref));
	
      printf("%s", SpeciesReference_getSpecies(sref));
      if(k+1<Reaction_getNumReactants(r))
	printf("%s", " + ");
    }
    
    printf("%s", Reaction_getReversible(r) ? " <-> " : " -> ");
    for ( k=0; k<Reaction_getNumProducts(r); k++ ) {
      sref = Reaction_getProduct(r,k);
      if ( SpeciesReference_isSetStoichiometryMath(sref) )
	printf("%s ",
	       SBML_formulaToString(\
		   SpeciesReference_getStoichiometryMath(sref)));
      else
	if ( SpeciesReference_getStoichiometry(sref) != 1. )
	  printf("%g ", SpeciesReference_getStoichiometry(sref));
      
      printf("%s", SpeciesReference_getSpecies(sref));
      if(k+1<Reaction_getNumProducts(r))
	printf("%s", " + ");
    }
    printf(";  ");
    if(Reaction_isSetKineticLaw(r)){
      kl = Reaction_getKineticLaw(r);
      math = KineticLaw_getMath(kl);
      printf("%s;", SBML_formulaToString(math));
      for(k=0;k<KineticLaw_getNumParameters(kl);k++){
	
	p = KineticLaw_getParameter(kl,k);
	printf(" %s",  Parameter_getId(p));
	if(Parameter_isSetName(p))
	  printf(" (%s)", Parameter_getName(p));
	if(Parameter_isSetValue(p))
	  printf(" = %g", Parameter_getValue(p));
	if(Parameter_isSetUnits(p))
	  printf(" [%s]", Parameter_getUnits(p));
	if ( !Parameter_getConstant(p) )
	  printf(" (variable)");
	printf(";");
      }
     /*  printf("\n"); */
    }else
      printf("#   no rate law is set for this reaction.");
    printf("\n");
  }
    
  for(i=0;i<Model_getNumRules(m);i++){
    rl = Model_getRule(m,i);
    if ( i == 0 ) {
      printf("# Rules:\n");
    }
    type = SBase_getTypeCode((SBase_t *)rl);
     
     
    if ( type == SBML_RATE_RULE ) {
      rr =  (RateRule_t *) rl;
      printf(" rateRule:       d%s/dt = ", RateRule_getVariable(rr));
    }
    if ( type == SBML_ALGEBRAIC_RULE ) {
      alr = (AlgebraicRule_t *) rl;
      printf(" algebraicRule:       0 = ");
    }
    if ( type == SBML_ASSIGNMENT_RULE ) {
      asr = (AssignmentRule_t *) rl;
      printf(" assignmentRule (%s): %s = ",
	     RuleType_toString(AssignmentRule_getType(asr)),
	     AssignmentRule_getVariable(asr));
    }
    if(!Rule_isSetMath(rl)){
      if(Rule_isSetFormula(rl)){
	Rule_setMathFromFormula(rl);
	
      }
    }
    if(Rule_isSetMath(rl))
      printf("%s\n", SBML_formulaToString(Rule_getMath(rl)));
	     
  }
  printf("\n");

  for(i=0;i<Model_getNumEvents(m);i++){
    if(i==0)
      printf("# Events:\n");
    e = Model_getEvent(m,i);
    if(Event_isSetId(e))
      printf("%s: ", Event_getId(e));
    if(Event_isSetName(e))
      printf("(%s) ", Event_getName(e));   
    if(Event_isSetTrigger(e)) {
      math = Event_getTrigger(e);
      printf("trigger: %s\n", SBML_formulaToString(math));
    }
    if(Event_isSetDelay(e))
      printf("delay: %s;\n", SBML_formulaToString(Event_getDelay(e)));
    if(Event_isSetTimeUnits(e))
      printf("time Units: %s;\n", Event_getTimeUnits(e));
    for(k=0;k<Event_getNumEventAssignments(e);k++){      
      ea = Event_getEventAssignment(e,k);
      if(EventAssignment_isSetVariable(ea))
	printf("  event:  %s = %s;\n",
	       EventAssignment_getVariable(ea),
	       EventAssignment_isSetMath(ea) ?
	       SBML_formulaToString(EventAssignment_getMath(ea)) :
	       "# no math set;\n");
    }

    if(i==Model_getNumEvents(m)-1)
       printf("\n"); 
  }  


  for ( i=0; i<Model_getNumFunctionDefinitions(m); i++ ) {

    if ( i==0 ) printf("# Functions:\n");

    f = Model_getFunctionDefinition(m,i);
    if ( FunctionDefinition_isSetName(f) )
      printf("%s: ", FunctionDefinition_getName(f));
    if(FunctionDefinition_isSetId(f) && FunctionDefinition_isSetMath(f)){
      printf("%s( ", FunctionDefinition_getId(f));
      math = FunctionDefinition_getMath(f);
	for(j=0;j<ASTNode_getNumChildren(math)-1;j++){
	  printf("%s", SBML_formulaToString(ASTNode_getChild(math, j)));
	  if(j<ASTNode_getNumChildren(math)-2)
	    printf(", ");
	  if(j==ASTNode_getNumChildren(math)-2)
	    printf(") = ");
	}
      printf("%s;", SBML_formulaToString(ASTNode_getRightChild(math)));
    }
    printf("\n");
  } 
  
}


void
printODEsToSBML(CvodeData data){
      
  SBMLDocument_t *d;
  char *model;
  d = SBMLDocument_create();
  SBMLDocument_setModel(d, data->model->simple);
  model = writeSBMLToString(d);
  printf("%s", model);
  free(model);
}


void
printODEs(CvodeData x){
  
  int i;
  char *f;
  odeModel_t *data = x->model;

  printf("\n");
  printf("# Derived system of Ordinary Differential Equations (ODEs):\n");

  printf("# Parameters:\n");
  for ( i=0; i<data->nconst; i++ ) {
    printf("%s = %g\n", data->parameter[i], x->pvalue[i]);
  }
  
  for ( i=0; i<data->neq; i++ ) {
    if ( i == 0 ) {
      printf("# ODEs:\n");
    }
    f = SBML_formulaToString(data->ode[i]);
    printf("%d: d%s/dt =  %s;\n",
	   i+1,
	   data->speciesname[i],
	   f);
    free(f);
  }
  printf("\n");

  /* change in behaviour AMF 27th June 05 
  if ( data->errors>0 ) {
    printf("# %d ODEs could not be constructed."
	   " Try to add or correct kinetic laws!\n",
	   data->errors); 
  }*/
  return;
}

void 
printJacobian(CvodeData x){
  
  odeModel_t *data = x->model;
  int i, j;
  if ( data == NULL ) {
    fprintf(stderr, "No data available.\n");
    return;
  }

  if ( data->jacob == NULL ) {
    fprintf(stderr, "Jacobian Matrix has not been constructed.\n");
    return;
  }
  if ( Opt.InterActive == 0 && Opt.Jacobian == 0 ) {
    fprintf(stderr, "Don't use option -j to also print the"
	    " jacobian matrix expressions.\n");
    fprintf(stderr, "If one or more of the ODEs were not differentiable"
	    " option -j was set\nand jacobian matrix construction"
	    " deactivated automatically\n");
    return;
  }
  
  printf("\n");
  printf("# Jacobian Matrix:\n");
  for ( i=0; i<data->neq; i++ ) {
    printf("# %s: \n", data->speciesname[i]);
    for ( j=0; j<data->neq; j++ ) {
      printf("  (d[%s]/dt)/d[%s] = %s;\n", 
	     data->speciesname[i], data->speciesname[j], 
	     SBML_formulaToString(data->jacob[i][j]));
    }
  }
  printf("\n");
  fprintf(stderr, "Use option -j to avoid printing the"
	  " jacobian matrix expressions.\n");
  return;
}

/* The following functions print results of integration */

void
printDeterminantTimeCourse(CvodeData data) {
  int i,j;
  FILE *f;
  CvodeResults results;

  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }
  
  f = data->outfile;
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
    fprintf(f, "%g\n", evaluateAST(data->model->det, data));
  }
  fprintf(f, "##DETERMINANT OF THE JACOBIAN MATRIX\n");
  fprintf(f, "#t det(j)\n");
  fflush(f);
}


void 
printJacobianTimeCourse(CvodeData data){

  int i, j, k;
  FILE *f;
  CvodeResults results;


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
  f = data->outfile;
  if ( Opt.PrintMessage )
    fprintf(stderr, "Printing time course of the jacobian matrix.\n\n");
  
  fprintf(f, "#t ");
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      fprintf(f, "%s ", data->model->species[j]);
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
      fprintf(f, "%s ", data->model->species[j]);
    }
  }
  fprintf(f, "\n");
  fflush(f);
  
  /* Print if simulation was aborted at steady state */
  if(data->steadystate==1){
    fprintf(f, "# Found steady state; aborted at time %g!\n",
	    data->currenttime);
    fprintf(stderr, "Simulation aborted at steady state!\n");
    fprintf(stderr, "Rates at abortion at time  %g: \n", data->currenttime);
    for(i=0;i<data->model->neq;i++) 
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->model->species[i], fabs(evaluateAST(data->model->ode[i],data)));
    fprintf(stderr, "\n");
    fprintf(stderr, "Mean of rates:\n %g, std %g\n\n",
	    data->dy_mean, data->dy_std);
    fprintf(stderr, "\n");     
  }

#if !USE_GRACE

  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr,
	    "odeSolver has been compiled without XMGRACE functionality.\n");
    fprintf(stderr,
	    "The requested data have been printed to stdout instead.\n");
  }

#endif
  
}

void
printOdeTimeCourse(CvodeData data){
  
  int i,j;
  FILE *f;
  CvodeResults results;

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
  
  f = data->outfile;
  results = data->results;
  if ( Opt.PrintMessage )
    fprintf(stderr, "\nPrinting time course of the ODEs.\n\n");
    
  fprintf(f, "#t ");
  for (i=0; i<data->model->neq; i++ ) {
    fprintf(f, "%s ", data->model->species[i]);
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
    fprintf(f, "%s ", data->model->species[i]);
  }
  fprintf(f, "\n");
  fflush(f);

  /* Print if simulation was aborted at steady state */
  if ( data->steadystate==1 ) {
    fprintf(f, "# Found steady state; aborted at time %g!\n",
	    data->currenttime);
    fprintf(stderr, "Simulation aborted at steady state!\n");
    fprintf(stderr, "Rates at abortion at time  %g: \n", data->currenttime);
    for(i=0;i<data->model->neq;i++)
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->model->species[i], fabs(evaluateAST(data->model->ode[i],data)));
    fprintf(stderr, "\n");
    fprintf(stderr, "Mean of rates:\n %g, std %g\n\n",
	    data->dy_mean, data->dy_std);
    fprintf(stderr, "\n");
  }
  
#if !USE_GRACE

  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr,
	    "odeSolver has been compiled without XMGRACE functionality.\n");
    fprintf(stderr,
	    "The requested data have been printed to stdout instead.\n");
  }

#endif
  
}

void
printReactionTimeCourse(CvodeData data) {

  int i, j;
  CvodeResults results;
  Model_t *m;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;
  FILE *f;

  if ( data == NULL || data->results == NULL ) {
    Warn(stderr, "No results, please integrate first.\n");
    return;
  }

#if USE_GRACE
  if ( Opt.Xmgrace == 1 ) {
    fprintf(stderr, "Sorry, kinetic law timecourse can only be\n");
    fprintf(stderr, "printed to outfile (default: stdout)\n");
  }
#endif
  
  m = data->model->m;
  f = data->outfile;
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

  /* Print if simulation was aborted at steady state */
  if ( data->steadystate==1 ) {
    fprintf(f, "# Found steady state; aborted at time %g!\n",
	    data->currenttime);
    fprintf(stderr, "Simulation aborted at steady state!\n");
    fprintf(stderr, "Rates at abortion at time  %g: \n", data->currenttime);
    for(i=0;i<data->model->neq;i++)
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->model->species[i], fabs(evaluateAST(data->model->ode[i],data)));
    fprintf(stderr, "\n");
    fprintf(stderr, "Mean of rates:\n %g, std %g\n\n",
	    data->dy_mean, data->dy_std);
    fprintf(stderr, "\n");
  }
}

void
printConcentrationTimeCourse(CvodeData data){
  
  int i,j;
  FILE *f;
  CvodeResults results;

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
  
  f = data->outfile;

  if ( Opt.PrintMessage )
    fprintf(stderr,
	    "\nPrinting time course of the species concentrations.\n\n");
  
  results = data->results;
  fprintf(f, "#t ");
  for(i=0;i<data->model->neq;i++) fprintf(f, "%s ", data->model->species[i]);
  for(i=0;i<data->model->nass;i++) fprintf(f, "%s ", data->model->ass_parameter[i]);
  for(i=0;i<data->model->nconst;i++) fprintf(f, "%s ", data->model->parameter[i]);
  fprintf(f, "\n");
  fprintf(f, "##CONCENTRATIONS\n");
  for ( i=0; i<=results->nout; ++i ) {
    fprintf(f, "%g ", results->time[i]);
    for ( j=0; j<data->model->neq; j++ ) {
      fprintf(f, "%g ", results->value[j][i]);
    }
    for ( j=0; j<data->model->nass; j++ ) {
      fprintf(f, "%g ", results->avalue[j][i]);
    }
    for ( j=0; j<data->model->nconst; j++ ) {
      fprintf(f, "%g ", results->pvalue[j][i]);
    }
    fprintf(f, "\n");
  }
  fprintf(f, "##CONCENTRATIONS\n");
  fprintf(f, "#t ");
  for(i=0;i<data->model->neq;i++) fprintf(f, "%s ", data->model->species[i]);
  for(i=0;i<data->model->nass;i++) fprintf(f, "%s ", data->model->ass_parameter[i]);
  for(i=0;i<data->model->nconst;i++) fprintf(f, "%s ", data->model->parameter[i]);
  fprintf(f, "\n\n");
  
  /* Print if simulation was aborted at steady state */
  if ( data->steadystate == 1 ) {
    fprintf(f, "# Found steady state; aborted at time %g!\n",
	    data->currenttime);
    fprintf(stderr, "Simulation aborted at steady state!\n");
    fprintf(stderr, "Rates at abortion at time  %g: \n", data->currenttime);
    for(i=0;i<data->model->neq;i++) 
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->model->species[i], fabs(evaluateAST(data->model->ode[i],data)));
    fprintf(stderr, "\n");
    fprintf(stderr, "Mean of rates:\n %g, std %g\n\n",
	    data->dy_mean, data->dy_std);
    fprintf(stderr, "\n");     
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


/*
  The following functions print results of the integration
  to XMGrace.
*/

/*
  The function printPhase(data) asks the user to enter 2 species of
  the integrated system and prints a phase diagram to XMGrace.
*/

void
printPhase(CvodeData data) {

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

  CvodeResults results;

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
  /*     GracePrintf("xaxis tick minor %d", (int) data->tout/100); */
  GracePrintf("yaxis tick major %g", (1.25*maxY)/12.5 );
  GracePrintf("subtitle \"%s, %s\"", data->model->modelName, "phase diagram");
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
    if ( !strcmp(x, data->model->species[j]) ) {
      xvalue = 0;
      GracePrintf("xaxis label \"%s\"", data->model->speciesname[j]);
    }
    if ( !strcmp(y, data->model->species[j]) ) {
      yvalue = 0;
      GracePrintf("yaxis label \"%s\"", data->model->speciesname[j]);
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
      if ( !strcmp(x, data->model->species[j]) ) {
	xvalue = results->value[j][i];
      }
      if ( !strcmp(y, data->model->species[j]) ) {
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
  This function prints the values of the Jacobian Matrix
  for each simulated time point to XMGrace
  using subfunctions openXMGrace(), and printXMGLegend().
*/

static int
printXMGJacobianTimeCourse ( CvodeData data ) {


  int i, j, k, n;
  double maxY;
  double minY; 
  double result;

  CvodeResults results;

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
  GracePrintf("subtitle \"%s, %s\"",
		  data->model->modelName, "jacobian matrix time course");


  /* print legend */  
  n = 1;  
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      GracePrintf("g0.s%d legend  \"%s / %s\"\n",
		  n, data->model->speciesname[i], data->model->speciesname[j]);
      n++;
    }
  }  
  GracePrintf("legend 1.155, 0.85");
  GracePrintf("legend font 8");
  GracePrintf("legend char size 0.6");

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
  using subfunctions openXMGrace(), and printXMGLegend().
*/

static int
printXMGOdeTimeCourse(CvodeData data){
  
  
  int i, j, n;
  double maxY;
  double minY; 
  double result;

  CvodeResults results;

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
  GracePrintf("subtitle \"%s, %s\"", data->model->modelName,
		  "ODEs time course");

  /* print legend */  
  n = 1;  
  for ( i=0; i<data->model->neq; i++ ) {
      GracePrintf("g0.s%d legend  \"%s\"\n",
		  n, data->model->speciesname[i]);
      n++;
  }  
  GracePrintf("legend 1.155, 0.85");
  GracePrintf("legend font 8");
  GracePrintf("legend char size 0.6");

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
  using subfunctions openXMGrace(), and printXMGLegend().
*/

static int
printXMGConcentrationTimeCourse(CvodeData data){

  int i,j,n;
  double maxY;
  double minY;

  CvodeResults results;

  maxY = 1.0;
  minY = 0.0;

  fprintf(stderr, "Printing results to XMGrace!\n");

  results = data->results;
  
  if ( openXMGrace(data) > 0 ){
    fprintf(stderr,
	    "Error: Couldn't open XMGrace\n");
    return 1;     
  }
  if ( printXMGLegend(data) > 0 ){
    fprintf(stderr,
	    "Warning: Couldn't print legend\n");
    return 1;
  }
    

  
  for ( i=0; i<=results->nout; ++i ) {
    n=1; 
    for ( j=0; j<data->model->neq; j++ ) {
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
    for ( j=0; j<data->model->nass; j++ ) {
      if ( results->avalue[j][i] > maxY ) {
	maxY = results->avalue[j][i];
	GracePrintf("world ymax %g", 1.25*maxY);	
      }
      if ( results->avalue[j][i] < minY ) {
	minY = results->avalue[j][i];
	GracePrintf("world ymin %g", 1.25*minY);
      }
      GracePrintf("g0.s%d point %g, %g", 
		  n, results->time[i], results->avalue[j][i]);
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


static int
printXMGLegend(CvodeData data){

  int i;
  int n;
  n=1;
  
  for ( i=0; i<data->model->neq; i++ ) {
    GracePrintf("g0.s%d legend  \"%s\"\n", n, data->model->speciesname[i]);    
    n++;
  }
  for ( i=0; i<data->model->nass; i++ ) {
    GracePrintf("g0.s%d legend  \"%s\"\n", n, data->model->ass_parameter[i]);    
    n++;
  }

  GracePrintf("legend 1.155, 0.85");
  GracePrintf("legend font 8");
  GracePrintf("legend char size 0.6");

  return 0;
}


/* Opens XMGrace */

static int
openXMGrace(CvodeData data){

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
     /*  GracePrintf("with g%d", data->results->xmgrace); */
      GracePrintf("world xmax %g", data->tout);
      GracePrintf("world ymax %g", 1.25*maxY);
      GracePrintf("xaxis tick major %g", data->tout/10);
      /*     GracePrintf("xaxis tick minor %d", (int) data->tout/100); */
      GracePrintf("yaxis tick major %g", (1.25*maxY)/12.5 );
      GracePrintf("xaxis label font 4");
      GracePrintf("xaxis label \"time\"");
      GracePrintf("xaxis ticklabel font 4");
      GracePrintf("xaxis ticklabel char size 0.7");
      GracePrintf("yaxis label font 4");
      GracePrintf("yaxis label \"concentration\"");    
      GracePrintf("yaxis ticklabel font 4");
      GracePrintf("yaxis ticklabel char size 0.7");
      GracePrintf("subtitle \"%s\"", data->model->modelName);
      GracePrintf("subtitle font 8");   
    }
  }
  else {
    fprintf(stderr, "Please close XMGrace first.\n");
    return 1;
  }

  return 0;
}

/** Closes the pipe to Grace and
    saves a grace data file when option -w/--write
    was given */

static int
closeXMGrace(CvodeData data, char *safename) {

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

static void
grace_error(const char *msg) {
  fprintf(stderr, "library message: \"%s\"\n", msg);  
}

#endif

/* End of file */
