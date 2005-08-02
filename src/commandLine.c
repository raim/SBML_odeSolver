/*
  Last changed Time-stamp: <2005-08-02 17:33:23 raim>
  $Id: commandLine.c,v 1.1 2005/08/02 15:47:59 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/options.h"
#include "sbmlsolver/odeSolver.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/commandLine.h"

/** odeSolver(): Starting the SBML_odeSolver from command-line !!
    Command-line options are interpreted
    mainly from here and the according
    procedures are started.
    See file sbml.c for parsing and validation of
    SBML (Step C.1 of the documentation),
    see file odeConstruct.c for construction of 
    an ODE model and initialisation of CVODE data
    (Steps I.1, C.2-C.5 of the documentation),
    see file odeIntegrate.c for the integration procedure
    using CVODE.
    The file printModel.c contains all printing functions,
    from model structures to results, including printing
    to XMGrace, (Step X in the documentation).
    The file drawModel.c contains the graph drawing
    functions, using graphviz layout algorithms.   
*/

SBML_ODESOLVER_API int
odeSolver (int argc, char *argv[])
{
  SBMLDocument_t *d  = NULL;
  Model_t        *m  = NULL;
  char *filename;
  FILE *outfile;
  ASTNode_t *det;
  cvodeData_t *data;
  cvodeSettings_t *set;
  clock_t startTime, endTime ;

  /* read command-line arguments */
  decodeCML(argc, argv);
  
  /** -i: Enter Interactive Mode
      See files interactive.c.
      If stdin && stdout are bound to a terminal
      then option '-i' will redirect you to the
      interactive mode
  */
  if ( Opt.InterActive ) {
    interactive(); /* try it, it's fun :) */
  }   
  else {
    /* --model, --mpath: Read filename and path
      Read model path and model name from command-line
      options '--model' and '--mpath'.
      The option '--model' is not necessary,
      an argument without option tag will be interpreted
      as the file name (Opt.ModelFile), where relative paths
      to the directory from which the program is called
      are ok, while absolute paths must be set by the option
      '--mpath'.
    */
    char* model;
    model = concat(Opt.ModelPath, Opt.ModelFile);

    /** Parse the Model from File, -v: validation
	See file sbml.c.
	The SBMLDocument will be read in, validate
	if requested by command-line option '-v' and
	converted to level 2. All further routines are
	written for SBML level 2!
	Then the model will be retrieved from the document
	and used for further processing.	
    */
    if ( (d = parseModel(model)) == 0 ) {
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
            "Can't parse Model >%s<", model);
        SolverError_dump();
        SolverError_haltOnErrors();
    }
    else {
      m = SBMLDocument_getModel(d);
    }
    
    /** -g: Draw a Graph of the Reaction Network
	See file drawModel.c.
	On option -g the program will call the graph drawing
	function to draw a graph of the reaction network
	and then leave the program. Thus option '-g' overrules
	any of the following actions and many other
	command-line options.
    */
    if ( Opt.DrawReactions == 1 ) {
      if ( drawModel(m) > 0 ) {
	xfree(model);
	SBMLDocument_free(d);
	fatal(stderr, "%s:%d main(): Couldn't calculate graph for >%s<",
	      __FILE__, __LINE__, model);
      }
      xfree(model); 
      SBMLDocument_free(d);
      return(EXIT_SUCCESS);  
    }

    /* setting the file to write output data */
    if ( Opt.Write && !Opt.Xmgrace ) {
      filename = (char *) calloc(strlen(Opt.ModelPath)+
				 strlen(Opt.ModelFile)+5, sizeof(char));
      sprintf(filename, "%s%s.dat", Opt.ModelPath, Opt.ModelFile);
      outfile = fopen(filename, "w");
    }
    else {
      filename = "";
      outfile = stdout;
    }
    /** -d + -e, -de: Print Determinant of the Jacobian Matrix
	See file odeConstruct.c.
	First the differential equations will be constructed
	from the SBML model, this function will also construct
	the symbolic expressions of the jacobian matrix, and
	upon a combination of command-line options '-e' and
	'-d', the determinant of the jacobian will be calculated
	and printed out. The program is left afterwards, again
	overruling the following actions and many command-line
	options. 
    */
    if ( Opt.Determinant == 1 && Opt.PrintModel == 1 ) {
      data = constructODEs(m, Opt.Jacobian);
      det = determinantNAST(data->model->jacob, data->model->neq)
;
      /* slight change in behaviour any errors cause halt - AMF 23rd June 05
         used to continue with empty model */
      SolverError_haltOnErrors();
      fprintf(outfile, "det(J) = %s\n", SBML_formulaToString(det));
      CvodeData_free(data);   
      xfree(model);
      SBMLDocument_free(d);
      ASTNode_free(det);
      return(EXIT_SUCCESS);    
    }

    /** -e: Print All Model Contents and ODEs
	See files printModel.c.
	With command-line option '-e', the program will
	just print out everything: model contents, and the
	derived ODE system; the jacobian matrix expressions
	will NOT be printed ONLY when additionally option '-j' was
	set. Exit afterwards.
    */
    if ( Opt.PrintModel == 1 ) {
      printModel(m);
      printSpecies(m);
      printReactions(m);

      data = constructODEs(m, Opt.Jacobian);
      /* slight change in behavour - halt now rather than continue
	 with null model
         used to continue with empty model */
      SolverError_haltOnErrors();
      printODEs(data);
      printJacobian(data);

      CvodeData_free(data);   
      xfree(model);
      SBMLDocument_free(d);       
      return(EXIT_SUCCESS);      
    }
    
    /** -o: Print ODE Model 
	See files odeConstruct.c and printModel.c.
	When option '-o' is given, the model will be simplified
	just like done for integration with CVODE. This
	simplified model will be printed to the given outfile or
	to stdout. Exit afterwards.
    */
    if ( Opt.PrintODEsToSBML == 1 ) {

      data = constructODEs(m, Opt.Jacobian);
      /* slight change in behavour - halt now rather than
	 continue with null model
         used to continue with empty model */
      SolverError_haltOnErrors();
      printODEsToSBML(data);

      CvodeData_free(data);   
      xfree(model);
      SBMLDocument_free(d);       
      
      return(EXIT_SUCCESS);      
    }

    
    /** Default: Start Integration Procedure
	See files odeConstruct.c,
	odeIntegrate.c and printModel.c.
    */

    
    /** At first, the function constructODEs(m)
	will attempt to construct a simplified SBML model,
	that only consists of species and their ODEs, represented
	as Rate Rules, and of Events. All constant species, parameters
	compartments, assignment rules and function definitions
	will be replaced by their values or expressions respectively
	in all remaining formulas (ie. rate and algebraic rules and
	events).
	Then the initial values and ODEs of the remaining species
	will be written to the structure cvodeData_t *data.
    */
    data = constructODEs(m, Opt.Jacobian);
    /** Errors will cause the program to stop,
	e.g. when some mathematical expressions are missing.
    */
    SolverError_haltOnErrors();
    

    /** Set integration parameters:
	Now that we have arrived here, we can set the parameters
	for integration, like
	the end time and the number of steps to be printed out,
	absolute and relative error tolerances,
	use of exact or approximated Jacobian matrix,
	printing of messages during integration,
	event handling, steady state detection, and
	runtime printing of results.
	And then ..
    */

    data->tout  = Opt.Time;
    data->nout  = Opt.PrintStep;
    data->tmult = data->tout / data->nout;
    data->currenttime = 0.0;
    data->t0 = 0.0;

    set = (cvodeSettings_t *)calloc(1, sizeof(cvodeSettings_t));

    set->StoreResults = 1;
    set->Error = Opt.Error;
    set->RError = Opt.RError;
    set->Mxstep = Opt.Mxstep;
    set->HaltOnEvent = Opt.HaltOnEvent;
    set->SteadyState = Opt.SteadyState;
    set->EnableVariableChanges = 0;

    data->opt = set;

    
    /* allow setting of Jacobian,
       only if its construction was succesfull */
    set->UseJacobian = data->model->jacobian && set->UseJacobian;


    /** .... we can call the integrator function,
	that invokeds CVODE and stores results.
	The function will also handle events and
	check for steady states.
    */    
    
    startTime = clock();

    integrator(data, Opt.PrintMessage, Opt.PrintOnTheFly, outfile);
    SolverError_dump();
    RETURN_ON_FATALS_WITH(EXIT_FAILURE);
    SolverError_clear();

    endTime = clock();

    printf("#execution time %f\n",
	   ((double)(endTime-startTime))/ CLOCKS_PER_SEC);

    /** Finally, print out the results
	in the format specified by commanline option,
    */

    /** -f, --onthefly: print results during integration
	no other printing needed
	-a, --all: print all results
    */
    if ( !Opt.PrintOnTheFly && !Opt.PrintAll ) {
      /** -y: print time course of the jacobian matrix
	  expressions
      */	  
      if ( Opt.PrintJacobian == 1 ) {
	printJacobianTimeCourse(data, outfile);
      }

      /** -k: print time course of the reactions, i.e.
	  kinetic law expressions
      */      
      else if ( Opt.PrintReactions == 1 ) {
	printReactionTimeCourse(data, outfile);
      }

      /** -r: print time coures of ODE values
      */
      else if ( Opt.PrintRates == 1 ) {
	printOdeTimeCourse(data, outfile);
      }
      
      /** -d: print time course of the determinant
	  of the jacobian matrix
      */
      else if ( Opt.Determinant == 1 ) {
	det = determinantNAST(data->model->jacob, data->model->neq);
	printDeterminantTimeCourse(data, det, outfile);
	ASTNode_free(det);
      }
      
      /** Default (no printing options):
	  print species concentrations
       */
      else {
	printConcentrationTimeCourse(data, outfile);
      }      
    }
    /** -a, --all: print all results
    */
    else if ( !Opt.PrintOnTheFly && Opt.PrintAll ) {
      printJacobianTimeCourse(data, outfile);
      printReactionTimeCourse(data, outfile);
      printOdeTimeCourse(data, outfile);
      printConcentrationTimeCourse(data, outfile);
    }

    /** -m: Draw the jacobian interaction graph
	with values from the last integration point (tout)
	defining edge color and arrowheads (red, ie. inhibitory
	for negative values, black, activating for positive
	values.
    */
    if ( Opt.DrawJacobian == 1 ) {
      if ( drawJacoby(data) > 0 ) {
	Warn(stderr,
	     "%s:%d main(): Couldn't calculate jacobian graph for >%s<",
	      __FILE__, __LINE__, model);
      }
    }

    
    /* thx and good bye. */
    /* save and close results file */
    if ( Opt.Write && !Opt.Xmgrace ) {
      fclose(outfile);
      fprintf(stderr, "Saved results to file %s.\n\n", filename);
      free(filename);
    }
    CvodeData_free(data); 
    xfree(model);   
    SBMLDocument_free(d);
  }

  return(EXIT_SUCCESS);
}

SBMLDocument_t*
parseModel (char *file)
{
    return parseModelPassingOptions(
        file, Opt.PrintMessage, Opt.Validate, Opt.SchemaPath,
        Opt.Schema11, Opt.Schema12, Opt.Schema21);
}



/* End of file */
