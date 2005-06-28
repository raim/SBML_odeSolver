#include "sbmlsolver/integratorInstance.h"

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


/* Header Files for CVODE */
#include "cvode.h"    
#include "cvdense.h"  
#include "dense.h"

#include "sbmlsolver/util.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"

/* integrator state information */
struct integratorInstance
{
  real ropt[OPT_SIZE], reltol, t, tout, atol1, rtol1, t0, t1, tmult;
  long int iopt[OPT_SIZE];
  N_Vector y, abstol;
  void *cvode_mem;
  int iout, nout;
  CvodeResults results; 
  CvodeData data;
} ;

static void
f(integer N, real t, N_Vector y, N_Vector ydot, void *f_data);
static void
Jac(integer N, DenseMat J, RhsFn f, void *f_data, real t,
    N_Vector y, N_Vector fy, N_Vector ewt, real h, real uround,
    void *jac_data, long int *nfePtr, N_Vector vtemp1,
    N_Vector vtemp2, N_Vector vtemp3);
static int checkTrigger(integratorInstance_t *);
static int checkSteadyState(CvodeData data);

SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *om, CvodeSettings *options)
{
    CvodeData data = CvodeData_createFromODEModel(om, 0 /* use stdout for results */);

    if (options->Indefinitely)
    {
        data->tout = -1; /* delibrate trap for bugs - there is no defined end time */
        data->nout = -1; /* delibrate trap for bugs - this is no defined number of steps */
        data->tmult = options->Time;
    }
    else
    {
        data->tout  = options->Time;
        data->nout  = options->PrintStep;
        data->tmult = data->tout / data->nout;
    }

    data->currenttime = 0.0;
    data->t0 = 0.0;
    data->Error = options->Error;
    data->RError = options->RError;
    data->Mxstep = options->Mxstep;
    data->PrintOnTheFly = options->PrintOnTheFly;
    data->PrintMessage = options->PrintMessage;
    data->HaltOnEvent = options->HaltOnEvent;
    data->SteadyState = options->SteadyState;
    data->storeResults = !options->Indefinitely && options->StoreResults;
    /* allow setting of Jacobian,
       only if its construction was succesfull */
    data->UseJacobian = om->simplified && options->UseJacobian;

    return IntegratorInstance_createFromCvodeData(data);
}

SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *ii, variableIndex_t *vi)
{
    CvodeData data = ii->data ;

    switch (vi->type)
    {
        case SPECIES : 
            return data->value[vi->index];

        case ASSIGNMENT_PARAMETER :
            return data->avalue[vi->index];

        case PARAMETER :
            return data->pvalue[vi->index];
    }

    return 0.0;
}

SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *engine)
{
    return engine->t + engine->data->t0 ;
}

SBML_ODESOLVER_API int IntegratorInstance_timeCourseCompleted(integratorInstance_t *engine)
{
    return engine->iout > engine->nout;
}

SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *ii, variableIndex_t *vi, double value)
{
    CvodeData data = ii->data ;

    switch (vi->type)
    {
        case SPECIES : 
            data->value[vi->index] = value;
            return;

        case ASSIGNMENT_PARAMETER :
            data->avalue[vi->index] = value;
            return;

        case PARAMETER :
            data->pvalue[vi->index] = value;
            return;
    }
}

