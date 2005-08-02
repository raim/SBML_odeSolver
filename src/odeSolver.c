/*
  Last changed Time-stamp: <2005-08-02 02:30:44 raim>
  $Id: odeSolver.c,v 1.11 2005/08/02 13:20:28 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/odeSolver.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/solverError.h"

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

SBMLResults_t *
Model_odeSolver(SBMLDocument_t *d, cvodeSettings_t *set) {
  
  SBMLDocument_t *d2 = NULL;
  cvodeData_t *data;
  SBMLResults_t *results;
  Model_t *m;

 
  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    m = SBMLDocument_getModel(d2);
  }
  else {
    m = SBMLDocument_getModel(d);
  }
  
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
  
  data = constructODEs(m, set->UseJacobian);
  /** Errors, found during odeConstruct, will cause the program to exit,
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
  data->tout  = set->Time;
  data->nout  = set->PrintStep;
  data->tmult = set->Time / set->PrintStep;
  data->currenttime = 0.0;
  data->t0 = 0.0;
  data->opt = set;

  /* allow setting of Jacobian,
     only if its construction was succesfull */
   if ( data->opt->UseJacobian == 1 ) { 
     data->opt->UseJacobian = set->UseJacobian && data->model->jacobian; 
   } 
  
  /** .... we can call the integrator function,
      that invokeds CVODE and stores results.
      The function will also handle events and
      check for steady states.
  */    

  integrator(data, 0, 0, stdout);
  SolverError_dump();
  RETURN_ON_FATALS_WITH(NULL);
  SolverError_clear();

  /* Write simulation results into result structure */
  results = Results_fromCvode(data); 
  CvodeData_free(data);
  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }
  return(results);
}

SBMLResults_t **
Model_odeSolverBatch (SBMLDocument_t *d,
		      cvodeSettings_t *settings, VarySettings vary) {

  int i;
  double value, increment;
  SBMLResults_t **results;
  SBMLDocument_t *d2 = NULL;
  Model_t *m;

  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    d = d2;
  }
  m = SBMLDocument_getModel(d);
    
  if(!(results = (SBMLResults_t **)calloc(vary.steps+1, sizeof(*results)))){
    fprintf(stderr, "failed!\n");
  }

  value = vary.start;
  increment = (vary.end - vary.start) / vary.steps;
  
  for ( i=0; i<=vary.steps; i++ ) {
    
      
    if ( ! Model_setValue(m, vary.id, vary.rid, value) ) {
      Warn(stderr, "Parameter for variation not found in the model.", vary.id);
      return(NULL);
    }
    results[i] = Model_odeSolver(d, settings);
    value = value + increment;
  }

  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }

  return(results);

}

SBMLResults_t ***
Model_odeSolverBatch2 (SBMLDocument_t *d, cvodeSettings_t *settings,
		      VarySettings vary1, VarySettings vary2) {

  int i, j;
  double value1, increment1, value2, increment2;
  SBMLResults_t ***results;
  SBMLDocument_t *d2 = NULL;
  Model_t *m;

  /** Convert SBML Document level 1 to level 2, and
      get the contained model
  */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    d = d2;
  }
  m = SBMLDocument_getModel(d);
    
  if(!(results = (SBMLResults_t ***)calloc(vary1.steps+1, sizeof(**results)))){
    fprintf(stderr, "failed!\n");
  }
  for ( i=0; i<=vary1.steps; i++ ) {
    if(!(results[i] = (SBMLResults_t **)calloc(vary2.steps+1, sizeof(*results)))){
      fprintf(stderr, "failed!\n");
    }    
  }

  value1 = vary1.start;
  increment1 = (vary1.end - vary1.start) / vary1.steps;
  
  value2 = vary2.start;
  increment2 = (vary2.end - vary2.start) / vary2.steps;
  
  for ( i=0; i<=vary1.steps; i++ ) {
    if ( ! Model_setValue(m, vary1.id, vary1.rid, value1) ) {
      Warn(stderr, "Parameter for variation not found in the model.",
	   vary1.id);
      return(NULL);
    }
    value2 = vary2.start; 
    for ( j=0; j<=vary2.steps; j++ ) {      
      if ( ! Model_setValue(m, vary2.id, vary2.rid, value2) ) {
	Warn(stderr, "Parameter for variation not found in the model.",
	     vary2.id);
      return(NULL);
      }
      
      results[i][j] = Model_odeSolver(d, settings);
      value2 = value2 + increment2;
    }
    value1 = value1 + increment1;
  }

  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }

  return(results);

}

