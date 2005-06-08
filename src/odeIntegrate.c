/*
  Last changed Time-stamp: <2005-05-31 12:26:59 raim>
  $Id: odeIntegrate.c,v 1.3 2005/06/08 15:15:29 afinney Exp $
  Last changed Time-stamp: <2004-12-23 16:16:19 xtof>
  $Id: odeIntegrate.c,v 1.3 2005/06/08 15:15:29 afinney Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sbml/SBMLTypes.h>
#include <sbml/common/common.h> 

/* Header Files for CVODE */
#include "cvode.h"    
#include "cvdense.h"  
#include "dense.h"

/* own header files */
#include "sbmlsolver/util.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/processAST.h"


/* static void */
/* checkData(CvodeData data); */
static int
checkSteadyState(CvodeData data);
static void
PrintFinalStats(long int iopt[], CvodeData data);
static void
f(integer N, real t, N_Vector y, N_Vector ydot, void *f_data);
static void
Jac(integer N, DenseMat J, RhsFn f, void *f_data, real t,
    N_Vector y, N_Vector fy, N_Vector ewt, real h, real uround,
    void *jac_data, long int *nfePtr, N_Vector vtemp1,
    N_Vector vtemp2, N_Vector vtemp3);
static int
checkTrigger(CvodeData data);
static int
checkSteadyState(CvodeData data);
static int
calculateResults(CvodeData data);
static int
updateModel(CvodeData data, int nout);


/************************ Integrator Program *************************/
/**
  The function "static int integrator(CvodeData data)" gets a filled
  structure "CvodeData" and calls CVODE to integrate the ODEs,
  represented as libsbml Abstract Syntax Trees.  The CvodeData also
  contains the number of equations neq, the nout to which CVODE should
  integrate (starting from time 0), an array of neq species names for
  which ODEs are defined and an array of neq, intial values for the
  species.  The integer errors in CvodeData should be 0 and is
  incremented by errors that occured during CvodeData construction.
*/

