/*
  Last changed Time-stamp: <2005-08-31 23:00:05 xtof>
  $Id: integratorInstance.c,v 1.8 2005/09/01 15:31:04 chfl Exp $
*/

#include "sbmlsolver/integratorInstance.h"

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Header Files for CVODE */
#include "cvode.h"    
#include "cvdense.h"
#include "nvector_serial.h"  

#include "sbmlsolver/util.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"

static int
check_flag(void *flagvalue, char *funcname, int opt);

/**
 * CVode solver: function computing the ODE rhs for a given value
 * of the independent variable t and state vector y.
 */
static void
f(realtype t, N_Vector y, N_Vector ydot, void *f_data);

/**
 * CVode solver: function computing the dense Jacobian J of the ODE system
 * (or an approximation to it).
 */
static void
JacODE(long int N, DenseMat J, realtype t,
       N_Vector y, N_Vector fy, void *jac_data,
       N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3);
static int checkTrigger(integratorInstance_t *);
static int checkSteadyState(cvodeData_t *data);

SBML_ODESOLVER_API integratorInstance_t *
IntegratorInstance_create(odeModel_t *om, cvodeSettings_t *options)
{
    cvodeData_t *data =  CvodeData_createFromODEModel(om);
    
    RETURN_ON_FATALS_WITH(NULL)

    data->opt = options;
     
    if (options->Indefinitely)
    {
        data->tout = -1; /* delibrate trap for bugs - there is no
			    defined end time */
        data->nout = -1; /* delibrate trap for bugs - this is no
			    defined number of steps */
        data->tmult = options->Time;
    }
    else
    {
        data->tout  = options->Time;
        data->nout  = options->PrintStep;
        data->tmult = data->tout / data->nout;
    }


    options->StoreResults = !options->Indefinitely && options->StoreResults;
    /* allow setting of Jacobian,
       only if its construction was succesfull */
    options->UseJacobian = om->jacobian && options->UseJacobian;
    

    return IntegratorInstance_createFromCvodeData(data);
}

SBML_ODESOLVER_API void IntegratorInstance_copyVariableState(integratorInstance_t *target, integratorInstance_t *source)
{
    int i;
    cvodeData_t *targetData = target->data;
    cvodeData_t *sourceData = source->data;
    odeModel_t *model = targetData->model;

    if (model == sourceData->model)
    {
        for ( i=0; i<sourceData->nvalues; i++ )
            targetData->value[i] = sourceData->value[i];
    }
    else
        SolverError_error(
            ERROR_ERROR_TYPE,
            SOLVER_ERROR_ATTEMPTING_TO_COPY_VARIABLE_STATE_BETWEEN_INSTANCES_OF_DIFFERENT_MODELS,
            "Attempting to copy variable state between instances of "
	    "different models");
}

SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *ii, variableIndex_t *vi)
{
    return ii->data->value[vi->index];
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
    ii->data->value[vi->index] = value;
}

