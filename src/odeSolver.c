/*
  Last changed Time-stamp: <2005-05-31 12:25:00 raim>
  $Id: odeSolver.c,v 1.3 2005/05/31 13:54:00 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sbml/SBMLTypes.h>

#include "sbmlsolver/odeSolver.h"



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
  SBMLDocument_t *d  = NULL;
  Model_t        *m  = NULL;
  CvodeData data;

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
      if ( Opt.Validate ) {
	Warn(stderr, "Please make sure that path >%s< contains",
	     Opt.SchemaPath);
	Warn(stderr, "the correct SBML schema for validation.");
	Warn(stderr, "Or try running without validation.");
      }
      fatal(stderr, "%s:%d main(): Can't parse Model >%s<",
	    __FILE__, __LINE__, model);
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
      data = constructODEs(m);      
      printf("det(J) = %s\n",
	     SBML_formulaToString(data->det));
      CvodeData_free(data);   
      xfree(model);
      SBMLDocument_free(d);       
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

      data = constructODEs(m);
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
	just like done for integration with CVODE, but without
	replacement of constant parameters, compartments, species,
	assignment rules and function definitions. Then this
	simplified model will be printed to the given outfile or
	to stdout. Exit afterwards.
    */
    if ( Opt.PrintODEsToSBML == 1 ) {

      data = constructODEs(m);
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

    if ( strcmp(Opt.Parameter, "") != 0  ) {
      batchIntegrator(m);
      xfree(model);
      SBMLDocument_free(d);             
      return(EXIT_SUCCESS);  
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
	will be written to the structure CvodeData data.
    */
    data = constructODEs(m);
    /** Errors will cause the program to stop,
	e.g. when some mathematical expressions are missing.
    */
    if ( data->errors > 0 ) {
      fatal(stderr, "%s:%d main(): Can't construct ODEs for Model >%s<",
	    __FILE__, __LINE__, model);    
    }
    

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
    data->Error = Opt.Error;
    data->RError = Opt.RError;
    data->Mxstep = Opt.Mxstep;
    data->PrintOnTheFly = Opt.PrintOnTheFly;
    data->PrintMessage = Opt.PrintMessage;
    data->HaltOnEvent = Opt.HaltOnEvent;
    data->SteadyState = Opt.SteadyState;
    /* allow setting of Jacobian,
       only if its construction was succesfull */
    if ( data->UseJacobian == 1 ) {
      data->UseJacobian = Opt.Jacobian;
    }

    /** .... we can call the integrator function,
	that invokeds CVODE and stores results.
	The function will also handle events and
	check for steady states.
    */    
    if ( integrator(data) < 0 ) {
      Warn(stderr,
	   "Integration not successful. Results may not be complete.");
    }

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
	printJacobianTimeCourse(data);
      }

      /** -k: print time course of the reactions, i.e.
	  kinetic law expressions
      */      
      else if ( Opt.PrintReactions == 1 ) {
	printReactionTimeCourse(data);
      }

      /** -r: print time coures of ODE values
      */
      else if ( Opt.PrintRates == 1 ) {
	printOdeTimeCourse(data);
      }
      
      /** -d: print time course of the determinant
	  of the jacobian matrix
      */
      else if ( Opt.Determinant == 1 ) {
	printDeterminantTimeCourse(data);
      }
      
      /** Default (no printing options):
	  print species concentrations
       */
      else {
	printConcentrationTimeCourse(data);
      }      
    }
    /** -a, --all: print all results
    */
    else if ( !Opt.PrintOnTheFly && Opt.PrintAll ) {
      printJacobianTimeCourse(data);
      printReactionTimeCourse(data);
      printOdeTimeCourse(data);
      printConcentrationTimeCourse(data);
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
    CvodeData_free(data); 
    xfree(model);   
    SBMLDocument_free(d);
  }

  return(EXIT_SUCCESS);
}

SBMLResults
Model_odeSolver(SBMLDocument_t *d, CvodeSettings set) {
  
  SBMLDocument_t *d2 = NULL;
  CvodeData data;
  SBMLResults results;
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
      will be written to the structure CvodeData data.
  */
  
  data = constructODEs(m);
  /** Errors, found during odeConstruct, will cause the program to exit,
      e.g. when some mathematical expressions are missing.
  */
  if ( data->errors > 0 ) {
    fatal(stderr, "%s:%d main(): Can't construct ODEs for Model >%s<",
	  __FILE__, __LINE__, Model_getId(m));    
  }
    
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
  data->tout  = set.Time;
  data->nout  = set.PrintStep;
  data->tmult = set.Time / set.PrintStep;
  data->currenttime = 0.0;
  data->t0 = 0.0;
  data->Error  = set.Error;
  data->RError = set.RError;
  data->Mxstep = set.Mxstep;
  data->PrintOnTheFly = set.PrintOnTheFly;
  data->PrintMessage = set.PrintMessage;
  data->HaltOnEvent = set.HaltOnEvent;
  data->SteadyState = set.SteadyState;
  /* allow setting of Jacobian,
     only if its construction was succesfull */
  if ( data->UseJacobian == 1 ) {
    data->UseJacobian = set.UseJacobian;
  }  
  
  /** .... we can call the integrator function,
      that invokeds CVODE and stores results.
      The function will also handle events and
      check for steady states.
  */    

  if ( integrator(data) < 0 ) {
    Warn(stderr,
	 "Integration not successful. Results may not be complete.");
  }

  /* Write simulation results into result structure */
  results = Results_fromCvode(data); 
  CvodeData_free(data);
  /* free temporary level 2 version of the document */
  if ( d2 != NULL ) {
    SBMLDocument_free(d2);
  }
  return(results);
}

