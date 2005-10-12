/*
  Last changed Time-stamp: <2005-10-12 14:36:01 raim>
  $Id: integratorInstance.c,v 1.9 2005/10/12 12:52:08 raimc Exp $
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
 */
static void
JacODE(long int N, DenseMat J, realtype t,
       N_Vector y, N_Vector fy, void *jac_data,
       N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3);
static int checkTrigger(integratorInstance_t *);
static int checkSteadyState(integratorInstance_t *);
static integratorInstance_t *IntegratorInstance_allocate(cvodeData_t *data,
							 cvodeSettings_t *opt,
							 odeModel_t *om);



/** Creates an new integratorInstance:
    reads initial values from odeModel and integration settings from
    cvodeSettings to create integration data cvodeData and
    cvodeResults and initializes cvodeSolver structures.
*/

SBML_ODESOLVER_API integratorInstance_t *
IntegratorInstance_create(odeModel_t *om, cvodeSettings_t *opt)
{
  cvodeData_t *data;
  integratorInstance_t *engine;
    
  data = CvodeData_create(om);
  RETURN_ON_FATALS_WITH(NULL);

  CvodeData_initialize(data, opt, om);
  RETURN_ON_FATALS_WITH(NULL);
  
  engine = IntegratorInstance_allocate(data, opt, om);
  
  return engine;
}


/* allocate memory for a new integrator, initialize cvodeSolver
   structures from cvodeData, cvodeSettings and the odeModel */
static integratorInstance_t *IntegratorInstance_allocate(cvodeData_t *data,
							 cvodeSettings_t *opt,
							 odeModel_t *om)
{
  integratorInstance_t *engine;
    
  ASSIGN_NEW_MEMORY(engine, struct integratorInstance, NULL);
  ASSIGN_NEW_MEMORY(engine->cv, struct cvodeSolver, 0);

  data->run = 0;

  if (IntegratorInstance_initializeSolver(engine, data, opt, om))
    return engine;
  else
    return NULL;
}


/** Resets and existing integratorInstance with new settings.
    The instance can the be used for further integration runs
    with these new settings.
*/

SBML_ODESOLVER_API int
IntegratorInstance_set(integratorInstance_t *engine, cvodeSettings_t *opt)
{
  CvodeData_initialize(engine->data, opt, engine->om);
  RETURN_ON_FATALS_WITH(0);
  return IntegratorInstance_initializeSolver(engine,
					     engine->data, opt, engine->om);
}


/** Resets and integratorInstance to its initial values from the
    integratorInstance's cvodeSettings. After that, a new integration
    can be run.
*/

SBML_ODESOLVER_API int
IntegratorInstance_reset(integratorInstance_t *engine)
{
  return IntegratorInstance_set(engine, engine->opt);
}


/* initialize cvodeData from cvodeSettings and odeModel (could be
   separated in to functions to further support modularity and
   independence of data structures */