/* create structures in the integration engine for CVODE 
    return 1 => success
    return 0 => failure
*/
int IntegratorInstance_createODESolverStructures(integratorInstance_t *engine)
{
    int i, flag, neq;
    realtype *ydata, *abstoldata;
    cvodeData_t *data = engine->data;
    cvodeSettings_t *opt = data->opt;
    neq = data->model->neq; /* number of equations */

    /**
     * Allocate y, abstol vectors
     */
    engine->y = N_VNew_Serial(neq);
    if (check_flag((void *)engine->y, "N_VNew_Serial", 0)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector y failed");
      return 0; /* error */
    }
    engine->abstol = N_VNew_Serial(neq);
    if (check_flag((void *)engine->abstol, "N_VNew_Serial", 0)) {
      /* Memory allocation of vector abstol failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector abstol failed");
      return 0; /* error */
    }

    /**
     * Initialize y, abstol vectors
     */
    ydata      = NV_DATA_S(engine->y);
    abstoldata = NV_DATA_S(engine->abstol);
    for ( i=0; i<neq; i++ ) {
      /* Set initial value vector components of y and y' */
      ydata[i] = data->value[i];
      /* Set absolute tolerance vector components */ 
      abstoldata[i] = engine->atol1;       
    }

    /* Set the scalar relative tolerance */
    engine->reltol = engine->rtol1;                  

    /**
     * Call CVodeCreate to create the solver memory:
     *
     * CV_BDF     specifies the Backward Differentiation Formula
     * CV_NEWTON  specifies a Newton iteration
     */
    engine->cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);
    if (check_flag((void *)(engine->cvode_mem), "CVodeCreate", 0)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"CVodeCreate failed");
    }

    /**
     * Call CVodeMalloc to initialize the integrator memory:
     *
     * cvode_mem  pointer to the CVode memory block returned by CVodeCreate
     * f          user's right hand side function in y'=f(t,y)
     * t0         initial value of time
     * y          the initial dependent variable vector
     * CV_SV      specifies scalar relative and vector absolute tolerances
     * reltol     the scalar relative tolerance
     * abstol     pointer to the absolute tolerance vector
     */
    flag = CVodeMalloc(engine->cvode_mem, f, engine->t0, engine->y,
                       CV_SV, engine->reltol, engine->abstol);
    if (check_flag(&flag, "CVodeMalloc", 1)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"CVodeMalloc failed");
      return 0; /* error */
    }

    /**
     * Link the main integrator with data for right-hand side function
     */ 
    flag = CVodeSetFdata(engine->cvode_mem, engine->data);
    if (check_flag(&flag, "CVodeSetFdata", 1)) {
      /* ERROR HANDLING CODE if CVodeSetFdata failes */
    }
    
    /**
     * Link the main integrator with the CVDENSE linear solver
     */
    flag = CVDense(engine->cvode_mem, neq);
    if (check_flag(&flag, "CVDense", 1)) {
      /* ERROR HANDLING CODE if CVDense failes */
    }

    /**
     * Set the routine used by the CVDENSE linear solver
     * to approximate the Jacobian matrix to ...
     */
    if ( opt->UseJacobian == 1 ) {
      /* ... user-supplied routine Jac */
      flag = CVDenseSetJacFn(engine->cvode_mem, JacODE, engine->data);
    }
    else {
      /* ... the internal default difference quotient routine CVDenseDQJac */
      flag = CVDenseSetJacFn(engine->cvode_mem, NULL, NULL);
    }
    
    if ( check_flag(&flag, "CVDenseSetJacFn", 1) ) {
      /* ERROR HANDLING CODE if CVDenseSetJacFn failes */
    }

    /**
     * Set maximum number of internal steps to be taken
     * by the solver in its attempt to reach tout
     */
    CVodeSetMaxNumSteps(engine->cvode_mem, opt->Mxstep);

    return 1 ; /* OK */
}

/* frees stuff allocated for cvode */
void IntegratorInstance_freeODESolverStructures(integratorInstance_t *engine)
{
    /* Free the y, abstol vectors */ 
    N_VDestroy_Serial(engine->y);
    N_VDestroy_Serial(engine->abstol);

    /* Free the integrator memory */
    CVodeFree(engine->cvode_mem);
}

/* frees the integrator */
void IntegratorInstance_free(integratorInstance_t *engine)
{
    if (engine->data->model->neq && !engine->data->opt->EnableVariableChanges)
        IntegratorInstance_freeODESolverStructures(engine);

    CvodeData_freeExcludingModel(engine->data);
    free(engine);
}

void IntegratorInstance_freeExcludingCvodeData(integratorInstance_t *engine)
{
    if (engine->data->model->neq && !engine->data->opt->EnableVariableChanges)
        IntegratorInstance_freeODESolverStructures(engine);

    free(engine);
}

/* does all the stuff currently before the main loop in integrate()
   a NULL result indicates an error */