/* create structures in the integration engine for CVODE 
    return 1 => success
    return 0 => failure
*/
int IntegratorInstance_createODESolverStructures(integratorInstance_t *engine)
{
    int i ;
    CvodeData data = engine->data;

    /* Allocate y, abstol vectors */
    engine->y = N_VNew(data->model->neq, NULL);     
    engine->abstol = N_VNew(data->model->neq, NULL);

    /* initialize Ith(y,i) and Ith(abstol,i) the absolute tolerance vector */  
    for ( i=0; i<data->model->neq; i++ ) {
        N_VIth(engine->y,i) = data->value[i];   /* vector of initial values             */
        N_VIth(engine->abstol,i) = engine->atol1;       /* vector absolute tolerance components */ 
    }
    engine->reltol = engine->rtol1;                  /* scalar relative tolerance            */

    /* (no) optional inputs and outputs to CVODE initialized */ 
    for ( i=0; i < OPT_SIZE; i++ ) {
        engine->iopt[i] = 0; engine->ropt[i] = 0.;
    }    

    /** Setting MXSTEP:
        the only input set is MXSTEP, the maximal number of internal
        steps that CVode takes to reach outtime tout
    */
    engine->iopt[MXSTEP] = data->Mxstep;
        
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

        
    engine->cvode_mem = CVodeMalloc(data->model->neq, f, engine->t0, engine->y, BDF, NEWTON, SV,
			    &(engine->reltol), engine->abstol, data, stderr, TRUE,
			    engine->iopt, engine->ropt, NULL);

    if ( engine->cvode_mem == NULL ) {
        SolverError_error(
            FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED, "CVodeMalloc failed");
        return 0; /* error */
    }

    /* CVDiag(cvode_mem); */
    /* direct method; approx diagonal Jac by way of diff quot */  

    if ( data->UseJacobian == 1 && data->model->jacob != NULL ){
        /*
        Call CVDense to specify the CVODE dense linear solver with the
        user-supplied Jacobian matrix evalution routine Jac.
        */
        CVDense(engine->cvode_mem, Jac, NULL);
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
        CVDense(engine->cvode_mem, NULL, NULL);
        if ( data->PrintMessage )
        fprintf(stderr,
	        "Using CVODE's internal approximation of the Jacobian"
	        " for integration.\n");
    }

    return 1 ; /* OK */
}

void IntegratorInstance_freeODESolverStructures(integratorInstance_t *engine)
{
    /* free stuff allocated for cvode */ 
    N_VFree(engine->y);                  /* Free the y and abstol vectors */
    N_VFree(engine->abstol);
    CVodeFree(engine->cvode_mem);        /* Free the CVODE problem memory */
}

/* frees the integrator */
void IntegratorInstance_free(integratorInstance_t *engine)
{
    if (engine->data->model->neq)
        IntegratorInstance_freeODESolverStructures(engine);

    CvodeData_freeExcludingModel(engine->data);
    free(engine);
}

void IntegratorInstance_freeExcludingCvodeData(integratorInstance_t *engine)
{
    if (engine->data->model->neq)
        IntegratorInstance_freeODESolverStructures(engine);

    free(engine);
}

/* does all the stuff currently before the main loop in integrate()
   a NULL result indicates an error */
integratorInstance_t *IntegratorInstance_createFromCvodeData(CvodeData data)
{
  int i ;
  integratorInstance_t *engine = malloc(sizeof(integratorInstance_t));

  /* CVODE settings: set Problem Constants */
  /* set first output time, output intervals and number of outputs
     from the values in CvodeData data */
  
  engine->data = data;
  engine->atol1 = data->Error;        /* vector absolute tolerance components */ 
  engine->rtol1 = data->RError;       /* scalar relative tolerance */
  engine->t0 = 0.0;                 /* initial time           */
  engine->t1 = data->tmult;         /* first output time      */
  engine->tmult = engine->t1;               /* output time factor     */
  engine->nout = data->nout;        /* number of output steps */
  data->cnt = data->nout;   /* used counting actual output steps */
  engine->t = 0;

  if (data->model->neq)
  {
      IntegratorInstance_createODESolverStructures(engine);
      RETURN_ON_ERRORS_WITH(NULL);
  }

  /*
    first, check if formulas can be evaluated, and CvodeData
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<data->model->neq; i++ ) {
    evaluateAST(data->model->ode[i], data);
  }
  
  /*
    Now we should have all variables, and can allocate the
    results structure, where the time series will be stored
  */
  if ( data->results == NULL ) {
    engine->results = CvodeResults_create(data);
    data->results = engine->results;
  }
  else {
    engine->results = data->results;
  }
  

  /* Writing initial conditions to results structure */

  if (data->storeResults)
  {
    engine->results->time[0] = data->t0;
    for ( i=0; i<data->model->neq; i++ ) {
      engine->results->value[i][0] = data->value[i];
    }
    for ( i=0; i<data->model->nass; i++ ) {
      engine->results->avalue[i][0] = data->avalue[i];
    }
    for ( i=0; i<data->model->nconst; i++ ) {
      engine->results->pvalue[i][0] = data->pvalue[i];
    }
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
      for ( i=0; i<data->model->neq; i++ )
        fprintf(data->outfile, "%s ", data->model->speciesname[i]);
      for ( i=0; i<data->model->nass; i++ )
        fprintf(data->outfile, "%s ", data->model->ass_parameter[i]);
      for ( i=0; i<data->model->nconst; i++ )
        fprintf(data->outfile, "%s ", data->model->parameter[i]);
      fprintf(data->outfile, "\n");
	}

    fprintf(data->outfile, "%g ", data->t0);
    for ( i=0; i<data->model->neq; i++ )
      fprintf(data->outfile, "%g ", data->value[i]);
    for ( i=0; i<data->model->nass; i++ )
      fprintf(data->outfile, "%g ", data->avalue[i]);
    for ( i=0; i<data->model->nconst; i++ )
      fprintf(data->outfile, "%g ", data->pvalue[i]);
    fprintf(data->outfile, "\n");
  }
  else {
    if ( data->PrintMessage )
      fprintf(stderr,"Integrating        ");
  }

  /* set up loop variables */
  engine->iout=1;
  engine->tout=engine->t1;

  return engine ;
}