int
CvodeData_initialize(cvodeData_t *data, cvodeSettings_t *opt, odeModel_t *om)
{

  int i;
  Parameter_t *p;
  Species_t *s;
  Compartment_t *c;

  Model_t *ode = om->simple;

  /* data now also depends on cvodeSettings */
  data->opt = opt;
     
  if (opt->Indefinitely)
    {
      data->tout = -1; /* delibrate trap for bugs - there is no
			  defined end time */
      data->nout = -1; /* delibrate trap for bugs - this is no
			  defined number of steps */
      data->tmult = opt->Time;
    }
  else
    {
      data->tout  = opt->TimePoints[opt->PrintStep];
      data->nout  = opt->PrintStep;
      data->tmult = data->tout / data->nout;
    } 


  /* allow storage of results only for finite integrations */
  opt->StoreResults = !opt->Indefinitely && opt->StoreResults;
  
  /* allow setting of Jacobian,
     only if its construction was succesfull */
  opt->UseJacobian = om->jacobian && opt->UseJacobian;
    

  /*
    First, fill cvodeData_t  structure with data from
    the ODE model
  */

  for ( i=0; i<data->nvalues; i++ ) {
    if ( (s = Model_getSpeciesById(ode, om->names[i])) )
      data->value[i] = Species_getInitialConcentration(s);
    else if ( (c = Model_getCompartmentById(ode, om->names[i])) )
      data->value[i] = Compartment_getSize(c);
    else if ((p = Model_getParameterById(ode, om->names[i])) )
      data->value[i] = Parameter_getValue(p);
  }
  
  /*
    Then, check if formulas can be evaluated, and cvodeData_t *
    contains all necessary variables:
    evaluateAST(ASTNode_t *f, ,data) will
    ask the user for a value, if a a variable is unknown
  */
  for ( i=0; i<om->neq; i++ ) {
    evaluateAST(om->ode[i], data);
  }
  /* initialize assigned parameters */
  for ( i=0; i<om->nass; i++ ) {
    data->value[om->neq+i] =
      evaluateAST(om->assignment[i],data);
  }
  
  /*
    Now we should have all variables, and can allocate the
    results structure, where the time series will be stored ...
  */
  if ( opt->StoreResults ) {

    if ( !data->results )
      CvodeResults_free(data->results, data->nvalues);
    
    data->results = CvodeResults_create(data);
    RETURN_ON_FATALS_WITH(0);
  }
  
  return 1;
}


/** Copies variable and parameter values between two integratorInstances
    that have been created from the same odeModel
*/