integratorInstance_t *IntegratorInstance_createFromCvodeData(cvodeData_t *data)
{
  int i;
  integratorInstance_t *engine;
  cvodeSettings_t *opt;
  
  ASSIGN_NEW_MEMORY(engine, integratorInstance_t, NULL);

  
 
  /* CVODE settings: set Problem Constants */
  /* set first output time, output intervals and number of outputs
     from the values in cvodeData_t *data */

  opt = data->opt;
  engine->data = data;

  engine->atol1 = opt->Error;  /* vector absolute tolerance components */ 
  engine->rtol1 = opt->RError; /* scalar relative tolerance */
  engine->t0 = 0.0;                 /* initial time           */
  engine->t1 = data->tmult;         /* first output time      */
  engine->tmult = engine->t1;       /* output time factor     */
  engine->nout = data->nout;        /* number of output steps */
  engine->t = 0;

  
  if (data->model->neq && !data->opt->EnableVariableChanges)
  {
      IntegratorInstance_createODESolverStructures(engine);
      RETURN_ON_ERRORS_WITH(NULL);
  }

  /*
    first, check if formulas can be evaluated, and cvodeData_t *
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<data->model->neq; i++ ) {
    evaluateAST(data->model->ode[i], data);
  }
  /* initialize assigned parameters */
  for ( i=0; i<data->model->nass; i++ ) {
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);
  }
  /*
    Now we should have all variables, and can allocate the
    results structure, where the time series will be stored
  */
  if ( data->results == NULL ) {
    engine->results = CvodeResults_create(data);
    RETURN_ON_FATALS_WITH(NULL);
    data->results = engine->results;
  }
  else {
    engine->results = data->results;
  }
  

  /* Writing initial conditions to results structure */

  if (data->opt->StoreResults)
  {
    engine->results->time[0] = data->t0;
    for ( i=0; i<data->nvalues; i++ ) {
      engine->results->value[i][0] = data->value[i];
    }
  }
 
  /* set up loop variables */
  engine->iout=1;
  engine->tout=engine->t1;

  return engine ;
}

int
IntegratorInstance_integrate(integratorInstance_t *engine) {

  while (!IntegratorInstance_timeCourseCompleted(engine)) {
    if (!IntegratorInstance_integrateOneStep(engine))
      return IntegratorInstance_handleError(engine);
  }
  return 0;
}

/* moves the current simulation one time step, returns 1 if
   the intergation can continue, 0 otherwise */