/* moves the current simulation one time step, returns 1 if
   the intergation can continue, 0 otherwise */
int IntegratorInstance_integrateOneStep(integratorInstance_t *engine)
{
    int i, flag ;

    if (engine->data->model->neq)
    {
        /* !! calling Cvode !! */
        flag = CVode(engine->cvode_mem, engine->tout,
		     engine->y, &engine->t, NORMAL);
    
        if ( flag != SUCCESS )
        {
            char *message[] =
            {
              /* SUCCESS */
                "Success",
              /* CVODE_NO_MEM */
                "The cvode_mem argument was NULL",
              /* ILL_INPUT */
                "One of the inputs to CVode is illegal. This "
                "includes the situation when a component of the "
                "error weight vectors becomes < 0 during "
                "internal time-stepping. The ILL_INPUT flag "
                "will also be returned if the linear solver "
                "routine CV--- (called by the user after "
                "calling CVodeMalloc) failed to set one of the "
                "linear solver-related fields in cvode_mem or "
                "if the linear solver's init routine failed. In "
                "any case, the user should see the printed "
                "error message for more details.",
              /* TOO_MUCH_WORK */
                "The solver took %g internal steps but could not compute variable values for time %g",
              /* TOO_MUCH_ACC */
                "The solver could not satisfy the accuracy " 
                "requested for some internal step.",
              /* ERR_FAILURE */
                "Error test failures occurred too many times "
                "during one internal time step or "
                "occurred with |h| = hmin.",
              /* CONV_FAILURE */
                "Convergence test failures occurred too many "
                "times during one internal time step or occurred with |h| = hmin.",
              /* SETUP_FAILURE */
                "The linear solver's setup routine failed in an "
                "unrecoverable manner.",
              /* SOLVE_FAILURE */
                "The linear solver's solve routine failed in an "
                "unrecoverable manner."
            };

            SolverError_error(
                ERROR_ERROR_TYPE,
                flag,
                message[flag * -1],
                engine->data->Mxstep,
                engine->tout);
            SolverError_error(
                WARNING_ERROR_TYPE,
                SOLVER_ERROR_INTEGRATION_NOT_SUCCESSFUL,
                "Integration not successful. Results are not complete.");

	        return 0 ; /* Error - stop integration*/
        }
    }
    else
    {
        /* faking a timestep when we don't have ODEs */
        engine->t = engine->tout ;
    }

    /* update CvodeData */
    engine->data->currenttime = engine->t;
    engine->data->cnt--;
    
    for ( i=0; i<engine->data->model->neq; i++ )
      engine->data->value[i] = N_VIth(engine->y,i);

    for ( i=0; i<engine->data->model->nass; i++ )
      engine->data->avalue[i] =
	evaluateAST(engine->data->model->assignment[i], engine->data);

    if (checkTrigger(engine))
    {
        /* recalculate assignments - they may be dependent
	   on event assignment results */
        for ( i=0; i<engine->data->model->nass; i++ )
            engine->data->avalue[i] =
	      evaluateAST(engine->data->model->assignment[i], engine->data);

        if (engine->data->HaltOnEvent)
            return 0; /* stop integration */

        if (engine->data->model->neq)
        {
            /* reset CVODE */
            IntegratorInstance_freeODESolverStructures(engine);
            engine->t0 = engine->t;
            IntegratorInstance_createODESolverStructures(engine);
            RETURN_ON_ERRORS_WITH(0);
        }
    }

    /* store results */
    if (engine->data->storeResults)
    {
      engine->results->nout = engine->iout;
      engine->results->time[engine->iout] = engine->t + engine->data->t0;
      for ( i=0; i<engine->data->model->neq; i++ ) {
        engine->results->value[i][engine->iout] = engine->data->value[i];
      }
      for ( i=0; i<engine->data->model->nass; i++ ) {
        engine->results->avalue[i][engine->iout] = engine->data->avalue[i];
      }
      for ( i=0; i<engine->data->model->nconst; i++ ) {
        engine->results->pvalue[i][engine->iout] = engine->data->pvalue[i];
      }
    }
          
    /* check for steady state if set by commandline option -s */
    if ( engine->data->SteadyState == 1 ) {
      if ( checkSteadyState(engine->data) ) {
	engine->data->nout = engine->iout;
	engine->iout = engine->nout+1;
      }      
    }
    
    /* print immediately if data->PrintOnTheFly was set
       with '-d' or '--onthefly'
     */
    if ( engine->data->PrintOnTheFly ) {
      fprintf(engine->data->outfile, "%g ", engine->t + engine->data->t0);
      /* fprintf(stdout, "%g ", t + data->t0); */
      for ( i=0; i<engine->data->model->neq; i++ )
	fprintf(engine->data->outfile, "%g ", engine->data->value[i]);
      for ( i=0; i<engine->data->model->nass; i++ )
	fprintf(engine->data->outfile, "%g ", engine->data->avalue[i]);
      for ( i=0; i<engine->data->model->nconst; i++ )
	fprintf(engine->data->outfile, "%g ", engine->data->pvalue[i]);
      fprintf(engine->data->outfile, "\n");
    }
    else if ( engine->data->PrintMessage ) {
      const  char chars[5] = "|/-\\";
      fprintf(stderr, "\b\b\b\b\b\b");
      fprintf(stderr, "%.2f %c",
	      (float)engine->iout/(float)engine->nout,
	      chars[engine->iout % 4]);

    }

    engine->iout++;
    engine->tout += engine->tmult;

    return 1; /* continue integration */
}