SBML_ODESOLVER_API void
IntegratorInstance_copyVariableState(integratorInstance_t *target,
				     integratorInstance_t *source)
{
    int i;
    cvodeData_t *targetData = target->data;
    cvodeData_t *sourceData = source->data;
    odeModel_t *model = target->om;

    if (model == source->om)
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


/** Returns the current time of an integration
*/

SBML_ODESOLVER_API double
IntegratorInstance_getTime(integratorInstance_t *engine)
{
    return engine->cv->t + engine->data->t0 ;
}

/* not yet working, but would be nice for infinite integration runs */
void IntegratorInstance_setNextTimeStep(integratorInstance_t *engine,
					double nexttime)
{
    engine->cv->tout = nexttime;
}

/** Returns TRUE if the requested timecourse has been completed
    for the passed integratorInstance
*/

SBML_ODESOLVER_API int
IntegratorInstance_timeCourseCompleted(integratorInstance_t *engine)
{
    return engine->cv->iout > engine->cv->nout;
}

/** Gets the value of a variable or parameter during an integration
    via its variableIndex. The variableIndex can be retrieved from the
    odeModel with ODEModel_getVariable via the variable's or the
    parameter's ID symbol in the input SBML model (can be SBML
    compartments, species and parameters).
*/

SBML_ODESOLVER_API double
IntegratorInstance_getVariableValue(integratorInstance_t *engine,
				    variableIndex_t *vi)
{
    return engine->data->value[vi->index];
}


/**
   Sets the value of a variable or parameter during an integration via
   its variableIndex.This function also takes care of creating and
   freeing solver structures and ODE variables are changed!
   The variableIndex can be retrieved from the odeModel with
   ODEModel_getVariable via the variable's or the parameter's ID
   symbol in the input SBML model (can be SBML compartments, species
   and parameters).
*/

SBML_ODESOLVER_API void
IntegratorInstance_setVariableValue(integratorInstance_t *engine,
				    variableIndex_t *vi, double value)
{
  engine->data->value[vi->index] = value;

  if ( vi->index < engine->om->neq ) {
    IntegratorInstance_freeODESolverStructures(engine->cv);
    engine->cv->t0 = engine->cv->t;
    IntegratorInstance_createODESolverStructures(engine);  
  }
}


/* creates CVODE structures and fills cvodeSolver 
   return 1 => success
   return 0 => failure
*/
int
IntegratorInstance_createODESolverStructures(integratorInstance_t *engine)
{
    int i, flag, neq;
    realtype *ydata, *abstoldata;

    cvodeData_t *data = engine->data;
    cvodeSolver_t *cv = engine->cv;
    cvodeSettings_t *opt = engine->opt;

    neq = engine->om->neq; /* number of equations */

    /**
     * Allocate y, abstol vectors
     */
    cv->y = N_VNew_Serial(neq);
    if (check_flag((void *)cv->y, "N_VNew_Serial", 0)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector y failed");
      return 0; /* error */
    }
    cv->abstol = N_VNew_Serial(neq);
    if (check_flag((void *)cv->abstol, "N_VNew_Serial", 0)) {
      /* Memory allocation of vector abstol failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector abstol failed");
      return 0; /* error */
    }

    /**
     * Initialize y, abstol vectors
     */
    ydata      = NV_DATA_S(cv->y);
    abstoldata = NV_DATA_S(cv->abstol);
    for ( i=0; i<neq; i++ ) {
      /* Set initial value vector components of y and y' */
      ydata[i] = data->value[i];
      /* Set absolute tolerance vector components */ 
      abstoldata[i] = cv->atol1;       
    }

    /* Set the scalar relative tolerance */
    cv->reltol = cv->rtol1;                  

    /**
     * Call CVodeCreate to create the solver memory:
     *
     * CV_BDF     specifies the Backward Differentiation Formula
     * CV_NEWTON  specifies a Newton iteration
     */
    cv->cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);
    if (check_flag((void *)(cv->cvode_mem), "CVodeCreate", 0)) {
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
    flag = CVodeMalloc(cv->cvode_mem, f, cv->t0, cv->y,
                       CV_SV, cv->reltol, cv->abstol);
    if (check_flag(&flag, "CVodeMalloc", 1)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"CVodeMalloc failed");
      return 0; /* error */
    }

    /**
     * Link the main integrator with data for right-hand side function
     */ 
    flag = CVodeSetFdata(cv->cvode_mem, engine->data);
    if (check_flag(&flag, "CVodeSetFdata", 1)) {
      /* ERROR HANDLING CODE if CVodeSetFdata failes */
    }
    
    /**
     * Link the main integrator with the CVDENSE linear solver
     */
    flag = CVDense(cv->cvode_mem, neq);
    if (check_flag(&flag, "CVDense", 1)) {
      /* ERROR HANDLING CODE if CVDense failes */
    }

    /**
     * Set the routine used by the CVDENSE linear solver
     * to approximate the Jacobian matrix to ...
     */
    if ( opt->UseJacobian == 1 ) {
      /* ... user-supplied routine Jac */
      flag = CVDenseSetJacFn(cv->cvode_mem, JacODE, engine->data);
    }
    else {
      /* ... the internal default difference quotient routine CVDenseDQJac */
      flag = CVDenseSetJacFn(cv->cvode_mem, NULL, NULL);
    }
    
    if ( check_flag(&flag, "CVDenseSetJacFn", 1) ) {
      /* ERROR HANDLING CODE if CVDenseSetJacFn failes */
    }

    /**
     * Set maximum number of internal steps to be taken
     * by the solver in its attempt to reach tout
     */
    CVodeSetMaxNumSteps(cv->cvode_mem, opt->Mxstep);

    return 1; /* OK */
}

/* frees N_V vector structures, and the cvode_mem solver */
void IntegratorInstance_freeODESolverStructures(cvodeSolver_t *cv)
{
    /* Free the y, abstol vectors */ 
    N_VDestroy_Serial(cv->y);
    N_VDestroy_Serial(cv->abstol);

    /* Free the integrator memory */
    CVodeFree(cv->cvode_mem);
}


/** Frees an integratorInstance, including cvodeData
    but not the originating odeModel!
*/

SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *engine)
{
    if (engine->om->neq)
        IntegratorInstance_freeODESolverStructures(engine->cv);

    CvodeData_free(engine->data);
    free(engine->cv);
    free(engine);
}