int IntegratorInstance_integrateOneStep(integratorInstance_t *engine)
{
    int i, flag;
    realtype *ydata;

    if (engine->data->model->neq)
    {
        if (engine->data->opt->EnableVariableChanges)
        {
            IntegratorInstance_createODESolverStructures(engine);
            RETURN_ON_ERRORS_WITH(0);
        }

        /* !! calling Cvode !! */
        flag = CVode(engine->cvode_mem, engine->tout,
                     engine->y, &(engine->t), CV_NORMAL);

        if ( flag != CV_SUCCESS )
        {
            char *message[] =
            {
              /*  0 CV_SUCCESS */
                "Success",
	      /**/
	      /*  1 CV_ROOT_RETURN */
              /*   "CVode succeeded, and found one or more roots" */
	      /*  2 CV_TSTOP_RETURN */
              /*   "CVode succeeded and returned at tstop" */
              /**/
              /* -1 CV_MEM_NULL -1 (old CVODE_NO_MEM) */
                "The cvode_mem argument was NULL",
              /* -2 CV_ILL_INPUT */
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
	      /* -3 CV_NO_MALLOC */
                "cvode_mem was not allocated",
              /* -4 CV_TOO_MUCH_WORK */
                "The solver took %g internal steps but could not "
		"compute variable values for time %g",
              /* -5 CV_TOO_MUCH_ACC */
                "The solver could not satisfy the accuracy " 
                "requested for some internal step.",
              /* -6 CV_ERR_FAILURE */
                "Error test failures occurred too many times "
                "during one internal time step or "
                "occurred with |h| = hmin.",
              /* -7 CV_CONV_FAILURE */
                "Convergence test failures occurred too many "
                "times during one internal time step or occurred "
		"with |h| = hmin.",
	      /* -8 CV_LINIT_FAIL */
                "CVode -- Initial Setup: "
		"The linear solver's init routine failed.",
              /* -9 CV_LSETUP_FAIL */
                "The linear solver's setup routine failed in an "
                "unrecoverable manner.",
              /* -10 CV_LSOLVE_FAIL */
                "The linear solver's solve routine failed in an "
                "unrecoverable manner.",
	      /* -11 CV_MEM_FAIL */
                "A memory allocation failed. "
                "(including an attempt to increase maxord)",
	      /* -12 CV_RTFUNC_NULL */
                "nrtfn > 0 but g = NULL.",
	      /* -13 CV_NO_SLDET */
                "CVodeGetNumStabLimOrderReds -- Illegal attempt "
		"to call without enabling SLDET.",
	      /* -14 CV_BAD_K */
                "CVodeGetDky -- Illegal value for k.",
	      /* -15 CV_BAD_T */
                "CVodeGetDky -- Illegal value for t.",
	      /* -16 CV_BAD_DKY */
                "CVodeGetDky -- dky = NULL illegal.",
	      /* -17 CV_PDATA_NULL */
                "???",
            };

            SolverError_error(
                ERROR_ERROR_TYPE,
                flag,
                message[flag * -1],
                engine->data->opt->Mxstep,
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

    /* update cvodeData_t **/
    engine->data->currenttime = engine->t;

    ydata = NV_DATA_S(engine->y);
    for ( i=0; i<engine->data->model->neq; i++ )
      engine->data->value[i] = ydata[i];
    
    /* should this be below the next for loop?? */
    if (engine->data->model->neq && engine->data->opt->EnableVariableChanges)
    {
        IntegratorInstance_freeODESolverStructures(engine);
        engine->t0 = engine->t;
    }

    for ( i=0; i<engine->data->model->nass; i++ )
      engine->data->value[engine->data->model->neq+i] =
	evaluateAST(engine->data->model->assignment[i], engine->data);

    if (checkTrigger(engine))
    {
        /* recalculate assignments - they may be dependent
	   on event assignment results */
        for ( i=0; i<engine->data->model->nass; i++ )
            engine->data->value[engine->data->model->neq+i] =
	      evaluateAST(engine->data->model->assignment[i], engine->data);

        if (engine->data->opt->HaltOnEvent) 
            return 0; /* stop integration */

        if (engine->data->model->neq &&
	    !engine->data->opt->EnableVariableChanges)
        {
            /* reset CVODE */
            IntegratorInstance_freeODESolverStructures(engine);
            engine->t0 = engine->t;
            IntegratorInstance_createODESolverStructures(engine);
            RETURN_ON_ERRORS_WITH(0);
        }
    }

    /* store results */
    if (engine->data->opt->StoreResults)
    {
      engine->results->nout = engine->iout;
      engine->results->time[engine->iout] = engine->t + engine->data->t0;
      for ( i=0; i<engine->data->nvalues; i++ ) {
        engine->results->value[i][engine->iout] = engine->data->value[i];
      }
    }
          
    /* check for steady state if set by commandline option -s */
    if ( engine->data->opt->SteadyState == 1 ) {
      if ( checkSteadyState(engine->data) ) {
	engine->data->nout = engine->iout;
	engine->iout = engine->nout+1;
      }
    }

    engine->iout++;
    engine->tout += engine->tmult;

    return 1; /* continue integration */
}

/* standard handler for when the integrate function fails */
int IntegratorInstance_handleError(integratorInstance_t *engine)
{
    int i;
    int errorCode = SolverError_getLastCode(ERROR_ERROR_TYPE) ;

    if ( errorCode ) {
        
        /* on flag -6/CV_CONV_FAILURE
        try again, but now with/without generated Jacobian matrix  */
        if ( errorCode == CV_CONV_FAILURE && engine->data->run == 0 &&
	     engine->data->opt->StoreResults) {
             engine->data->opt->UseJacobian = !engine->data->opt->UseJacobian;
            engine->data->run++;
            for ( i=0; i<engine->data->nvalues; i++ ) {
                engine->data->value[i] = engine->results->value[i][0];
            }

            engine->data->currenttime = engine->data->t0;
            IntegratorInstance_freeExcludingCvodeData(engine);
            SolverError_clear();

            return IntegratorInstance_integrate(engine);
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
    int flag;
    long int nst, nfe, nsetups, nje, nni, ncfn, netf;
    cvodeData_t *data = engine->data;

    flag = CVodeGetNumSteps(engine->cvode_mem, &nst);
    check_flag(&flag, "CVodeGetNumSteps", 1);
    CVodeGetNumRhsEvals(engine->cvode_mem, &nfe);
    check_flag(&flag, "CVodeGetNumRhsEvals", 1);
    flag = CVodeGetNumLinSolvSetups(engine->cvode_mem, &nsetups);
    check_flag(&flag, "CVodeGetNumLinSolvSetups", 1);
    flag = CVDenseGetNumJacEvals(engine->cvode_mem, &nje);
    check_flag(&flag, "CVDenseGetNumJacEvals", 1);
    flag = CVodeGetNonlinSolvStats(engine->cvode_mem, &nni, &ncfn);
    check_flag(&flag, "CVodeGetNonlinSolvStats", 1);
    flag = CVodeGetNumErrTestFails(engine->cvode_mem, &netf);
    check_flag(&flag, "CVodeGetNumErrTestFails", 1);

    fprintf(stderr, "\nIntegration Parameters:\n");
    fprintf(stderr, "mxstep   = %-6g rel.err. = %-6g abs.err. = %-6g \n",
	    data->opt->Mxstep, data->opt->RError, data->opt->Error);
    fprintf(stderr, "CVode Statistics:\n");
    fprintf(stderr, "nst = %-6ld nfe  = %-6ld nsetups = %-6ld nje = %ld\n",
	    nst, nfe, nsetups, nje); 
    fprintf(stderr, "nni = %-6ld ncfn = %-6ld netf = %ld\n\n",
	    nni, ncfn, netf);
}

/**
 * check return values of SUNDIALS functions
 */
static int
check_flag(void *flagvalue, char *funcname, int opt)
{

  int *errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL) {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n",
            funcname);
    return(1); }

  /* Check if flag < 0 */
  else if (opt == 1) {
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with flag = %d\n",
              funcname, *errflag);
      return(1); }}

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && flagvalue == NULL) {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n",
            funcname);
    return(1); }

  return(0);
}