/* standard handler for when the integrate function fails */
int IntegratorInstance_handleError(integratorInstance_t *engine)
{
    CvodeData data = engine->data;
    int i;
    int errorCode = SolverError_getLastCode(ERROR_ERROR_TYPE) ;

    if ( errorCode ) {
        
        /* on flag -6/CONV_FAILURE
        try again, but now with/without generated Jacobian matrix  */
        if ( errorCode == CONV_FAILURE && engine->data->run == 0 &&
	     engine->data->storeResults) {
            fprintf(
                stderr,
                "Trying again; now with %s Jacobian matrix\n",
                engine->data->UseJacobian ?
                    "CVODE's internal approximation of the" :
                    "automatically generated");
            engine->data->UseJacobian = !engine->data->UseJacobian;
            engine->data->run++;
            for ( i=0; i<engine->data->model->neq; i++ ) {
                engine->data->value[i] = engine->results->value[i][0];
            }
            for ( i=0; i<engine->data->model->nass; i++ ) {
                engine->data->avalue[i] = engine->results->avalue[i][0];
            }
            for ( i=0; i<engine->data->model->nconst; i++ ) {
                engine->data->pvalue[i] = engine->results->pvalue[i][0];
            }
            engine->data->currenttime = engine->data->t0;
            IntegratorInstance_freeExcludingCvodeData(engine);
            SolverError_clear();

            return integrator(data);
        }
        else
            SolverError_dumpAndClearErrors();
    }

    return errorCode ;
}

