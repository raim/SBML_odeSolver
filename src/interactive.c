/*
  Last changed Time-stamp: <2005-08-01 21:15:37 raim>
  $Id: interactive.c,v 1.6 2005/08/02 13:20:28 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/util/util.h> /* only for util_trim */

/* own header files */
#include "sbmlsolver/util.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/sbmlUsingOptions.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/drawGraph.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/solverError.h"


static void
printMenu(void);
static void
setValues(Model_t *m);
static void
setFormat(void);
static SBMLDocument_t * 
loadFile(SBMLReader_t *sr);
static cvodeData_t *
callIntegrator(Model_t *m);

/**
   Interactive Mode:
   Within this while loop the user is asked to choose different
   actions.
   The model and the ODEs constructed from the model
   can be inspected, integrated and the results printed.
   Also graphs of the reaction network, or the species interactions
   as specified in the Jacobian matrix can be constructed. The files
   will be written to the folder from where odeSolver was started.
   When started the user can type 'h' to see what can be done.
   
   NOTE: differences to commandline mode
   b) Setting alternative initial amounts for species is only possible
   from the interactive mode.
   c) Phase Diagrams can only be printed from interactive mode and to
   XMGrace.
*/

void
interactive() {
  
  char *model;
  char *select;
  int quit;
  SBMLDocument_t *d = NULL;
  SBMLReader_t   *sr = NULL;
  Model_t        *m  = NULL;
  cvodeData_t * data     = NULL;
  ASTNode_t *det;

  sr = newSBMLReader(Opt.SchemaPath, Opt.Schema11, Opt.Schema12, Opt.Schema21);
  
  printf("\n\nWelcome to the simple SBML ODE solver.\n");
  printf("You have entered the interactive mode.\n");
  printf("All other commandline options have been ignored.\n");
  printf("Have fun!\n\n");

  initializeOptions();
  /* reset steady state */
  Opt.SteadyState = 0;
  /* activate printing of results to XMGrace */
  Opt.Xmgrace = 1;
  /* activate printing integrator messages */
  Opt.PrintMessage = 1;
  /* deactivate on the fly printing of results */
  Opt.PrintOnTheFly = 0;

  model = concat(Opt.ModelPath, Opt.ModelFile);

  if ( (d = parseModel(model)) == 0 ) {
    if ( Opt.Validate ) {
	Warn(stderr, "Please make sure that path >%s< contains",
	     Opt.SchemaPath);
	Warn(stderr, "the correct SBML schema for validation.");
	Warn(stderr, "Or try running without validation.");
    }
    Warn(stderr, "%s:%d interactive(): Can't parse Model >%s<",
	  __FILE__, __LINE__, model);
    d = loadFile(sr);
  }

  m = SBMLDocument_getModel(d);


  quit = 0;
  data = NULL;
  
  while ( quit == 0 ) {

    printf("\n");
    printf("Press (h) for instructions or (q) to quit.\n");
    printf("> ");
    select = get_line( stdin );
    select = util_trim(select);
    printf("\n");

    if( strcmp(select,"l") == 0 ) {
      d = loadFile(sr);
      m = SBMLDocument_getModel(d);
    }

    if(strcmp(select,"h")==0){
      printMenu();
    }
    
    if(strcmp(select,"s")==0){
      printModel(m);
    }
    
    if(strcmp(select,"c")==0){
      printSpecies(m);
    }
    
    if(strcmp(select,"r")==0){
      printReactions(m);
    }
    
    if(strcmp(select,"o")==0){      
      data = constructODEs(m, Opt.Jacobian);
      SolverError_dumpAndClearErrors();
      if (data)
          printODEs(data);
    }

    if(strcmp(select,"i")==0){
      data = callIntegrator(m);
     }
    
    if(strcmp(select,"x")==0){
      if ( Opt.Xmgrace == 1 ) {
	Opt.Xmgrace = 0;
	printf(" Printing results to stdout\n");
      }
      else if ( Opt.Xmgrace == 0 ) {
	Opt.Xmgrace = 1;
	printf(" Printing results to XMGrace\n");
      }
    }      
    if(strcmp(select,"st")==0){
      printConcentrationTimeCourse(data, stdout);
    }
    
    if(strcmp(select,"jt")==0){
      printJacobianTimeCourse(data, stdout);
    }
    
    if(strcmp(select,"rt")==0){
      printOdeTimeCourse(data, stdout);
    }

    if(strcmp(select,"dt")==0){
      if ( data != NULL && data->model->jacob != NULL ) {
	det = determinantNAST(data->model->jacob, data->model->neq);
	printDeterminantTimeCourse(data, det, stdout);
	ASTNode_free(det);
      }
      else {
	printf("Please integrate first!\n");
      }
    }
	

    if(strcmp(select,"xp")==0){
      printPhase(data);
    }
    
    if(strcmp(select,"set")==0){
      setValues(m);
    }
    
    if(strcmp(select,"ss")==0){
      if ( Opt.SteadyState == 1 ) {
	Opt.SteadyState = 0;
	printf(" Not checking for steady states during integration.\n");
      }
      else if ( Opt.SteadyState == 0 ) {
	Opt.SteadyState = 1;
	printf(" Checking for steady states during integration.\n");
      }
    }
    if(strcmp(select,"uj")==0){
      if ( Opt.Jacobian == 1 ) {
	Opt.Jacobian = 0;
	printf(" Using CVODE's internal approximation\n");
	printf(" of the jacobian matrix for integration\n");
      }
      else if ( Opt.Jacobian == 0 ) {
	Opt.Jacobian = 1;
	printf(" Using automatically generated\n");
	printf(" jacobian matrix for integration\n");
      }
    }    
    if(strcmp(select,"gf")==0){
      setFormat();
    }
    if(strcmp(select,"rg")==0){
      drawModel(m);
    }
    if(strcmp(select,"jg")==0){
      if ( data == NULL ) {
	Opt.Jacobian = 1;
	data = constructODEs(m, Opt.Jacobian);
    SolverError_dumpAndClearErrors();
    
	Opt.Jacobian = 0;	
      }
      if (data)
        drawJacoby(data);
    }
    if(strcmp(select,"j")==0){
      data = constructODEs(m, Opt.Jacobian);
      SolverError_dumpAndClearErrors();
      if (data)
          printJacobian(data);
    }
    if(strcmp(select,"q")==0){
      quit = 1;
    }
  }

  if ( data != NULL ) {
    CvodeData_free(data);
  }
  SBMLDocument_free(d);
  xfree(sr);
  printf("\n\nGood Bye. Thx for using.\n\n");
}