/* initializes a cvodeSolver structure */
int IntegratorInstance_initializeSolver(integratorInstance_t *engine,
					cvodeData_t *data,
					cvodeSettings_t *opt, odeModel_t *om)
{
  
  cvodeSolver_t *cv = engine->cv;

  /* irreversible linking the engine to its input model */
  engine->om = om;

  /* joining option, data and result structures */
  engine->opt = opt;
  engine->data = data;
  engine->results = data->results;


  /* CVODE settings: set Problem Constants */
  /* set first output time, output intervals and number of outputs
     from the values in cvodeData_t *data */

  cv->atol1 = opt->Error;      /* vector absolute tolerance components */ 
  cv->rtol1 = opt->RError;     /* scalar relative tolerance */
  cv->t0 = 0.0;                /* initial time           */
  
  if ( opt->Indefinitely ) 
    cv->t1 = data->tmult;           /* first output time      */
  else 
    cv->t1 = opt->TimePoints[1];    /* first output time      */  
  
  cv->tmult = cv->t1;           /* first output time factor */         
  cv->nout = data->nout;        /* number of output steps */
  cv->t = 0.0;                  /* CVODE current time, always 0, when
				   starting from odeModel */

  /* count integration run with this instance */
  data->run++;

  if (om->neq)
  {
    /* SolverStructures from former runs must be freed */
    if ( data->run > 1 )
      IntegratorInstance_freeODESolverStructures(engine->cv);

    IntegratorInstance_createODESolverStructures(engine);
      RETURN_ON_ERRORS_WITH(0);
  }

  /* set up loop variables */
  cv->iout=1;         /* counts integration steps, start with 1 */
  
  /* first output time as passed to CVODE */
  if ( opt->Indefinitely )
    cv->tout = cv->t1;
  else 
    cv->tout = opt->TimePoints[1];
  
  return 1;
}


/** Start the default integration loop with standard error handling
*/

SBML_ODESOLVER_API int
IntegratorInstance_integrate(integratorInstance_t *engine) {

  while (!IntegratorInstance_timeCourseCompleted(engine)) {
    if (!IntegratorInstance_integrateOneStep(engine))
      return IntegratorInstance_handleError(engine);
  }
  return 0;
}

/** Creates and returns a cvodeResults structure containing
    the results of one integration run and NULL if not successful
*/

SBML_ODESOLVER_API cvodeResults_t *
IntegratorInstance_createResults(integratorInstance_t *engine) {

  int i, j;
  cvodeResults_t *results;

  cvodeSettings_t *opt = engine->opt;
  cvodeResults_t *iResults = engine->results;

  if ( !opt->StoreResults || iResults == NULL )
    return NULL;
  
  results = CvodeResults_create(engine->data);
  RETURN_ON_FATALS_WITH(0);

  results->nout = iResults->nout;

  for ( i=0; i < iResults->nout; i++ ) {
    results->time[i] = iResults->time[i];
    for ( j=0; j < iResults->nvalues; j++ )
      results->value[i][j] = iResults->value[i][j];
  }

  return results;  
}


/** The Hot Stuff!
   Calls CVODE to move the current simulation one time step; produces
   appropriate error messages on failures and returns 1 if the
   integration can continue, 0 otherwise.  The function also checks
   for events and steady states and stores results if requested by
   cvodeSettings.  It also handles models without ODEs (only
   assignment rules or constant parameters).
*/