int
integrator(CvodeData data)
{
  real ropt[OPT_SIZE], reltol, t, tout, atol1, rtol1, t0, t1, tmult;
  long int iopt[OPT_SIZE];
  N_Vector y, abstol;
  void *cvode_mem;
  int iout, nout, flag, i;
  CvodeResults results; 

  
  if ( data->errors>0 ) {
    fprintf(stderr,
	    "CvodeData contains %d error(s). Integration is aborted.\n",
	    data->errors);
    return 0;
  }
  else {
    if ( data->PrintMessage )
      fprintf(stderr, "Data ready for integration.\n");
  }

  /* models can only be defined by assignments, or constants.
     Such models don't need integration!     
  */
  if ( data->neq == 0 ) {
    return calculateResults(data);
  }
  

  /* CVODE settings: set Problem Constants */
  /* set first output time, output intervals and number of outputs
     from the values in CvodeData data */
   
  atol1 = data->Error;        /* vector absolute tolerance components */ 
  rtol1 = data->RError;       /* scalar relative tolerance */
  t0 = 0.0;                 /* initial time           */
  t1 = data->tmult;         /* first output time      */
  tmult = t1;               /* output time factor     */
  nout = data->nout;        /* number of output steps */
  data->cnt = data->nout;   /* used counting actual output steps */

  /* Allocate y, abstol vectors */
  y = N_VNew(data->neq, NULL);     
  abstol = N_VNew(data->neq, NULL);


  /* initialize Ith(y,i) and Ith(abstol,i) the absolute tolerance vector */  
  for ( i=0; i<data->neq; i++ ) {
    N_VIth(y,i) = data->value[i];   /* vector of initial values             */
    N_VIth(abstol,i) = atol1;       /* vector absolute tolerance components */ 
  }
  reltol = rtol1;                  /* scalar relative tolerance            */

  /* (no) optional inputs and outputs to CVODE initialized */ 
  for ( i=0; i < OPT_SIZE; i++ ) {
    iopt[i] = 0; ropt[i] = 0.;
  }    

  /** Setting MXSTEP:
    the only input set is MXSTEP, the maximal number of internal
    steps that CVode takes to reach outtime tout
  */
  iopt[MXSTEP] = data->Mxstep;
  
  /* Call CVodeMalloc to initialize CVODE: 

     data->neq     is the problem size = number of equations
     f             is the user's right hand side function in y'=f(t,y)
     t0            is the initial time
     y             is the initial dependent variable vector
     BDF           specifies the Backward Differentiation Formula
     NEWTON        specifies a Newton iteration
     SV            specifies scalar relative and vector absolute tolerances
     &reltol       is a pointer to the scalar relative tolerance
     abstol        is the absolute tolerance vector
     data          the user data passed to CVODE: includes
                   all ODEs and parameters
     stderr        Error file pointer, here stderr
     TRUE          indicates there are some optional inputs in iopt and ropt
     iopt          is an array used to communicate optional integer
                   input and output
     ropt          is an array used to communicate optional real
                   input and output
     NULL          could be a pointer to machine environment-specific
                   information

     A pointer to CVODE problem memory is returned and stored in cvode_mem. */

  
  cvode_mem = CVodeMalloc(data->neq, f, t0, y, BDF, NEWTON, SV,
			  &reltol, abstol, data, stderr, TRUE,
			  iopt, ropt, NULL);

  if ( cvode_mem == NULL ) {
    fprintf(stderr, "CVodeMalloc failed.\n"); return 0;
  }

  /* CVDiag(cvode_mem); */
  /* direct method; approx diagonal Jac by way of diff quot */  

  if ( data->UseJacobian == 1 && data->jacob != NULL ){
    /*
      Call CVDense to specify the CVODE dense linear solver with the
      user-supplied Jacobian matrix evalution routine Jac.
    */
    CVDense(cvode_mem, Jac, NULL);
    if ( data->PrintMessage )
      fprintf(stderr,
	      "Using automatically generated Jacobian Matrix"
	      " for integration.\n");
  }
  else{
    /*
      CVDense(cvode_mem, NULL, NULL)
      uses difference quotient routine CVDenseDQJac to approximate
      values of the Jacobian matrix.
    */
    CVDense(cvode_mem, NULL, NULL);
    if ( data->PrintMessage )
      fprintf(stderr,
	      "Using CVODE's internal approximation of the Jacobian"
	      " for integration.\n");
  }

  /*
    first, check if formulas can be evaluated, and CvodeData
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<data->neq; i++ ) {
    evaluateAST(data->ode[i], data);
  }
  
  /*
    Now we should have all variables, and can allocate the
    results structure, where the time series will be stored
  */
  if ( data->results == NULL ) {
    results = CvodeResults_create(data);
    data->results = results;
  }
  else {
    results = data->results;
  }
  

  /* Writing initial conditions to results structure */
  results->time[0] = data->t0;
  for ( i=0; i<data->neq; i++ ) {
    results->value[i][0] = data->value[i];
  }
  for ( i=0; i<data->nass; i++ ) {
    results->avalue[i][0] = data->avalue[i];
  }
  for ( i=0; i<data->nconst; i++ ) {
    results->pvalue[i][0] = data->pvalue[i];
  }

  /** Command-line option -f/--onthefly:
      print initial values, if on-the-fly printint is set
  */
  if ( data->PrintOnTheFly && data->run == 0 ) {
	if ( data->t0 == 0.0 )
	{
      fprintf(stderr, "\nPrinting results on the fly to %s!\n",
	          data->filename == NULL ? "stdout" :
	          data->filename);
      fprintf(stderr, "Overruling all other print options!!\n\n");      
      fprintf(data->outfile, "#t ");
      for ( i=0; i<data->neq; i++ )
        fprintf(data->outfile, "%s ", data->speciesname[i]);
      for ( i=0; i<data->nass; i++ )
        fprintf(data->outfile, "%s ", data->ass_parameter[i]);
      for ( i=0; i<data->nconst; i++ )
        fprintf(data->outfile, "%s ", data->parameter[i]);
      fprintf(data->outfile, "\n");
	}

    fprintf(data->outfile, "%g ", data->t0);
    for ( i=0; i<data->neq; i++ )
      fprintf(data->outfile, "%g ", data->value[i]);
    for ( i=0; i<data->nass; i++ )
      fprintf(data->outfile, "%g ", data->avalue[i]);
    for ( i=0; i<data->nconst; i++ )
      fprintf(data->outfile, "%g ", data->pvalue[i]);
    fprintf(data->outfile, "\n");
  }
  else {
    if ( data->PrintMessage )
      fprintf(stderr,"Integrating        ");
  }

  /*
    In loop over output points, call CVode, test for error
    and print results at specified intervals into results
    structure. If option '-f' or '--onthefly' was set, then
    print results immediately to the given outfile (default:
    stdout).
  */
  
  for (iout=1, tout=t1; iout <= nout; iout++, tout += tmult){


    /* !! calling Cvode !! */
    flag = CVode(cvode_mem, tout, y, &t, NORMAL);
    if ( flag != SUCCESS ) {
      N_VFree(y);                  /* Free the y and abstol vectors */
      N_VFree(abstol);
      CVodeFree(cvode_mem);        /* Free the CVODE problem memory */
      
      /* on flag -6/CONV_FAILURE
	 try again, but now with/without generated Jacobian matrix  */
      if ( flag == CONV_FAILURE && data->run == 0 ) {
	fprintf(stderr, "Trying again; now with %s Jacobian matrix\n",
		data->UseJacobian ?
		"CVODE's internal approximation of the" :
		"automatically generated");
	data->UseJacobian = !data->UseJacobian;
	data->run++;
	for ( i=0; i<data->neq; i++ ) {
	  data->value[i] = results->value[i][0];
	}
	for ( i=0; i<data->nass; i++ ) {
	  data->avalue[i] = results->avalue[i][0];
	}
	for ( i=0; i<data->nconst; i++ ) {
	  data->pvalue[i] = results->pvalue[i][0];
	}
	data->currenttime = data->t0;
	if ( integrator(data) < 0 ) {
	  Warn(stderr,
	       "Integration not successful. Results may not be complete.");
	  return 0;
	}
	return 1;
      }
      else {
      	fprintf(data->outfile, "# CVode failed, with flag=%d, at time %g\n",
		flag, t+data->t0);
      }
      return (flag);
    }

    /* update CvodeData */
    data->currenttime = t;
    results->nout = iout;
    data->cnt--;
    
    for ( i=0; i<data->neq; i++ )
      data->value[i] = N_VIth(y,i);

    for ( i=0; i<data->nass; i++ )
      data->avalue[i] = evaluateAST(data->assignment[i], data);

    /* store results */
    results->time[iout] = t + data->t0;
    for ( i=0; i<data->neq; i++ ) {
      results->value[i][iout] = data->value[i];
    }
    for ( i=0; i<data->nass; i++ ) {
      results->avalue[i][iout] = data->avalue[i];
    }
    for ( i=0; i<data->nconst; i++ ) {
      results->pvalue[i][iout] = data->pvalue[i];
    }

          
    /* check for events, not functional at the moment!! */
    if ( checkTrigger(data) ) {
      N_VFree(y);                  /* Free the y and abstol vectors */
      N_VFree(abstol);
      CVodeFree(cvode_mem);        /* Free the CVODE problem memory */
      return 1;
    }
    /* check for steady state if set by commandline option -s */
    if ( data->SteadyState == 1 ) {
      if ( checkSteadyState(data) ) {
	data->nout = iout;
	iout = nout+1;
      }      
    }
    
    /* print immediately if data->PrintOnTheFly was set
       with '-d' or '--onthefly'
     */
    if ( data->PrintOnTheFly ) {
      fprintf(data->outfile, "%g ", t + data->t0);
      /* fprintf(stdout, "%g ", t + data->t0); */
      for ( i=0; i<data->neq; i++ )
	fprintf(data->outfile, "%g ", data->value[i]);
      for ( i=0; i<data->nass; i++ )
	fprintf(data->outfile, "%g ", data->avalue[i]);
      for ( i=0; i<data->nconst; i++ )
	fprintf(data->outfile, "%g ", data->pvalue[i]);
      fprintf(data->outfile, "\n");
    }
    else if ( data->PrintMessage ) {
      const  char chars[5] = "|/-\\";
      fprintf(stderr, "\b\b\b\b\b\b");
      fprintf(stderr, "%.2f %c", (float)iout/(float)nout, chars[iout % 4]);

    }
  }

  N_VFree(y);                  /* Free the y and abstol vectors */
  N_VFree(abstol);   
  CVodeFree(cvode_mem);        /* Free the CVODE problem memory */

  if ( !data->PrintOnTheFly && data->PrintMessage ) {
    fprintf(stderr,
	    "finished. Results stored.\n");
  }

  if ( data->PrintMessage )
    PrintFinalStats(iopt, data);        /* Print some final statistics   */

  /* data->results = results; */

/*   updateModel(data, data->nout); */
  
  return 0;
}