/***************** Functions Called by the CVODE Solver ******************/

/**
   f routine. Compute f(t,y).
   This function is called by CVODE's integration routines every time
   needed.
   It evaluates the ODEs with the current species values, as supplied
   by CVODE's N_VIth(y,i) vector containing the values of all species.
   These values are first written back to CvodeData.
   Then every ODE is passed to processAST, together with the cvodeData_t *, and
   this function calculates the current value of the ODE.
   The returned value is written back to CVODE's N_VIth(ydot,i) vector that
   contains the values of the ODEs.
   The the cvodeData_t * is updated again with CVODE's internal values for
   species.
*/

static void f(realtype t, N_Vector y, N_Vector ydot, void *f_data)
{
  
  int i;
  realtype *ydata, *dydata;
  cvodeData_t *data;
  data   = (cvodeData_t *) f_data;
  ydata  = NV_DATA_S(y);
  dydata = NV_DATA_S(ydot);

  /* update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = ydata[i];
  }
  /* update assignment rules */
  for ( i=0; i<data->model->nass; i++ ) {
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);
  }
  /* update time  */
  data->currenttime = t;

  /* evaluate ODEs */
  for ( i=0; i<data->model->neq; i++ ) {
    dydata[i] = evaluateAST(data->model->ode[i],data);
  } 

}

/**
   Jacobian routine. Compute J(t,y).
   This function is (optionally) called by CVODE's integration routines
   every time needed.
   Very similar to the f routine, it evaluates the Jacobian matrix
   equations with CVODE's current values and writes the results
   back to CVODE's internal vector DENSE_ELEM(J,i,j).
*/