SBML_ODESOLVER_API int
IntegratorInstance_integrateOneStep(integratorInstance_t *engine)
{
    int i, flag;
    realtype *ydata = NULL;
    
    cvodeSolver_t *cv = engine->cv;
    cvodeData_t *data = engine->data;
    cvodeSettings_t *opt = engine->opt;
    cvodeResults_t *results = engine->results;
    odeModel_t *om = engine->om;
    
    /* At first integration step, write initial conditions to results
       structure */
    if ( cv->iout == 1 && opt->StoreResults ) {
      data->results->time[0] = 0.0;
      for ( i=0; i<data->nvalues; i++ ) 
	data->results->value[i][0] = data->value[i];
    }
    
    if (om->neq)
    {

        /* !! calling Cvode !! */
        flag = CVode(cv->cvode_mem, cv->tout,
                     cv->y, &(cv->t), CV_NORMAL);

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
                opt->Mxstep,
                cv->tout);
            SolverError_error(
                WARNING_ERROR_TYPE,
                SOLVER_ERROR_INTEGRATION_NOT_SUCCESSFUL,
                "Integration not successful. Results are not complete.");

	        return 0 ; /* Error - stop integration*/
        }
	ydata = NV_DATA_S(cv->y);
    }
    else
    {
        /* faking a timestep when we don't have ODEs */
        cv->t = cv->tout ;
    }

    /* update cvodeData_t **/
    data->currenttime = cv->t;
   
    for ( i=0; i<om->neq; i++ )
      data->value[i] = ydata[i];

    for ( i=0; i<om->nass; i++ )
      data->value[om->neq+i] =
	evaluateAST(om->assignment[i], data);

    /* check for event triggers and evaluate the triggered
       events' assignments;
       stop integration if requested by cvodeSettings */
    if ( checkTrigger(engine) )
    {
        /* recalculate assignments - they may be dependent
	   on event assignment results */
        for ( i=0; i<om->nass; i++ )
            data->value[om->neq+i] =
	      evaluateAST(om->assignment[i], data);

        if (opt->HaltOnEvent) 
            return 0; /* stop integration */
    }

    /* store results */
    if (opt->StoreResults)
    {
      results->nout = cv->iout;
      results->time[cv->iout] = cv->t + data->t0;
      for ( i=0; i<data->nvalues; i++ ) {
        results->value[i][cv->iout] = data->value[i];
      }
    }
          
    /* check for steady state if requested by cvodeSettings */
    if ( opt->SteadyState == 1 ) {
      if ( checkSteadyState(engine) ) {
	data->nout = cv->iout;
	cv->iout = cv->nout+1;
      }
    }

    /* increase integration step counter */
    cv->iout++;
    /* ... and set next output time */
    if ( opt->Indefinitely )
      cv->tout += cv->tmult;
    else if ( cv->iout <= cv->nout )
      cv->tout = opt->TimePoints[cv->iout];
    
    return 1; /* continue integration */
}


/** Standard handler for when the integrate function fails.
    WARNING: RERUN CURRENTLY DOESN't WORK 
*/

SBML_ODESOLVER_API int
IntegratorInstance_handleError(integratorInstance_t *engine)
{
/*   int i; */
  int errorCode = SolverError_getLastCode(ERROR_ERROR_TYPE) ;
  cvodeData_t *data = engine->data;
  cvodeSettings_t *opt = engine->opt;
  
  if ( errorCode ) {
        
    /* on flag -6/CV_CONV_FAILURE
       try again, but now with/without generated Jacobian matrix  */
    if ( errorCode == CV_CONV_FAILURE && data->run == 1 &&
	 opt->StoreResults) {
      
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_MESSAGE_RERUN_WITH_OR_WO_JACOBIAN,
			"Rerun with %s Jacobian matrix.",
			opt->UseJacobian ?
			"CVODE's internal approximation of the" :
			"automatically generated");

      /* integrate again */
      opt->UseJacobian = !opt->UseJacobian;
      IntegratorInstance_reset(engine);
      return IntegratorInstance_integrate(engine);
    }
    else
      SolverError_dumpAndClearErrors();
  }

  return errorCode ;
}