/**
  setValues: the user can enter a species name and
  change its initial condition (amount or concentration)
*/

static void
setValues(Model_t *m) {

  char *species;
  char *newIA;
  char *newIC;
  Species_t *s;

  printf("Please enter the id of the species to change: ");
  species = get_line(stdin);
  species = util_trim(species);
  
  if ( (s = Model_getSpeciesById(m,species) ) ) {      
    printf("\n");
    printf("Id:                    %s\n", Species_getId(s));
    if ( Species_isSetName(s) ) {
      printf("Name:                  %s\n", Species_getName(s));
    }
    if ( Species_isSetInitialAmount(s) ) {
      printf("Initial Amount:        %g", Species_getInitialAmount(s));
    }
    else if (Species_isSetInitialConcentration(s) ) {
      printf("Initial Concentration: %g", Species_getInitialConcentration(s));
    }
   

    if ( Species_getHasOnlySubstanceUnits(s) ) {
      if ( Species_isSetSubstanceUnits(s) ) {
	printf("%s ", Species_getSubstanceUnits(s));
      }
    } else {
      if ( Species_isSetSubstanceUnits(s) ) {
	printf("%s ", Species_getSubstanceUnits(s));
      }
      if ( Species_isSetSpatialSizeUnits(s) ) {
	printf("%s%s", "/", Species_getSpatialSizeUnits(s));
      }
    }
    if ( Species_getHasOnlySubstanceUnits(s) ) {

    }
    printf("\n");
    if ( Species_isSetCharge(s) ) {
      printf("Charge: %-10d", Species_getCharge(s));
    }
    printf("\n");   
    printf("%s       ", Species_getBoundaryCondition(s) ?
	   "Species is a Boundary\n" : "\n");
    printf("%s       ", Species_getConstant(s) ?
	   "Species is set constant" : "\n");
    printf("\n");
   
    if ( Species_isSetInitialAmount(s) ) {
      printf("Please enter new initial Amount: ");
      newIA = get_line(stdin);
      newIA = util_trim(newIA);
      Species_setInitialAmount(s, (float) atof(newIA));
    }
    else if ( Species_isSetInitialConcentration(s) ) {
      printf("Please enter new initial Amount: ");
      newIC = get_line(stdin);
      newIC = util_trim(newIC);
      Species_setInitialConcentration(s, (float) atof(newIC));
    }
  }
  else {
    printf("%s not found.\n", species);
  }
  
}

/*
  Set a new output format for graph drawing with graphviz.
*/

static void
setFormat(void) {

  char *format;
  
  while(1){
    printf("Please enter a new output format for graph drawing, \n");
    printf("or press enter to keep current setting (%s): ", Opt.GvFormat);
    format = get_line(stdin);
    format = util_trim(format);
    if ( (strlen(format)) == 0 ) {
      free(format);
      return;
    }
    else {
      sprintf(Opt.GvFormat, "%s", format);
      free(format);
      return;
    }
  }
}