SBMLResults *
Model_odeSolverBatch (SBMLDocument_t *d,
		      CvodeSettings settings, VarySettings vary) {

  int i;
  double value, increment;
  SBMLResults *results;
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
    
  if(!(results = (SBMLResults *)calloc(vary.steps+1, sizeof(*results)))){
    fprintf(stderr, "failed!\n");
  }

  value = vary.start;
  increment = (vary.end - vary.start) / vary.steps;
  
  for ( i=0; i<=vary.steps; i++ ) {
    
      
    if ( ! Model_setValue(m, vary.id, value) ) {
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

int
Model_setValue(Model_t *m, const char *id, double value) {

  Compartment_t *c;
  Species_t *s;
  Parameter_t *p;

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

/* The function Results_fromCvode(CvodeData data)
   maps the integration results of CVODE
   back to SBML structures.
*/

SBMLResults
Results_fromCvode(CvodeData data) {

  int i, j, k, found;
  SBMLResults sbml_results;
  Model_t *m;
  Reaction_t *r;
  KineticLaw_t *kl;
  ASTNode_t **kls;
  
  CvodeResults cvode_results;
  TimeCourse tc;
  
  if ( data == NULL ) {
    fatal(stderr, "No data, please construct ODE system first.\n");
    return NULL;
  }
  else if ( data->results == NULL ) {
    fatal(stderr, "No results, please integrate first.\n");
    return NULL;
  }

  sbml_results = SBMLResults_create(data->m, data->results->nout+1);      
  cvode_results = data->results;
  m = data->m;

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
    /* updating time and values in CvodeData for calculations */
    data->currenttime = cvode_results->time[i]; 
    for ( j=0; j<data->neq; j++ ) {
      data->value[j] = cvode_results->value[j][i]; 
    }
 
    /* filling time courses for SBML species  */
    tc = sbml_results->species;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in CvodeData for values */
      found = 0;
      for ( k=0; k<data->neq; k++ ) {
	if ( (strcmp(tc->names[j],data->species[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	  found++;
	}
      }
      if ( ! found ) {
	for ( k=0; k<data->nass; k++ ) {
	  if ( (strcmp(tc->names[j],data->ass_parameter[k]) == 0) ) {
	    tc->values[i][j] = cvode_results->avalue[k][i];
	    found++;
	  }
	}	
      }
      if ( ! found ) {
	for ( k=0; k<data->nconst; k++ ) {
	  if ( (strcmp(tc->names[j], data->parameter[k]) == 0) ) {
	    tc->values[i][j] = cvode_results->pvalue[k][i];
	  }
	}	
      }      
    }
    
    /* filling variable compartment time courses */
    tc = sbml_results->compartments;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in CvodeData for values */
      found = 0;
      for ( k=0; k<data->neq; k++ ) {
	if ( (strcmp(tc->names[j], data->species[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	  found++;
	}
      }
      if ( ! found ) {
	for ( k=0; k<data->nass; k++ ) {
	  if ( (strcmp(tc->names[j], data->ass_parameter[k]) == 0) ) {
	    tc->values[i][j] = cvode_results->avalue[k][i];
	    found++;
	  }
	}	
      }
      if ( ! found ) {
	for ( k=0; k<data->nconst; k++ ) {
	  if ( (strcmp(tc->names[j], data->parameter[k]) == 0) ) {
	    tc->values[i][j] = cvode_results->pvalue[k][i];
	  }
	}	
      }      
    }         

    /* filling variable parameter time courses */
    tc = sbml_results->parameters;  
    for ( j=0; j<tc->num_val; j++ ) {
      /* search in CvodeData for values */
      found = 0;
      fflush(stderr);
      for ( k=0; k<data->neq; k++ ) {
	if ( (strcmp(tc->names[j], data->species[k]) == 0) ) {
	  tc->values[i][j] = cvode_results->value[k][i];
	  found++;
	}
      }
      if ( ! found ) {
	for ( k=0; k<data->nass; k++ ) {
	  if ( (strcmp(tc->names[j], data->ass_parameter[k]) == 0) ) {
	    tc->values[i][j] = cvode_results->avalue[k][i];
	    found++;
	  }
	}	
      } 
      if ( ! found ) {
	for ( k=0; k<data->nconst; k++ ) {
	  if ( (strcmp(tc->names[j], data->parameter[k]) == 0) ) {
	    tc->values[i][j] = cvode_results->pvalue[k][i];
	  }
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