/** Prints some final statistics of the calls to CVODE routines, that
    are located in CVODE's iopt array.
*/
SBML_ODESOLVER_API void
IntegratorInstance_printStatistics(integratorInstance_t *engine)
{
    int flag;
    long int nst, nfe, nsetups, nje, nni, ncfn, netf;

    cvodeSettings_t *opt = engine->opt;
    cvodeSolver_t *cv = engine->cv;

    flag = CVodeGetNumSteps(cv->cvode_mem, &nst);
    check_flag(&flag, "CVodeGetNumSteps", 1);
    CVodeGetNumRhsEvals(cv->cvode_mem, &nfe);
    check_flag(&flag, "CVodeGetNumRhsEvals", 1);
    flag = CVodeGetNumLinSolvSetups(cv->cvode_mem, &nsetups);
    check_flag(&flag, "CVodeGetNumLinSolvSetups", 1);
    flag = CVDenseGetNumJacEvals(cv->cvode_mem, &nje);
    check_flag(&flag, "CVDenseGetNumJacEvals", 1);
    flag = CVodeGetNonlinSolvStats(cv->cvode_mem, &nni, &ncfn);
    check_flag(&flag, "CVodeGetNonlinSolvStats", 1);
    flag = CVodeGetNumErrTestFails(cv->cvode_mem, &netf);
    check_flag(&flag, "CVodeGetNumErrTestFails", 1);

    fprintf(stderr, "\nIntegration Parameters:\n");
    fprintf(stderr, "mxstep   = %-6g rel.err. = %-6g abs.err. = %-6g \n",
	    opt->Mxstep, opt->RError, opt->Error);
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
   It evaluates the ODEs with the current variable values, as supplied
   by CVODE's N_VIth(y,i) vector containing the values of all variables.
   These values are first written back to CvodeData.
   Then every ODE is passed to processAST, together with the cvodeData_t *,
   and this function calculates the current value of the ODE.
   The returned value is written back to CVODE's N_VIth(ydot,i) vector that
   contains the values of the ODEs.
   The the cvodeData_t * is updated again with CVODE's internal values for
   all variables.
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
  realtype *ydata;
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

/************* Internal Checks During Integration Step *******************/
/**
   evaluates event trigger expressions and executes event assignments
   for those triggers that are true. Results are stored appropriately
   in
        engine->data->value.

   Returns the number of triggers that fired.
*/

static int checkTrigger(integratorInstance_t *engine)
{  
    int i, j, fired;
    ASTNode_t *trigger, *assignment;
    Event_t *e;
    EventAssignment_t *ea;
    variableIndex_t *vi;

    cvodeSettings_t *opt = engine->opt;
    cvodeData_t *data = engine->data;
    odeModel_t *om = engine->om;

    fired = 0;

    for ( i=0; i<Model_getNumEvents(om->simple); i++ ) {
      e = Model_getEvent(om->simple, i);
      trigger = (ASTNode_t *) Event_getTrigger(e);
      if ( data->trigger[i] == 0 && evaluateAST(trigger, data) ) {

	if (opt->HaltOnEvent)
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
	  vi = ODEModel_getVariableIndex(om,
					 EventAssignment_getVariable(ea));
	  IntegratorInstance_setVariableValue(engine, vi,
					      evaluateAST(assignment, data));
	  VariableIndex_free(vi);
	}
      }
      else {
	data->trigger[i] = 0;
      }
    }

    return fired;

}

/** provisional identification of a steady state,
    evaluates mean and std of rates and returns 1 if a "steady state"
    is reached to stop  the calling integrator.
    This function is only called by the integrator function if specified
    via commandline options!
*/
/* NOTE: provisional steady state finding! */
static int checkSteadyState(integratorInstance_t *engine) {

  int i;
  double dy_mean, dy_var, dy_std;
  cvodeData_t *data = engine->data;
  odeModel_t *om = engine->om;
  
  /* calculate the mean and standard deviation of rates of change and
     store in cvodeData_t * */
  dy_mean = 0.0;
  dy_var = 0.0;
  dy_std = 0.0;
  
  for ( i=0; i<om->neq; i++ ) {
    dy_mean += fabs(evaluateAST(om->ode[i],data));
  }
  dy_mean = dy_mean / om->neq;
  for ( i=0; i<om->neq; i++ ) {
    dy_var += SQR(evaluateAST(om->ode[i],data) - dy_mean);
  }
  dy_var = dy_var / (om->neq -1);
  dy_std = SQRT(dy_var);

  /* stop integrator if mean + std of rates of change are lower than
     1e-11 */
  if ( (dy_mean + dy_std) < 1e-11 ) {
    data->steadystate = 1;
    SolverError_error(WARNING_ERROR_TYPE,
		       SOLVER_MESSAGE_STEADYSTATE_FOUND,
		       "Steady state found. "
		       "Simulation aborted at %g seconds. "
		       "Mean of rates: %g, std %g",
		      data->currenttime, dy_mean, dy_std);
    return(1) ;
  }
  else {
    data->steadystate = 0;
    return(0);
  }
}