static void
JacODE(long int N, DenseMat J, realtype t,
       N_Vector y, N_Vector fy, void *jac_data,
       N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3)
{
  
  int i, j;
  realtype *ydata, *dydata;
  cvodeData_t *data;
  data  = (cvodeData_t *) jac_data;
  ydata = NV_DATA_S(y);
  
  /* update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = ydata[i];
  }
  /* update assignment rules */
  for ( i=0; i<data->model->nass; i++ ) {
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);
  }
  /* update time */
  data->currenttime = t;

  /* evaluate Jacobian*/
  for ( i=0; i<data->model->neq; i++ ) {
    for ( j=0; j<data->model->neq; j++ ) {
      DENSE_ELEM(J,i,j) = evaluateAST(data->model->jacob[i][j], data);
     }
  }
  
}

/*
  evaluates event trigger expressions and executes event assignments
  for those triggers that are true.  results are stored appropriately 
  in
        engine->data->value

  returns the number of triggers that fired.
*/

static int checkTrigger(integratorInstance_t *engine)
{  
    cvodeData_t *data = engine->data;
    int i, j, k, fired;
    ASTNode_t *trigger, *assignment;
    Event_t *e;
    EventAssignment_t *ea;

    fired = 0;

    for ( i=0; i<Model_getNumEvents(data->model->simple); i++ ) {
      e = Model_getEvent(data->model->simple, i);
      trigger = (ASTNode_t *) Event_getTrigger(e);
      if ( data->trigger[i] == 0 && evaluateAST(trigger, data) ) {

	if (data->opt->HaltOnEvent)
	  SolverError_error(ERROR_ERROR_TYPE, SOLVER_ERROR_EVENT_TRIGGER_FIRED,
			    "Event Trigger %d : %s fired at %g. "
			    "Aborting simulation.",
			    i, SBML_formulaToString(trigger),
			    data->t0 + data->currenttime);

	fired++;
	data->trigger[i] = 1;      
	for ( j=0; j<Event_getNumEventAssignments(e); j++ ) {
	  ea = Event_getEventAssignment(e, j);
	  assignment = (ASTNode_t *) EventAssignment_getMath(ea);
	  for ( k=0; k<data->nvalues; k++ ) {
	    if ( strcmp(EventAssignment_getVariable(ea),
			data->model->names[k]) == 0 ) {
	      data->value[k] = evaluateAST(assignment, data);
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
static int checkSteadyState(cvodeData_t *data){

  int i;
  double dy_mean, dy_var, dy_std;
  
  /* calculate the mean and standard deviation of rates of change and
     store in cvodeData_t * */
  dy_mean = 0.0;
  dy_var = 0.0;
  dy_std = 0.0;
  
  for ( i=0; i<data->model->neq; i++ ) {
    dy_mean += fabs(evaluateAST(data->model->ode[i],data));
  }
  dy_mean = dy_mean / data->model->neq;
  for ( i=0; i<data->model->neq; i++ ) {
    dy_var += SQR(evaluateAST(data->model->ode[i],data) - dy_mean);
  }
  dy_var = dy_var / (data->model->neq -1);
  dy_std = SQRT(dy_var);

  /* stop integrator if mean + std of rates of change are lower than
     1e-11 */
  if ( (dy_mean + dy_std) < 1e-11 ) {
    data->steadystate = 1;
    fprintf(stderr, "\n\n");
    fprintf(stderr,
	    "Steady state found. Simulation aborted at %g seconds\n\n",
	    data->currenttime);
    fprintf(stderr, "Rates at abortion:  \n");
    fprintf(stderr, "%g  ", data->currenttime);
    for ( i=0; i<data->model->neq; i++ ) {
      fprintf(stderr, "d[%s]/dt=%g  ",
	      data->model->names[i],
	      fabs(evaluateAST(data->model->ode[i],data)));
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Mean of rates:\n %g, std %g\n\n",
	    dy_mean, dy_std);
    fprintf(stderr, "\n");    
    return(1) ;
  }
  else {
    data->steadystate = 0;
    return(0);
  }
}