static SBMLDocument_t * 
loadFile(SBMLReader_t *sr){

    char *filename;
    SBMLDocument_t *d;

    while(1){
        printf("Please enter a filename: ");
        filename = get_line(stdin);
        filename = util_trim(filename);
        if ( (strlen(filename)) == 0 ) {
            printf("No filename found.\n\n");
        }
        else {
            if ( (d = parseModel(filename)) == 0 ) {
                if ( Opt.Validate ) 
                    SolverError_error(
                        WARNING_ERROR_TYPE,
                        SOLVER_ERROR_MAKE_SURE_SCHEMA_IS_ON_PATH,
                        "Please make sure that path >%s< contains"
                        "the correct SBML schema for validation."
                        "Or try running without validation.", Opt.SchemaPath);

                SolverError_error(
                    ERROR_ERROR_TYPE,
                    SOLVER_ERROR_CANNOT_PARSE_MODEL,
                    "Can't parse Model >%s<", filename);
                SolverError_dumpAndClearErrors();
            }
            else {
                printf("SBML file %s successfully loaded.\n", filename);
                return d;
            }
        }
        free(filename);
    }
    return NULL;
}

static cvodeData_t *
callIntegrator(Model_t *m){

  char *tout;
  char *nout;

  cvodeData_t * data = NULL;

  data = constructODEs(m, Opt.Jacobian);
  
  /* chnage of behaviour by AMF no intergation if errors in ODE construction - 23rd June 2005 */
  if (!data)
  {
      SolverError_dumpAndClearErrors();
      return NULL;
  }
     
  printf("Please enter end time in seconds:           ");
  tout =  get_line(stdin);
  tout = util_trim(tout);
    
  printf("... and the number of output times:         ");
  nout = get_line(stdin);
  nout = util_trim(nout);

  
  if ( !(data->nout = (float) floor(atof(nout))) ||
       !(data->tout = (float) atof(tout)) ) {
    printf("\nEntered outtime %s or number of output times %s\n", tout, nout);
    printf("could not be converted to a number. Try again, please!\n");    
  }
  else {

    data->tmult = data->tout / data->nout;
    data->currenttime = 0.0;
    data->t0 = 0.0;

    data->opt->Error = Opt.Error;
    data->opt->RError = Opt.RError;
    data->opt->Mxstep = Opt.Mxstep;
    data->opt->HaltOnEvent = Opt.HaltOnEvent;
    data->opt->SteadyState = Opt.SteadyState;
    data->opt->EnableVariableChanges = 0;

    /* allow setting of Jacobian, only if construction was succesfull */
    if ( data->opt->UseJacobian == 1 ) {
      data->opt->UseJacobian = Opt.Jacobian;
    }
    
    printf("Numerical integration from\n t0 = %f  to \n tout"
	   " = %f s\n output interval: %f s\n\n",
	   data->t0, data->tout, data->tout/data->nout);
    
    integrator(data, Opt.PrintMessage, Opt.PrintOnTheFly, stdout);
    SolverError_dumpAndClearErrors();
  }
    
  free(nout);
  free(tout);
 
  return data;
}


static void
printMenu(void)
{  
  printf("\n");
  printf("(l)    Load Model\n");
  printf("VIEW MODEL CONTENTS\n");
  printf("(s)    View Model statistics\n");
  printf("(c)    View Initial Conditions (compartments and species)\n");
  printf("(r)    View Reactions, Parameters and Rules\n");
  printf("(o)    View ODEs\n");
  printf("(j)    View jacobian matrix\n");
  printf("SET OPTIONS AND VALUES\n");
  printf("(ss)   Toggle provisional steady state checking routine ");
  printf("(currently %s)\n", Opt.SteadyState ? "on" : "off");
  printf("(uj)   Toggle use of jacobian matrix / approximation ");
  printf("(currently %s)\n", Opt.Jacobian ? "jacobian" : "approximation");
  printf("(set)  Set initial values of species\n");  
  printf("\nINTEGRATE\n");
  printf("(i)    Integrate model\n\n");

/*   printf("       (Replaces constant parameters in all formulas!\n"); */
/*   printf("        Reload file to view original model afterwards)\n"); */
  printf("PRINT RESULTS\n");
  printf("(x)    Toggle printing to XMGrace or stdout, ");
  printf("currently %s\n", Opt.Xmgrace ? "XMGrace" : "stdout");
  printf("(st)   Print time course of species concentrations\n");
  printf("(rt)   Print time course of the ODEs\n");
  printf("(jt)   Print time course of the jacobian\n");
  printf("(dt)   Print time course of the determinant of the jacobian\n");
 
  printf("(xp)   Open XMGrace and print a phase diagram\n");  

  printf("DRAW GRAPHS\n");
  printf("(gf)   Set output format for graph drawing (now: %s)\n",
	 Opt.GvFormat);
  printf("(rg)   Draw a bipartite graph of the Reaction Network\n");
  printf("(jg)   Draw a interaction graph from the jacobian matrix\n");
  printf("       for the last time integrated\n");
/*   printf("()    Write model in SBML Level 2 Version 1\n"); */
  printf("\n");  
  printf("(h)    Print this Menu\n");
  printf("(q)    Quit Program\n");
  printf("\n\n");
  printf("Type any of the above numbers to start the process.\n");
  printf("\n");  
}


/* End of file */