int
Model_setValue(Model_t *m, const char *id, const char *rid, double value) {

  int i;
  Compartment_t *c;
  Species_t *s;
  Parameter_t *p;
  Reaction_t *r;
  KineticLaw_t *kl;

  if ( (r = Model_getReactionById(m, rid)) != NULL ) {
    kl = Reaction_getKineticLaw(r);
    for ( i=0; i<KineticLaw_getNumParameters(kl); i++ ) {
      p = KineticLaw_getParameter(kl, i);
      if ( strcmp(id, Parameter_getId(p)) == 0 ) {
	Parameter_setValue(p, value);
	return 1;
      }
    }
  }
  if ( (c = Model_getCompartmentById(m, id)) != NULL ) {
    Compartment_setSize(c, value);
    return 1;
  }
  if ( (s = Model_getSpeciesById(m, id)) != NULL ) {
    if ( Species_isSetInitialAmount(s) ) {
      Species_setInitialAmount(s, value);
    }
    else {
      Species_setInitialConcentration(s, value);
    }
    return 1;
  }
  if ( (p = Model_getParameterById(m, id)) != NULL ) {
    Parameter_setValue(p, value);
    return 1;
  }
  return 0;  
}

/* The function Results_fromCvode(cvodeData_t *data)
   maps the integration results of CVODE
   back to SBML structures.
*/

SBMLResults_t *
Results_fromCvode(cvodeData_t *data) {

  int i, j, k;
  SBMLResults_t *sbml_results;
  Model_t *m;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;
  
  cvodeResults_t *cvode_results;
  timeCourse_t *tc;
  
  if ( data == NULL ) {
    fatal(stderr, "No data, please construct ODE system first.\n");
    return NULL;
  }
  else if ( data->results == NULL ) {    
    fatal(stderr, "No results, please integrate first.\n");
    return NULL;
  }

  sbml_results = SBMLResults_create(data->model->m, data->results->nout+1);    
  cvode_results = data->results;
  m = data->model->m;

  /* Allocating temporary kinetic law ASTs, for evaluation of fluxes */

  if(!(kls =
       (ASTNode_t **)calloc(Model_getNumReactions(m),
			    sizeof(ASTNode_t *)))) {
    fprintf(stderr, "failed!\n");
  }  
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    r = Model_getReaction(m, i);
    kl = Reaction_getKineticLaw(r);
    kls[i] = copyAST(KineticLaw_getMath(kl));
    AST_replaceNameByParameters(kls[i], KineticLaw_getListOfParameters(kl));
    AST_replaceConstants(m, kls[i]);
  }
  
  
  /*
    Filling results for each calculated timepoint.
  */
  for ( i=0; i<sbml_results->timepoints; i++ ) {
    
    /* writing time steps */
    sbml_results->time[i] = cvode_results->time[i];
    /* updating time and values in cvodeData_t *for calculations */
    data->currenttime = cvode_results->time[i]; 
    for ( j=0; j<data->nvalues; j++ ) {
      data->value[j] = cvode_results->value[j][i]; 
    }
    /* filling time courses for SBML species  */
    tc = sbml_results->species;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in cvodeData_t for values */
       for ( k=0; k<data->nvalues; k++ ) {
	if ( (strcmp(tc->names[j], data->model->names[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];	
	}
      }      
    }
    
    /* filling variable compartment time courses */
    tc = sbml_results->compartments;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in cvodeData_t for values */
      for ( k=0; k<data->nvalues; k++ ) {
	if ( (strcmp(tc->names[j], data->model->names[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	}
      }  
    }         

    /* filling variable parameter time courses */
    tc = sbml_results->parameters;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in cvodeData_t for values */
      for ( k=0; k<data->nvalues; k++ ) {
	if ( (strcmp(tc->names[j], data->model->names[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	}
      }        
    }

    /* filling reaction flux time courses */
    tc = sbml_results->fluxes;
    for ( j=0; j<tc->num_val; j++ ) {
      tc->values[i][j] = evaluateAST(kls[j], data);
    }

  }

  /* freeing temporary kinetic law ASTs */
  for ( i=0; i<Model_getNumReactions(m); i++ ) {
    ASTNode_free(kls[i]);
  }  
  free(kls);
  
  return(sbml_results);
}



/* End of file */