/**
   Prints some final statistics of the calls to CVODE routines, that
   are located in CVODE's iopt array.
*/
void IntegratorInstance_printStatistics(integratorInstance_t *engine)
{
    long int *iopt = engine->iopt;
    CvodeData data = engine->data;

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
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;

  /* evaluate ODEs */
  for ( i=0; i<data->model->neq; i++ ) {
    N_VIth(ydot,i) = evaluateAST(data->model->ode[i],data);
  } 

  /* update CvodeData */
  for ( i=0; i<data->model->neq; i++ ) {
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
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;

  /* evaluate Jacobian*/
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      DENSE_ELEM(J,i,j) = evaluateAST(data->model->jacob[i][j], data);
     }
  }
  
  /* update CvodeData */
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = N_VIth(y,i);
  }
  data->currenttime = t;
}

/*
  evaluates event trigger expressions and executes event assignments
  for those triggers that are true.  results are stored appropriately stored
  in
        engine->data->value
        engine->data->avalue
        engine->data->pvalue
        engine->y

  returns the number of triggers that fired.
*/

static int checkTrigger(integratorInstance_t *engine)
{  
    CvodeData data = engine->data;
    int i, j, k, fired;
    ASTNode_t *trigger, *assignment;
    Event_t *e;
    EventAssignment_t *ea;

    fired = 0;

    for ( i=0; i<Model_getNumEvents(data->model->simple); i++ ) {
        e = Model_getEvent(data->model->simple, i);
        trigger = (ASTNode_t *) Event_getTrigger(e);
        if ( data->trigger[i] == 0 && evaluateAST(trigger, data) ) {

            if (data->HaltOnEvent)
                SolverError_error(
                    ERROR_ERROR_TYPE, SOLVER_ERROR_EVENT_TRIGGER_FIRED,
                    "Event Trigger %d : %s fired at %g. Aborting simulation.",
                    i, SBML_formulaToString(trigger),
		    data->t0 + data->currenttime);

            fired++;
            data->trigger[i] = 1;      
            for ( j=0; j<Event_getNumEventAssignments(e); j++ ) {
                ea = Event_getEventAssignment(e, j);
                assignment = (ASTNode_t *) EventAssignment_getMath(ea);
                for ( k=0; k<data->model->neq; k++ ) {
                    if ( strcmp(EventAssignment_getVariable(ea),
                        data->model->species[k]) == 0 ) {
                            data->value[k] = evaluateAST(assignment, data);
                        }
                }
                for ( k=0; k<data->model->nass; k++ ) {
                    if ( strcmp(EventAssignment_getVariable(ea),
                        data->model->ass_parameter[k]) == 0 ) {
                            data->avalue[k] = evaluateAST(assignment, data);
                        }
                }
                for ( k=0; k<data->model->nconst; k++ ) {
                    if ( strcmp(EventAssignment_getVariable(ea),
                        data->model->parameter[k]) == 0 ) {
                            data->pvalue[k] = evaluateAST(assignment, data);
                        }
                }   
            }

        }
        else {
            data->trigger[i] = 0;
        }
    }

    return fired;

}

/**
  provisional identification of a steady state,
  evaluates mean and std of rates and returns 1 if a "steady state"
  is reached to stop  the calling integrator.
  This function is only called by the integrator function if specified
  via commandline options!
*/
/* NOTE: provisional steady state finding! Don't rely on that! */
static int checkSteadyState(CvodeData data){

  int i;
  
  /* calculate the mean and standard deviation of rates of change and
     store in CvodeData */
  data->dy_mean = 0.0;
  data->dy_var = 0.0;
  data->dy_std = 0.0;
  
  for ( i=0; i<data->model->neq; i++ ) {
    data->dy_mean += fabs(evaluateAST(data->model->ode[i],data));
  }
  data->dy_mean = data->dy_mean / data->model->neq;
  for ( i=0; i<data->model->neq; i++ ) {
    data->dy_var += SQR(evaluateAST(data->model->ode[i],data) - data->dy_mean);
  }
  data->dy_var = data->dy_var / (data->model->neq -1);
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
    for ( i=0; i<data->model->neq; i++ ) {
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->model->species[i],
	      fabs(evaluateAST(data->model->ode[i],data)));
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
