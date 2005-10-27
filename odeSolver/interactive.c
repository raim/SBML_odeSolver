/*
  Last changed Time-stamp: <2005-10-27 16:35:04 raim>
  $Id: interactive.c,v 1.4 2005/10/27 14:52:51 raimc Exp $
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

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/util/util.h> /* only for util_trim */

/* own header files */
#include "../src/sbmlsolver/odeSolver.h"

#include "interactive.h"
#include "options.h"
#include "commandLine.h"
#include "printModel.h"

static void printMenu(void);
static void setValues(Model_t *);
static void setFormat(void);
static SBMLDocument_t *loadFile();
static integratorInstance_t *callIntegrator(odeModel_t *, cvodeSettings_t *);

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
  
  char *sbmlFilename;
  char *select;
  int quit;
  SBMLDocument_t *d  = NULL;
  Model_t        *m  = NULL;
  odeModel_t *om     = NULL;
  cvodeData_t * data = NULL;
  integratorInstance_t *ii = NULL;
  cvodeSettings_t *set = NULL;

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

  sbmlFilename = concat(Opt.ModelPath, Opt.ModelFile);

  if ( (d = parseModelWithArguments(sbmlFilename)) == 0 ) {
    if ( Opt.Validate ) {
	Warn(stderr, "Please make sure that path >%s< contains",
	     Opt.SchemaPath);
	Warn(stderr, "the correct SBML schema for validation.");
	Warn(stderr, "Or try running without validation.");
    }
    Warn(stderr, "%s:%d interactive(): Can't parse Model >%s<",
	  __FILE__, __LINE__, sbmlFilename);
    d = loadFile();
  }

  /* load models and default settings */
  m = SBMLDocument_getModel(d);
  om = ODEModel_create(m);
  set = CvodeSettings_create();
  SolverError_dumpAndClearErrors();
  
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
      /* free all existing structures */
      if ( om != NULL )
	ODEModel_free(data->model);
      if ( d != NULL )
	SBMLDocument_free(d);
      if ( ii != NULL )
	IntegratorInstance_free(ii);

      /* load a new file */
      d = loadFile();
      
      /* load new models */
      m = SBMLDocument_getModel(d);
      om = ODEModel_create(m);
      SolverError_dumpAndClearErrors();            
    }

    if(strcmp(select,"h")==0)
      printMenu();
    
    if(strcmp(select,"s")==0)
      printModel(m, stdout);
    
    if(strcmp(select,"c")==0)
      printSpecies(m, stdout);
    
    if(strcmp(select,"r")==0)
      printReactions(m, stdout);
    
    if(strcmp(select,"o")==0)
      printODEs(om, stdout);

    /* integrate interface functions, asks for time and printsteps */
    if(strcmp(select,"i")==0){
      ii = callIntegrator(om, set);
      SolverError_dumpAndClearErrors();
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
    if(strcmp(select,"st")==0)
      printConcentrationTimeCourse(ii->data, stdout);
    
    if(strcmp(select,"jt")==0)
      printJacobianTimeCourse(ii->data, stdout);

    
    if(strcmp(select,"ot")==0)
      printOdeTimeCourse(ii->data, stdout);

    
    if(strcmp(select,"rt")==0)
      printReactionTimeCourse(ii->data, m, stdout);
    
    if(strcmp(select,"xp")==0)
      printPhase(ii->data);
    
    
    if(strcmp(select,"set")==0)
      setValues(m);
    
    
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
    
    if(strcmp(select,"gf")==0)
      setFormat();
    
    if(strcmp(select,"rg")==0) {
      drawModel(m, sbmlFilename, Opt.GvFormat);
      SolverError_dumpAndClearErrors();
    }
    
    if(strcmp(select,"jg")==0){
      if ( ii == NULL ) {
	data = CvodeData_create(om);
	CvodeData_initialize(data, set, om);
	drawJacoby(data, sbmlFilename, Opt.GvFormat);
	SolverError_dumpAndClearErrors();
	CvodeData_free(data);
      }
      else {
	drawJacoby(ii->data, sbmlFilename, Opt.GvFormat);
	SolverError_dumpAndClearErrors();
      }
    }

    
    if(strcmp(select,"j")==0) {
      if ( om->jacob == NULL )
	ODEModel_constructJacobian(om);
      printJacobian(om, stdout);
    }

    
    if(strcmp(select,"q")==0)
      quit = 1;
    
  }

  if ( ii != NULL )
    IntegratorInstance_free(ii);
  if ( om != NULL ) 
    ODEModel_free(om);

  SBMLDocument_free(d);
  SolverError_dumpAndClearErrors();
  printf("\n\nGood Bye. Thx for using.\n\n");
}


/*
  setValues: the user can enter a species name and
  change its initial condition (amount or concentration)
*/
static void setValues(Model_t *m) {

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
  set a new output format for graph drawing with graphviz.
*/
static void setFormat(void)
{
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


/*
  force to load an SBML file
*/
static SBMLDocument_t *loadFile()
{
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
            if ( (d = parseModelWithArguments(filename)) == 0 ) {
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

static integratorInstance_t *callIntegrator(odeModel_t *om,
					    cvodeSettings_t *set)
{

  char *tout;
  char *nout;
  double time;
  int printstep;
  integratorInstance_t *ii;
  
  printf("Please enter end time in seconds:           ");
  tout =  get_line(stdin);
  tout = util_trim(tout);
    
  printf("... and the number of output times:         ");
  nout = get_line(stdin);
  nout = util_trim(nout);

  
  if ( !(time = (float) floor(atof(tout))) ||
       !(printstep = (int) atof(nout)) ) {
    printf("\nEntered outtime %s or number of output times %s\n", tout, nout);
    printf("could not be converted to a number. Try again, please!\n");    
  }
  else {
    CvodeSettings_setTime(set, time, printstep);
    CvodeSettings_dump(set);
    ii = IntegratorInstance_create(om, set);
    integrator(ii, 1, 0, stdout);
    SolverError_dumpAndClearErrors();
    return (ii);
  }
  
  return NULL;
}


static void printMenu(void)
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
  printf("(ot)   Print time course of the ODEs\n");
  printf("(rt)   Print time course of the reaction fluxes\n");
  printf("(jt)   Print time course of the jacobian\n");
 
  printf("(xp)   Open XMGrace and print a phase diagram\n");  

  printf("DRAW GRAPHS with GRAPHVIZ\n");
  printf("(gf)   Set output format for graph drawing (now: %s)\n",
	 Opt.GvFormat);
  printf("(rg)   Draw bipartite graph of the reaction network\n");
  printf("(jg)   Draw interaction graph from the jacobian matrix\n");
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