/************************ Private Helper Function ************************/

/**
   Prints some final statistics of the calls to CVODE routines, that
   are located in CVODE's iopt array.
*/

static void PrintFinalStats(long int iopt[], CvodeData data)
{
  fprintf(stderr, "\nIntegration Parameters:\n");
  fprintf(stderr, "mxstep   = %-6g rel.err. = %-6g abs.err. = %-6g \n",
	  data->Mxstep, data->RError, data->Error);
  fprintf(stderr, "CVode Statistics:\n");
  fprintf(stderr, "nst = %-6ld nfe  = %-6ld nsetups = %-6ld nje = %ld\n",
	 iopt[NST], iopt[NFE], iopt[NSETUPS], iopt[DENSE_NJE]);
  fprintf(stderr, "nni = %-6ld ncfn = %-6ld netf = %ld\n\n",
	 iopt[NNI], iopt[NCFN], iopt[NETF]);
}


/***************** Functions Called by the CVODE Solver ******************/

/**
   f routine. Compute f(t,y).
   This function is called by CVODE's integration routines every time
   needed.
   It evaluates the ODEs with the current species values, as supplied
   by CVODE's N_VIth(y,i) vector containing the values of all species.
   These values are first written back to CvodeData.
   Then every ODE is passed to processAST, together with the CvodeData, and
   this function calculates the current value of the ODE.
   The returned value is written back to CVODE's N_VIth(ydot,i) vector that
   contains the values of the ODEs.
   The the CvodeData is updated again with CVODE's internal values for
   species.
*/

