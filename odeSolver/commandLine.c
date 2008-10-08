/*
  Last changed Time-stamp: <2008-10-08 15:36:42 raim>
  $Id: commandLine.c,v 1.27 2008/10/08 17:07:16 raimc Exp $
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
 *     Christoph Flamm
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include <sbml/SBMLTypes.h>

#include "../src/sbmlsolver/odeSolver.h"

#include "interactive.h"
#include "printModel.h"
#include "options.h"
#include "commandLine.h"


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

int
odeSolver (int argc, char *argv[])
{
  SBMLDocument_t *d   = NULL;
  Model_t        *m   = NULL;
  Model_t        *ode = NULL;
  char *filename;
  FILE *outfile;
  ASTNode_t *det;
  odeModel_t *om;
  integratorInstance_t *ii;
  cvodeSettings_t *set;
  clock_t startTime, endTime ;

  startTime = clock();

  /* read command-line arguments */
  decodeCML(argc, argv);
  
  /** -i: Enter Interactive Mode
      See files interactive.c.
      If stdin && stdout are bound to a terminal
      then option '-i' will redirect you to the
      interactive mode
  */
  if ( Opt.InterActive )
  {
    interactive(); /* try it, it's fun :) */
  }   
  else
  {
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
    char* sbmlFilename;

    if (strlen(Opt.ModelPath) == 0)
    {
        ASSIGN_NEW_MEMORY_BLOCK(sbmlFilename,
				strlen(Opt.ModelFile) + 1, char, EXIT_FAILURE);
        strcpy(sbmlFilename, Opt.ModelFile);
    }
    else
        sbmlFilename = concat(Opt.ModelPath, Opt.ModelFile);

    /** Parse the Model from File, -v: validation
	See file sbml.c.
	The SBMLDocument will be read in, validate
	if requested by command-line option '-v' and
	converted to level 2. All further routines are
	written for SBML level 2!
	Then the model will be retrieved from the document
	and used for further processing.	
    */
    if ( (d = parseModelWithArguments(sbmlFilename)) == 0 )
    {
        SolverError_error(ERROR_ERROR_TYPE, SOLVER_ERROR_CANNOT_PARSE_MODEL,
			  "Can't parse Model >%s<", sbmlFilename);
        SolverError_dump();
        SolverError_haltOnErrors();
    }
    else
    {
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
    if ( Opt.DrawReactions == 1 )
    {
      if ( Opt.PrintMessage ) 
	fprintf(stderr,
		"\n\nTrying to draw reaction graph '%s_rn.%s' from the model. \nThis can take a while for big models... \n\n",
		sbmlFilename, Opt.GvFormat);
  
      if ( !drawModel(m, sbmlFilename, Opt.GvFormat) > 0 )
      {
	xfree(sbmlFilename);
	SBMLDocument_free(d);
	fatal(stderr, "%s:%d odeSolver(): Couldn't calculate graph for >%s<",
	      __FILE__, __LINE__, sbmlFilename);
      }
      xfree(sbmlFilename); 
      SBMLDocument_free(d);
      return(EXIT_SUCCESS);  
    }

    /* setting the file to write output data */
    if ( Opt.Write && !Opt.Xmgrace )
    {
      filename = (char *) calloc(strlen(Opt.ModelPath)+
				 strlen(Opt.ModelFile)+5, sizeof(char));
      sprintf(filename, "%s%s.dat", Opt.ModelPath, Opt.ModelFile);
      outfile = fopen(filename, "w");
    }
    else
    {
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
    if ( Opt.Determinant == 1 && Opt.PrintModel == 1 )
    {
      om = ODEModel_create(m);
      ODEModel_constructJacobian(om);
      det = ODEModel_constructDeterminant(om);
      /* slight change in behaviour any errors cause halt - AMF 23rd June 05
         used to continue with empty model */
      /* SolverError_haltOnErrors(); */
      fprintf(outfile, "det(J) = %s\n", SBML_formulaToString(det));
      ODEModel_free(om);
      Model_free(ode);
      xfree(sbmlFilename);
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
    if ( Opt.PrintModel == 1 )
    {
      printModel(m, outfile);
      printSpecies(m, outfile);
      printReactions(m, outfile);
      
      om = ODEModel_create(m);
      /* slight change in behavour - halt now rather than continue
	 with null model
         used to continue with empty model */
      SolverError_dump(); /* write out all everything including warnings */
      SolverError_haltOnErrors(); 
      printODEs(om, outfile);
      if ( Opt.Jacobian )
	ODEModel_constructJacobian(om);
      SolverError_dump(); /* write out all everything including warnings */
      SolverError_haltOnErrors(); 
      printJacobian(om, outfile);

      ODEModel_free(om);
      Model_free(ode);
      xfree(sbmlFilename);
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
    if ( Opt.PrintODEsToSBML == 1 )
    {
      ode = Model_reduceToOdes(m);
      /* slight change in behavour - halt now rather than
	 continue with null model
         used to continue with empty model */
      SolverError_dump(); /* write out all everything including warnings */
      SolverError_haltOnErrors();
      printODEsToSBML(ode, outfile);

      Model_free(ode);   
      xfree(sbmlFilename);
      SBMLDocument_free(d);       
      
      return(EXIT_SUCCESS);      
    }

    /**************** integration routines start below ****************/
    
    /** Default: Start Integration Procedure
	See files odeConstruct.c,
	odeIntegrate.c and printModel.c.
    */

    
    /** At first, the function Model_reduceToOdes(m)
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
    if ( Opt.PrintMessage ) 
      fprintf(stderr, "\nGenerating odeModel ...\n");
  
    om = ODEModel_create(m);
   
    SolverError_dump();
    SolverError_haltOnErrors();
    SolverError_clear();


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

    set = CvodeSettings_createWith(Opt.Time, Opt.PrintStep,
				   Opt.Error, Opt.RError,
				   Opt.Mxstep, Opt.Method, Opt.IterMethod,
				   Opt.Jacobian, 0, Opt.HaltOnEvent,
				   Opt.SteadyState, 1, Opt.Sensitivity, 2);
    CvodeSettings_setCompileFunctions(set, Opt.Compile);
    CvodeSettings_setSteadyStateThreshold(set, Opt.ssThreshold);
    CvodeSettings_setResetCvodeOnEvent(set, Opt.ResetCvodeOnEvents);
    CvodeSettings_setDetectNegState(set, Opt.DetectNegState);
    
    /* ... we can create an integratorInstance */
    ii = IntegratorInstance_create(om, set);
   
    SolverError_dump();
    SolverError_haltOnErrors();

    /** .... we can call the integrator functions,
	that invoke CVODE and stores results.
	The function will also handle events and
	check for steady states.
    */    
    
    integrator(ii, Opt.PrintMessage, Opt.PrintOnTheFly, outfile);
    
    SolverError_dump();
    RETURN_ON_FATALS_WITH(EXIT_FAILURE);

    /** Finally, print out the results
	in the format specified by commanline option,
    */

    /** -f, --onthefly: print results during integration
	no other printing needed
	-a, --all: print all results
    */
    if ( !Opt.PrintOnTheFly && !Opt.PrintAll )
    {
      /** -y: print time course of the jacobian matrix
	  expressions
      */	  
      if ( Opt.PrintJacobian == 1 )
      {
	printJacobianTimeCourse(ii->data, outfile);
      }

      /** -k: print time course of the reactions, i.e.
	  kinetic law expressions
      */      
      else if ( Opt.PrintReactions == 1 )
      {
	printReactionTimeCourse(ii->data, m, outfile);
      }

      /** -r: print time coures of ODE values
      */
      else if ( Opt.PrintRates == 1 )
      {
	printOdeTimeCourse(ii->data, outfile);
      }
      
      /** -d: print time course of the determinant
	  of the jacobian matrix
      */
      else if ( Opt.Determinant == 1 )
      {
	ODEModel_constructJacobian(om);
	det = determinantNAST(om->jacob, om->neq);
	printDeterminantTimeCourse(ii->data, det, outfile);
	ASTNode_free(det);
      }
      
      /** Default (no printing options):
	  print species concentrations
       */
      else
      {
	printConcentrationTimeCourse(ii->data, outfile);
      }      
    }
    /** -a, --all: print all results
    */
    else if ( !Opt.PrintOnTheFly && Opt.PrintAll )
    {
      printJacobianTimeCourse(ii->data, outfile);
      printReactionTimeCourse(ii->data, m, outfile);
      printOdeTimeCourse(ii->data, outfile);
      printConcentrationTimeCourse(ii->data, outfile);
    }

    /** -m: Draw the jacobian interaction graph
	with values from the last integration point (tout)
	defining edge color and arrowheads (red, ie. inhibitory
	for negative values, black, activating for positive
	values.
    */
    if ( Opt.DrawJacobian == 1 )
    {
      if ( Opt.PrintMessage )
        fprintf(stderr,
	  "\n\nTrying to draw a species interaction graph %s_jm.%s from\n  \
	  the jacobian matrix at the last time point of integration.\n     \
	  This can take a while for big models... \n\n",
	  sbmlFilename, Opt.GvFormat);

      if ( !drawJacoby(ii->data, sbmlFilename, Opt.GvFormat) > 0 ) {
	Warn(stderr,
	     "%s:%d main(): Couldn't calculate jacobian graph for >%s<",
	      __FILE__, __LINE__, sbmlFilename);
      }
    }

  endTime = clock();

    
  /* Print some final statistics   */
  if ( Opt.PrintMessage )
  {
    IntegratorInstance_printStatistics(ii, stdout);
  }

  if ( Opt.Benchmark )
  {
    printf("## execution time %f\n",
	   ((double)(endTime-startTime))/CLOCKS_PER_SEC);
    printf("## integrationTime %f\n",
	   IntegratorInstance_getIntegrationTime(ii));
  }

    
    /* thx and good bye. */
    /* save and close results file */
    if ( Opt.Write && !Opt.Xmgrace )
    {
      fclose(outfile);
      fprintf(stderr, "Saved results to file %s.\n\n", filename);
      free(filename);
    }

    SolverError_dump();
    SolverError_clear();
    IntegratorInstance_free(ii);
    CvodeSettings_free(set);
    ODEModel_free(om);
    
    xfree(sbmlFilename);   
    SBMLDocument_free(d);

  }

  return(EXIT_SUCCESS);
}

SBMLDocument_t*
parseModelWithArguments (char *file)
{
    return parseModel(file, Opt.PrintMessage, Opt.Validate);
}

/************************ Integrator Program *************************/
/**
  The function "static int integrator(cvodeData_t *data)" gets a filled
  structure "CvodeData" and calls CVODE via the integratorInstance group
  of functions, to integrate the ODEs.
*/

int integrator(integratorInstance_t *engine,
	       int PrintMessage, int PrintOnTheFly, FILE *outfile)
{
  int i, j;
  odeModel_t *om = engine->om;
  odeSense_t *os = engine->os;
  cvodeData_t *data = engine->data;
  cvodeSolver_t *solver = engine->solver;
  
 /** Command-line option -f/--onthefly:
      print initial values, if on-the-fly printing is set
 */
  if ( PrintOnTheFly && engine->run == 1 )
  {
    fprintf(stderr,
	    "\nPrinting concentrations or sensitivities on the fly !\n");
    fprintf(stderr, "Overruling all other print options!!\n\n");
    
    
    /* print sensitivities */
    if ( Opt.Sensitivity && data->sensitivity != NULL )
    {      
      fprintf(outfile, "##SENSITIVITIES\n");
      fprintf(outfile, "#t ");
      for ( i=0; i<om->neq; i++ ) 
	for ( j=0; j<os->nsens; j++ )
	  fprintf(outfile, "d%s/%s ",
		  om->names[i], om->names[os->index_sens[j]]);
      fprintf(outfile, "\n");
       
      for ( i=0; i<om->neq; i++ ) 
	for ( j=0; j<os->nsens; j++ )
	  fprintf(outfile, "%g ", data->sensitivity[i][j]);
      fprintf(outfile, "\n");
    }
    /* print concentrations */ 
    else
    {      
      fprintf(outfile, "##CONCENTRATIONS\n");
      fprintf(outfile, "#t ");      
      for ( i=0; i<data->nvalues; i++ )
	fprintf(outfile, "%s ", om->names[i]);

      fprintf(outfile, "\n");

      fprintf(outfile, "%g ", solver->t0);
      for ( i=0; i<data->nvalues; i++ )
	fprintf(outfile, "%g ", data->value[i]);

      fprintf(outfile, "\n");
    }
  }
  else
  {
    if ( PrintMessage )
      fprintf(stderr,"Integrating        ");
  }

  /*
    In loop over output points, call CVode, test for error
    and print results at specified intervals into results
    structure. If option '-f' or '--onthefly' was set, then
    print results immediately to the given outfile (default:
    stdout).
  */
  
  while (!IntegratorInstance_timeCourseCompleted(engine))
  {
    if (!IntegratorInstance_integrateOneStep(engine))
    {
      /* SolverError_dump(); */
      return IntegratorInstance_handleError(engine);
    }
          
    /* print immediately if PrintOnTheFly was set
       with '-d' or '--onthefly'
    */
 
    if ( PrintOnTheFly )
    {
      /* print sensitivities */
      if ( Opt.Sensitivity && data->sensitivity != NULL )
      {
	fprintf(outfile, "%g ", solver->t);
	for ( i=0; i<data->neq; i++ ) 
	  for ( j=0; j<data->nsens; j++ ) 
	    fprintf(outfile, "%g ", data->sensitivity[i][j]);
	fprintf(outfile, "\n");

      }
      /* print concentrations */
      else
      {
	/* first, update assignment rules */
	if ( !data->allRulesUpdated )
	{
	  for ( i=0; i<om->nass; i++ )
	  {
	    nonzeroElem_t *ordered = om->assignmentOrder[i];
#ifdef ARITHMETIC_TEST
	    data->value[ordered->i] = ordered->ijcode->evaluate(data);
#else
	    data->value[ordered->i] = evaluateAST(ordered->ij, data);
#endif
	  }
	  data->allRulesUpdated = 1;
	}	
	fprintf(outfile, "%g ", solver->t);
	for ( i=0; i<data->nvalues; i++ )
	  fprintf(outfile, "%g ", data->value[i]);
	fprintf(outfile, "\n");
	
      }
    }
    else if ( PrintMessage )
    {
      const  char chars[5] = "|/-\\";
      fprintf(stderr, "\b\b\b\b\b\b");
      fprintf(stderr, "%.2f %c",
	      (float)(solver->iout-1)/(float)solver->nout,
	      chars[(solver->iout-1) % 4]);
    }
  }
  
  if ( PrintOnTheFly && engine->run == 1 )
  {    
    fprintf(outfile, "#t ");
    /* print sensitivities */
    if ( Opt.Sensitivity && data->sensitivity != NULL )
    {      
      for ( i=0; i<om->neq; i++ ) 
	for ( j=0; j<os->nsens; j++ )
	  fprintf(outfile, "d%s/%s ",
		  om->names[i], om->names[os->index_sens[j]]);
      fprintf(outfile, "\n");
      fprintf(outfile, "##SENSITIVITIES\n");
    }
    /* print concentrations */
    else
    {
      for ( i=0; i<data->nvalues; i++ )
	fprintf(outfile, "%s ", om->names[i]);

      fprintf(outfile, "\n");
      fprintf(outfile, "##CONCENTRATIONS\n");
    }
  }
  else if ( PrintMessage )
    fprintf(stderr, "finished. Results stored.\n");

  return 0;

} 

/* End of file */