static void f(integer N, real t, N_Vector y, N_Vector ydot, void *f_data)
{
  
  int i;
  CvodeData data;
  data = (CvodeData) f_data;

  /* update CvodeData */
  for ( i=0; i<data->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;

  /* evaluate ODEs */
  for ( i=0; i<data->neq; i++ ) {
    N_VIth(ydot,i) = evaluateAST(data->ode[i],data);
  } 

  /* update CvodeData */
  for ( i=0; i<data->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;

}

/**
   Jacobian routine. Compute J(t,y).
   This function is (optionally) called by CVODE's integration routines
   every time needed.
   Very similar to the f routine, it evaluates the Jacobian matrix
   equations with CVODE's current values and writes the results
   back to CVODE's internal vector DENSE_ELEM(J,i,j).
*/

static void Jac(integer N, DenseMat J, RhsFn f, void *f_data, real t,
                N_Vector y, N_Vector fy, N_Vector ewt, real h, real uround,
                void *jac_data, long int *nfePtr, N_Vector vtemp1,
                N_Vector vtemp2, N_Vector vtemp3)
{
  
  int i, j;
  CvodeData data;
  data = (CvodeData) f_data;
  
   /* update CvodeData */
  for ( i=0; i<data->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;

  /* evaluate Jacobian*/
  for ( i=0; i<data->neq; i++ ) {
    for ( j=0; j<data->neq; j++ ) {
      DENSE_ELEM(J,i,j) = evaluateAST(data->jacob[i][j], data);
     }
  }
  
  /* update CvodeData */
  for ( i=0; i<data->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;
}
 

/*
  Soon to be implemented checkTrigger routine:
  will return, 1 if trigger is fired to stop the calling
  integrator and start an approximation to the exact
  time of trigger firing.
*/

static int
checkTrigger(CvodeData data){
  
  int i, j, k, fired;
  ASTNode_t *trigger, *assignment;
  Event_t *e;
  EventAssignment_t *ea;
  CvodeResults results;

  results = data->results;

  fired = 0;

  for ( i=0; i<Model_getNumEvents(data->simple); i++ ) {
    e = Model_getEvent(data->simple, i);
    trigger = (ASTNode_t *) Event_getTrigger(e);
    if ( data->trigger[i] == 0 && evaluateAST(trigger, data) ) {
      if (data->HaltOnEvent)
		  fprintf(data->outfile,
	        "# Trigger %d : %s fired at %g. Aborting simulation!!\n",
	        i, SBML_formulaToString(trigger), data->t0 + data->currenttime);
      fired++;
      data->trigger[i] = 1;      
      for ( j=0; j<Event_getNumEventAssignments(e); j++ ) {
	ea = Event_getEventAssignment(e, j);
	assignment = (ASTNode_t *) EventAssignment_getMath(ea);
	for ( k=0; k<data->neq; k++ ) {
	  if ( strcmp(EventAssignment_getVariable(ea),
		      data->species[k]) == 0 ) {
	    data->value[k] = evaluateAST(assignment, data);
	    results->value[k][results->nout] = data->value[k];
	  }
	}
	for ( k=0; k<data->nass; k++ ) {
	  if ( strcmp(EventAssignment_getVariable(ea),
		      data->ass_parameter[k]) == 0 ) {
	    data->avalue[k] = evaluateAST(assignment, data);
	    results->avalue[k][results->nout] = data->avalue[k];
	  }
	}
	for ( k=0; k<data->nconst; k++ ) {
	  if ( strcmp(EventAssignment_getVariable(ea),
		      data->parameter[k]) == 0 ) {
	    data->pvalue[k] = evaluateAST(assignment, data);
	    results->pvalue[k][results->nout] = data->pvalue[k];
	  }
	}   
      }
      
    }
    else {
      data->trigger[i] = 0;
    }
  }

  if ( fired > 0 ) {
    if ( data->HaltOnEvent ) {
      data->results->nout -=1;
    }
    else {
      data->tout -=data->currenttime;
      if ( data->tout != 0 ) {
	data->t0 += data->currenttime;
	data->nout = data->cnt;
	integrator(data);
      }
    }
    return 1;
  }
  return 0;
  
}

/**
  provisional identification of a steady state,
  evaluates mean and std of rates and returns 1 if a "steady state"
  is reached to stop  the calling integrator.
  This function is only called by the integrator function if specified
  via commandline options!
*/
/* NOTE: provisional steady state finding! Don't rely on that! */
static int
checkSteadyState(CvodeData data){

  int i;
  
  /* calculate the mean and standard deviation of rates of change and
     store in CvodeData */
  data->dy_mean = 0.0;
  data->dy_var = 0.0;
  data->dy_std = 0.0;
  
  for ( i=0; i<data->neq; i++ ) {
    data->dy_mean += fabs(evaluateAST(data->ode[i],data));
  }
  data->dy_mean = data->dy_mean / data->neq;
  for ( i=0; i<data->neq; i++ ) {
    data->dy_var += SQR(evaluateAST(data->ode[i],data) - data->dy_mean);
  }
  data->dy_var = data->dy_var / (data->neq -1);
  data->dy_std = SQRT(data->dy_var);

  /* stop integrator if mean + std of rates of change are lower than
     1e-11 */
  if ( (data->dy_mean + data->dy_std) < 1e-11 ) {
    data->steadystate = 1;
    fprintf(stderr, "\n\n");
    fprintf(stderr,
	    "Steady state found. Simulation aborted at %g seconds\n\n",
	    data->currenttime);
    fprintf(stderr, "Rates at abortion:  \n");
    fprintf(stderr, "%g  ", data->currenttime);
    for ( i=0; i<data->neq; i++ ) {
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->species[i], fabs(evaluateAST(data->ode[i],data)));
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Mean of rates:\n %g, std %g\n\n",
	    data->dy_mean, data->dy_std);
    fprintf(stderr, "\n");    
    return(1) ;
  }
  else {
    data->steadystate = 0;
    return(0);
  }
}

/** calculates results for models without ODEs,
    i.e. that don't need integration    
*/
static int
calculateResults(CvodeData data) {

  int i;
  CvodeResults results;
  int iout;

  /* set first output time, output intervals and number of outputs
     from the values in CvodeData data */
  /*   t0 = data->t0; */

  data->currenttime = data->t0;

  /*
    first, check if formulas can be evaluated, and CvodeData
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<data->neq; i++ ) {
    evaluateAST(data->ode[i], data);
  }
  
  /*
    Now we should have all variables, and can allocate the
    results structure, where the time series will be stored
  */
  if ( data->results == NULL ) {
    results = CvodeResults_create(data);
    data->results = results;
  }
  else {
    results = data->results;
  }
 /* Writing initial conditions to results structure */
  
  results->time[0]  = data->t0;
  
  for ( i=0; i<data->nass; i++ ) {
    results->avalue[i][0] = evaluateAST(data->assignment[i], data);
  }
  for ( i=0; i<data->nconst; i++ ) {
    results->pvalue[i][0] = data->pvalue[i];
  }

  /** Command-line option -f/--onthefly:
      print initial values, if on-the-fly printint is set
  */
  if ( data->PrintOnTheFly ) {
    fprintf(stderr, "\nPrinting results on the fly to %s!\n",
	    data->filename == NULL ? "stdout" :
	    data->filename);
    fprintf(stderr, "Overruling all other print options!!\n\n");      
    fprintf(data->outfile, "#t ");
    for ( i=0; i<data->nass; i++ )
      fprintf(data->outfile, "%s ", data->ass_parameter[i]);
    for ( i=0; i<data->nconst; i++ )
      fprintf(data->outfile, "%s ", data->parameter[i]);
    fprintf(data->outfile, "\n");
    
    fprintf(data->outfile, "%g ", data->t0);
    for ( i=0; i<data->nass; i++ )
      fprintf(data->outfile, "%g ", data->avalue[i]);
    for ( i=0; i<data->nconst; i++ )
      fprintf(data->outfile, "%g ", data->pvalue[i]);
    fprintf(data->outfile, "\n");
  }
  else {
    fprintf(stderr,"Calculating results (no ODEs!)");
  }

  for ( iout=1; iout <= data->nout; iout++ ) {

    /* update CvodeData */

    results->nout = iout;
    data->currenttime += data->tmult;

    /* calculate current values */
    for ( i=0; i<data->nass; i++ )
      data->avalue[i] = evaluateAST(data->assignment[i], data);

    /* store results */
    results->time[iout] = data->currenttime;

    for ( i=0; i<data->nass; i++ ) {
      results->avalue[i][iout] = data->avalue[i];
    }
    for ( i=0; i<data->nconst; i++ ) {
      results->pvalue[i][iout] = data->pvalue[i];
    }

    /* print immediately if data->PrintOnTheFly was set
       with '-d' or '--onthefly'
     */
    if ( data->PrintOnTheFly ) {
      fprintf(data->outfile, "%g ", data->currenttime);
      /* fprintf(stdout, "%g ", t + data->t0); */
      for ( i=0; i<data->nass; i++ )
	fprintf(data->outfile, "%g ", data->avalue[i]);
      for ( i=0; i<data->nconst; i++ )
	fprintf(data->outfile, "%g ", data->pvalue[i]);
      fprintf(data->outfile, "\n");
    }
    else {
      fprintf(stderr, ".");
    }

         
    /* check for events, not functional at the moment!! */
    if ( checkTrigger(data) ) {
      return 0;
    }
    /* check for steady state if set by commandline option -s */
    if ( data->SteadyState == 1 ) {
      if ( checkSteadyState(data) ) {
	data->nout = iout;
	iout = data->nout+1;
      }      
    }
  }


  if ( !data->PrintOnTheFly ) {
    fprintf(stderr,
	    "finished. Results stored.\n");
  }
  
/*   updateModel(data, data->nout); */
  
  return 0;
}

/** writes current simulation data to
    original model */
static int
updateModel(CvodeData data, int nout) {

  int i;
  Species_t *s;
  Compartment_t *c;
  Parameter_t *p;  
  
  for ( i=0; i<data->neq; i++ ) {
    if ( (s = Model_getSpeciesById(data->m, data->species[i])) != NULL ) {
      Species_setInitialConcentration(s,
				      data->results->value[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->m, data->species[i])) !=
	      NULL ) {
      Compartment_setSize(c, data->results->value[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->m, data->species[i])) !=
	      NULL ) {
      Parameter_setValue(p, data->results->value[i][nout]);
    }
  }
  for ( i=0; i<data->nass; i++ ) {
    if ( (s = Model_getSpeciesById(data->m, data->ass_parameter[i]))
	 != NULL ) {
      Species_setInitialConcentration(s,
				      data->results->avalue[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->m, data->ass_parameter[i]))
	      != NULL ) {
      Compartment_setSize(c, data->results->avalue[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->m, data->ass_parameter[i]))
	      != NULL ) {
      Parameter_setValue(p, data->results->avalue[i][nout]);
    }
  }
  for ( i=0; i<data->nconst; i++ ) {
    if ( (s = Model_getSpeciesById(data->m, data->parameter[i]))
	 != NULL ) {
      Species_setInitialConcentration(s,
				      data->results->pvalue[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->m, data->parameter[i]))
	      != NULL ) {
      Compartment_setSize(c, data->results->pvalue[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->m, data->parameter[i]))
	      != NULL ) {
      Parameter_setValue(p, data->results->pvalue[i][nout]);
    }
  }
  return 1;

}
/* static void */
/* checkData(CvodeData data) { */

/*   int i, j; */
/*   double result; */
  
/*   for ( i=0; i<data->neq; i++ ) { */
/*     result = evaluateAST(data->ode[i], data); */
/*     if ( isnan(result) ) { */
/*       fprintf(stderr,
	 "ODE for species %s is not a number.\n", data->species[i]); */
/*     } */
/*   } */
/*   if ( data->jacob != NULL ){ */
/*     for ( i=0; i<data->neq; i++ ) { */
/*       for ( j=0; j<data->neq; j++ ) { */
/* 	result = evaluateAST(data->jacob[i][j], data); */
/* 	if ( isnan(result) ) { */
/* 	  fprintf(stderr, "Jacobian (d[%s]/dt) / d[%s] is not a number.\n", */
/* 		  data->species[i], */
/* 		  data->species[j]); */
/* 	} */
/*       } */
/*     } */
/*   } */
/* } */
  


/* End of file */
