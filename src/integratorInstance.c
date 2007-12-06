/*
  Last changed Time-stamp: <2007-12-06 20:17:36 raim>
  $Id: integratorInstance.c,v 1.98 2007/12/06 19:22:05 raimc Exp $
*/
/* 
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but
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
 *     Andrew M. Finney
 *     Christoph Flamm
 */

/*! \defgroup integration Numerical Analysis */
/*! \defgroup integrator ODE/DAE Integrator Interface
  \ingroup integration
  This module contains all interfaces to an integratorInstance
    
  SOSlib allows to get and set model data during an integration run.
  This groups contain all functions available for this purpose.
*/
/*@{*/

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cvodes/cvodes.h>
#include <kinsol/kinsol.h>
#include <cvodes/cvodes_dense.h>
#include <nvector/nvector_serial.h>

#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"

#include "sbmlsolver/cvodeData.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/cvodeSolver.h"
#include "sbmlsolver/sensSolver.h"
#include "sbmlsolver/modelSimplify.h"
/* #include "sbmlsolver/daeSolver.h" */ /* !!! not yet working !!! */


/* local integratorInstance allocation and initialization */ 
static int
IntegratorInstance_initializeSolver(integratorInstance_t *, cvodeData_t *,
				    cvodeSettings_t *, odeModel_t *);
static odeSense_t *
IntegratorInstance_initializeSensitivity(odeModel_t *, cvodeSettings_t *,
					 odeSense_t *);
static int
IntegratorInstance_initializeJacobian(odeModel_t *, cvodeSettings_t *);

/* the following functions contain solver specific switches */
static integratorInstance_t *
IntegratorInstance_allocate(cvodeData_t *, cvodeSettings_t *, odeModel_t *);

/* handles event executions */
static int
IntegratorInstance_processEventsAndAssignments(integratorInstance_t *);


/***************** functions common to all solvers ************************/

/** Creates an new integratorInstance
     
reads initial values from odeModel and integration settings from
cvodeSettings to create integration data cvodeData and
cvodeResults and initializes cvodeSolver structures.
*/

SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *om, cvodeSettings_t *opt)
{
  cvodeData_t *data;

  data = CvodeData_create(om);
  RETURN_ON_FATALS_WITH(NULL);
 
  CvodeData_initialize(data, opt, om);
  RETURN_ON_FATALS_WITH(NULL);

  return IntegratorInstance_allocate(data, opt, om);      
}

/** Resets an existing integratorInstance to time 0 and the
    input SBML model's initial conditions with new settings.

    Returns 1 if succesfull, 0 otherwise.
    The instance can then be used for further integration runs
    with these new settings. Don't use this function
    during an integration run!
*/

SBML_ODESOLVER_API int IntegratorInstance_set(integratorInstance_t *engine, cvodeSettings_t *opt)
{
  CvodeData_initialize(engine->data, opt, engine->om);  
  RETURN_ON_FATALS_WITH(0);
  /* de-activate backward integration for adjoint solver */
  engine->AdjointPhase = 0;
  
  return IntegratorInstance_initializeSolver(engine, engine->data,
					     opt, engine->om);
}


/**  Resets an existing integratorInstance to time 0 and the
     input SBML model's initial conditions with original settings

     Returns 1 if succesfull, 0 otherwise.
     After that, a new integration can be run. Don't use during an
     integration run!
*/

SBML_ODESOLVER_API int IntegratorInstance_reset(integratorInstance_t *engine)
{
  return IntegratorInstance_set(engine, engine->opt);
}

/** Resets integrator for running the backward phase of the adjoint solver.

    Returns 1 if succesfull, 0 otherwise.
    The backward phase can then be run as usual via
    IntegratorInstance_integrate* functions.
*/

SBML_ODESOLVER_API int IntegratorInstance_resetAdjPhase(integratorInstance_t *engine)
{
  /* activate backward integration for adjoint solver */
  engine->AdjointPhase = 1;
  return IntegratorInstance_initializeSolver(engine, engine->data,
					      engine->opt, engine->om);
}


/* allocate memory for a new integrator, initialize cvodeSolver
   structures from cvodeData, cvodeSettings and the odeModel */
static integratorInstance_t *IntegratorInstance_allocate(cvodeData_t *data,
							 cvodeSettings_t *opt,
							 odeModel_t *om)
{
  integratorInstance_t *engine;

  ASSIGN_NEW_MEMORY(engine, struct integratorInstance, NULL);
  ASSIGN_NEW_MEMORY(engine->solver, struct cvodeSolver, 0);

  /* set integration run counter to 0,
     used for multiple reruns of integration  */
  engine->run = 0;

  /* set adjoint run counter to 0 */
  engine->adjrun = 0;
  
  /* initialize adjoint phase flag */
  engine->AdjointPhase = 0;

  engine->solver->cvode_mem = NULL;
  engine->solver->y = NULL;
  engine->solver->abstol = NULL;

  /* set state variable vector y to NULL for first run */
  engine->solver->y = NULL;
  engine->solver->cvode_mem = NULL;
  engine->solver->abstol = NULL;
  engine->solver->q = NULL;

  /* set sensitivity structure to NULL */
  engine->solver->yS = NULL;
  engine->solver->senstol = NULL;
  engine->solver->qS = NULL;
  /* set IDA structure to NULL */
  engine->solver->dy = NULL;
  /* set adjoint sensitivity structures to NULL */
  engine->solver->cvadj_mem = NULL;
  engine->solver->yA = NULL;
  engine->solver->qA = NULL;
  engine->solver->abstolA = NULL;
  engine->solver->abstolQA = NULL;

  engine->os = NULL;
/*   engine->solver->nsens = 0; */
  engine->os = NULL;

  if (IntegratorInstance_initializeSolver(engine, data, opt, om))
    return engine;
  else
    return NULL;
}

/* construct new sensitivities */
static
odeSense_t *IntegratorInstance_initializeSensitivity(odeModel_t *om,
						     cvodeSettings_t *opt,
						     odeSense_t *os) 
{
  int i, changed = 0;

  /* SENSITIVITIES */
  /* if no sensitivity structures are present yet, construct and return*/
  if ( os == NULL )
    return ODESense_create(om, opt);

  /* if sensitivity structures are already present check whether
     they need reconstruction */
  else
  {
    if ( os->index_sens != NULL  )
    {
      /* compare with options */	  
      if ( opt->sensIDs != NULL )
      {
	if ( opt->nsens != os->nsens ) changed++;
	else
	  for ( i=0; i<os->nsens; i++ )
	    if ( os->index_sens[i] !=
		 ODEModel_getVariableIndexFields(om, opt->sensIDs[i]) )
	      changed++;
      }
      /* compare with constants (default) */
      else 
      {
	if ( os->nsens != om->nconst ) changed++;
	else
	  for ( i=0; i<os->nsens; i++ )
	    if ( os->index_sens[i] !=  (om->neq+om->nass+om->nalg+i) )
	      changed++;
      }
    }
  }  
  
  /* free old and reconstruct sensitivities */
  if ( changed )
  {
     /* free old and ... */
    ODESense_free(os);
    /* ... construct new sensitivity */
    return ODESense_create(om, opt);    
  }
  else
  {
    /* odeSense model exists and hasn't changed wrt to input options */
    return os;
  }
}

/* The function compares diverse flags and settings whose combination
   indicates whether the solver should and can use the analytic
   Jacobian matrix. It also constructs the matrix when requested the
   first time. */
static int IntegratorInstance_initializeJacobian(odeModel_t *om,
						  cvodeSettings_t *opt)
{
    /* JACOBIAN */
  /* had jacobian matrix construction already failed in previous tries? */
  if ( om->jacobianFailed > 0 )
    return 0;
  /* ... is jacobian not wanted? */
  else if ( !opt->UseJacobian )
    return 0;
  /* ... is jacobian requested and already constructed ? */
  else if ( opt->UseJacobian && om->jacob != NULL )
    return 1;

  /* default: construct matrix,
     returns 1 if matrix construction is succesful, 0 otherwise  */
  return ODEModel_constructJacobian(om);
}

/* initializes the solver initial time setup with odeModel,
   cvodeSettings and cvodeData structures;
   these settings are supposed to be required for all different solvers;
   solver specific settings are initialized in create`SOLVER'Structures;
   NOTE that currently the adjoint phase requires specific initializations
   here: maybe this can be removed by using the same structures in
   cvodeSettings, as initially proposed by James !!?? */
static int IntegratorInstance_initializeSolver(integratorInstance_t *engine,
					       cvodeData_t *data,
					       cvodeSettings_t *opt,
					       odeModel_t *om)
{
  int i;
  cvodeSolver_t *solver = engine->solver;
  cvodeResults_t *results = data->results;
 
  /* irreversibly linking the engine to its input model */
  engine->om = om;

  /* joining option, data and result structures */
  engine->opt = opt;
  engine->data = data;
  engine->results = data->results;
  
  /* construct Jacobian matrix, if not yet existing */
  engine->UseJacobian = IntegratorInstance_initializeJacobian(engine->om, opt);
  RETURN_ON_FATALS_WITH(0);
  
  /* construct sensitivities, if not yet existing */
  /*!!! BETTER ERROR HANDLING?? !!!*/
  if ( (opt->Sensitivity || opt->DoAdjoint) && !engine->AdjointPhase )
  {
    engine->os = IntegratorInstance_initializeSensitivity(engine->om, opt,
							  engine->os);
    RETURN_ON_FATALS_WITH(0);
    if ( engine->os )
      CvodeData_initializeSensitivities(data, opt, engine->om, engine->os); 
  }
 
  
  /* initialize the solver's time settings */  
  if ( !engine->AdjointPhase )
  {
    /* set initial time, first output time and number of time steps */
    solver->t0 = opt->TimePoints[0];      /* initial time           */
  
    /* first output time as passed to CVODE */
    if ( opt->Indefinitely )
      solver->tout = opt->Time;      
    else 
      solver->tout = opt->TimePoints[1];

    solver->nout = opt->PrintStep;     /* number of output steps */
    solver->t = opt->TimePoints[0];   /* CVODE current time, always 0,
					 when starting from odeModel */
    /* set up loop variables */
    solver->iout=1;        /* counts integration steps, start with 1 */

 
    /* write initial conditions to results structure */
    if ( opt->StoreResults )
    {
      results->time[0] = data->currenttime;
      for ( i=0; i<data->nvalues; i++ )
	results->value[i][0] = data->value[i];
    }

    /* count integration runs with this integratorInstance */
    engine->run++;

  }
  else    
  {
    /* Adjoint Phase */
    solver->t0 = opt->AdjTimePoints[0]; 
    solver->tout = opt->AdjTimePoints[1]; 
    solver->nout = opt->AdjPrintStep;     /* number of adjoint output steps */
    solver->t = opt->AdjTimePoints[0];   
    /* set up loop variables */
    solver->iout=1;  


    /* write adjoint initial conditions to results structure */
    /* Need to look into modifying data values? */
    if ( opt->AdjStoreResults )
    {    
      for ( i=0; i<data->neq; i++ )
	results->adjvalue[i][0] = data->adjvalue[i];
    }

    /* count adjoint integration runs with this integratorInstance */
    engine->adjrun++;
  }

  /* set flag to 0 to indicate that solver structures need to be
     created before integration */
  engine->isValid = 0;

  /* reset integrator clock */
  engine->clockStarted = 0;
  
  return 1;  
}


/**  Returns the settings of this integratorInstance.

     These settings can then be changed with cvodeSettings interface
     functions. The changes become only effective after
     IntegratorInstance_reset has been called. Don't use during an
     integration run!
*/

SBML_ODESOLVER_API cvodeSettings_t *IntegratorInstance_getSettings(integratorInstance_t *engine)
{
  return engine->opt;
}



/**  Copies current time, variable and parameter values between two
     integratorInstances (source->target) which
     have been created from the same odeModel

     WARNING: does NOT copy the time of the other integrator! use
     IntegratorInstace_setInitialTime and/or IntegratorInstance_setNextTimeStep
     to also set the time of the target integrator
*/

SBML_ODESOLVER_API void IntegratorInstance_copyVariableState(integratorInstance_t *target, integratorInstance_t *source)
{
  int i;
  cvodeData_t *targetData = target->data;
  cvodeData_t *sourceData = source->data;
  odeModel_t *om = target->om;

  if ( om == source->om )
    for ( i=0; i<sourceData->nvalues; i++ )
      targetData->value[i] = sourceData->value[i];
  else
    SolverError_error(
		      ERROR_ERROR_TYPE,
		      SOLVER_ERROR_ATTEMPTING_TO_COPY_VARIABLE_STATE_BETWEEN_INSTANCES_OF_DIFFERENT_MODELS,
		      "Attempting to copy variable state between instances of "
		      "different models");

  /* reset if the integrator had already been run */
  if ( target->isValid )
  {
    /* set engine to invalid to cause reinitialization of solver */
    target->isValid = 0;

    /* and finally assignment rules, potentially depending on that variable
       but otherwise rarely executed need to be evaluated */
    /*!!! this could use only dependent assignments ? */
    for ( i=0; i<om->nass; i++ )
      targetData->value[om->neq+i] =
	evaluateAST(om->assignment[i], targetData);
    
    /* optimize ODEs for evaluation again */
    IntegratorInstance_optimizeOdes(target);    
  }
}


/**  Returns the current time of an integration */

SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *engine)
{
  return engine->solver->t;
}

/** Sets the initial time of the solver, see WARNING below

    WARNING:
    This function can only be used for fresh or reset integrators!
    Use with care, i.e. only when you know what you are doing!
*/
/*!!! needs testing and consideration of possible pitfalls */
SBML_ODESOLVER_API int IntegratorInstance_setInitialTime(integratorInstance_t *engine, double initialtime)
{
  if ( (engine->isValid == 0 &&	
	engine->solver->t == engine->solver->t0) &&
       engine->solver->tout > initialtime )
  {
    engine->solver->t0 = initialtime;
    engine->solver->t  = initialtime;
    engine->data->currenttime = initialtime;
    return 1;
  }
  else
    SolverError_error(ERROR_ERROR_TYPE,
		      SOLVER_ERROR_ATTEMPTING_TO_SET_IMPOSSIBLE_INITIAL_TIME,
		      "Requested intial time (%f) is not possible! "\
		      "Reset integrator first, and make sure that the first "\
		      "output time (%f) is smaller then the requested "\
		      "initial time!", initialtime,
		      engine->solver->tout);
  return 0;
}

/** Sets the next output time for infinite integration

    This function can only be used for infinite integration
    (CvodeSettings_setIndefinite(set, 1)).
    Returns 1 if successful and 0 otherwise.

    WARNING: the next output time must always be bigger then the previous.
*/

SBML_ODESOLVER_API int IntegratorInstance_setNextTimeStep(integratorInstance_t *engine, double nexttime)
{
  if ( engine->opt->Indefinitely )
  {
    engine->solver->tout = nexttime;
    return 1;
  }
  return 0;
}


/**  Gets the value of a variable or parameter during an integration
     via its variableIndex.

     The variableIndex can be retrieved from the odeModel with
     ODEModel_getVariable via the variable's or the parameter's ID
     symbol in the input SBML model (can be SBML compartments, species
     and parameters).
*/

SBML_ODESOLVER_API double IntegratorInstance_getVariableValue(integratorInstance_t *engine, variableIndex_t *vi)
{
  return engine->data->value[vi->index];
}


/** Gets the sensitivity of variable y to parameter or variable s at
    the current time.

    Returns 0 if no sensitivity has been calculated for s!
*/

SBML_ODESOLVER_API double IntegratorInstance_getSensitivity(integratorInstance_t *engine,  variableIndex_t *y,  variableIndex_t *s)
{
  int i;
  
  if ( y->index >= engine->om->neq )
  {
    printf("Warning: ID is not a variable, no sensitivities ");
    printf("can be calculated for %s \n", engine->om->names[y->index]);
    return 0.0;
  }
  /* find sensitivity for s */
  for ( i=0; i<engine->os->nsens &&
	  !(engine->os->index_sens[i] == s->index); i++ );

  if ( i == engine->os->nsens ) return 0.0;
  else return engine->data->sensitivity[y->index][i];
}


/**  Prints variable names, the first value is the time,

ODE variable values, assigned variable values and
constant values follow. The order is the same
as in IntegratorInstance_dumpData.
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpNames(integratorInstance_t *engine)
{
  ODEModel_dumpNames(engine->om);
}


/**  Prints the current integration data,

the first value is the current time, ODE variable values, assigned
variable values and constant values follow. The order is the same
as in IntegratorInstance_dumpNames.
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpData(integratorInstance_t *engine)
{
  int i;
  cvodeData_t *data = engine->data;

  printf("%g  ", data->currenttime);
  for ( i=0; i<data->nvalues; i++ )
    printf("%g ", data->value[i]);
  printf("\n");
}

/**  Prints the current adjoint integration data,
     The first value is the current time, followed by adjoint
     ODE variable values.
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpAdjData(integratorInstance_t *engine)
{
  int i;
  cvodeData_t *data = engine->data;

  printf("%g  ", data->currenttime);
  for ( i=0; i<data->neq; i++ )
    printf("%g ", data->adjvalue[i]);
  printf("\n");
}


/**  Prints the current time, current value of variable y and 
     sensitivities to all parameters for which calculated
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpYSensitivities(integratorInstance_t *engine, variableIndex_t *y)
{
  int i;
  cvodeData_t *data = engine->data;

  if ( data->sensitivity == NULL ) return;
  if ( y->index >= engine->om->neq )
  {
    printf("Warning: ID is not a variable, no sensitivities ");
    printf("can be calculated for %s \n", engine->om->names[y->index]);
    return;
  }
  
  printf("%g  ", data->currenttime);
  printf("%g  ", data->value[y->index]);
  for ( i=0; i<data->nsens; i++ )
    printf("%g ", data->sensitivity[y->index][i]);
  printf("\n");
}


/**  Prints the current time, the value of parameter p and 
     all variable's sensitivities Si to p, where
     i = 1 to NEQ
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpPSensitivities(integratorInstance_t *engine, variableIndex_t *p)
{
  int i, j;
  cvodeData_t *data = engine->data;
  odeSense_t *os = engine->os;
  odeModel_t *om = engine->om;
  
  if ( data->sensitivity == NULL ) return;

  /* find sensitivity for p */
  for ( i=0; i<os->nsens && !(os->index_sens[i] == p->index); i++ );

  if ( i == os->nsens )
    printf("Warning: no sensitivity requested for ID %s\n",
	   om->names[p->index]);
  else
  {
    printf("%g  ", data->currenttime);
    for ( j=0; j<data->neq; j++ )
      printf("%g ", data->sensitivity[j][i]);
    printf("\n");
  }
}


/**  Returns the name of sensitivity variable i
*/
SBML_ODESOLVER_API char* IntegratorInstance_getSensVariableName(integratorInstance_t *engine, int i)
{
  if (i > engine->os->nsens || !engine->opt->sensIDs )
    return NULL;

  return engine->om->names[engine->os->index_sens[i]];
}


/** Returns a pointer cvodeData of the integratorInstance, which contains
    the current values of all variables. This structure is `read-only'
    and can be used for evaluation of formulas with evaluateAST after
    integration. Ownership stays with the integratorInstance.
*/

SBML_ODESOLVER_API cvodeData_t *IntegratorInstance_getData(integratorInstance_t *engine)
{
  return engine->data;
}

/** Returns a pointer to the sensitivity structure of the solver or NULL
    if it has not been constructed.
*/
SBML_ODESOLVER_API odeSense_t *IntegratorInstance_getSensitivityModel(integratorInstance_t *engine)
{
  return engine->os;
}
/**  Starts the default integration loop with standard error
     handling and returns 0 if integration was OK, and the error code
     if not.
*/

SBML_ODESOLVER_API int IntegratorInstance_integrate(integratorInstance_t *engine)
{
  while ( engine->solver->iout <= engine->solver->nout )
    if (!IntegratorInstance_integrateOneStep(engine)) 
      return IntegratorInstance_handleError(engine);    
 
  return 0; /* return 0, if ok */
}


/** Returns TRUE if the requested timecourse has been completed
    for the passed integratorInstance
*/

SBML_ODESOLVER_API int IntegratorInstance_timeCourseCompleted(integratorInstance_t *engine)
{
 
  return engine->solver->iout > engine->solver->nout;
}


/**  Creates and returns a cvodeResults structure containing
     the results of one integration run and NULL if not successful.
    
     The results must be freed by the caller with
     CvodeResults_free(results).
*/

SBML_ODESOLVER_API cvodeResults_t *IntegratorInstance_createResults(integratorInstance_t *engine)
{
  int i, j, k;
  cvodeResults_t *results;

  cvodeSettings_t *opt = engine->opt;
  cvodeResults_t *iResults = engine->results;

  if ( !opt->StoreResults || iResults == NULL ) return NULL;
  
  results = CvodeResults_create(engine->data, iResults->nout);
  RETURN_ON_FATALS_WITH(0);

  results->nout = iResults->nout;

  for ( i=0; i <=results->nout; i++ )
  {
    results->time[i] = iResults->time[i];
    for ( j=0; j < iResults->nvalues; j++ )
      results->value[j][i] = iResults->value[j][i];
  }

  if ( iResults->sensitivity != NULL )
  {
    CvodeResults_allocateSens(results, iResults->neq, iResults->nsens,
			      iResults->nout);
    for ( i=0; i<results->neq; i++ )
      for ( j=0; j<results->nsens; j++ )
      {
	results->index_sens[j] = iResults->index_sens[j];
	for ( k=0; k<=results->nout; k++ )
	  results->sensitivity[i][j][k] = iResults->sensitivity[i][j][k];
      }
  }

  return results;  
}


/** Writes results to file
 */
SBML_ODESOLVER_API void IntegratorInstance_printResults(integratorInstance_t *ii, FILE *fp)
{
  int n, j;
  cvodeResults_t *results;
  variableIndex_t *vi;
  
  results = IntegratorInstance_createResults(ii);

  fprintf(fp, "#t ");
  for (j=0; j<ii->om->neq; j++){
    vi = ODEModel_getOdeVariableIndex(ii->om, j);
    fprintf(fp, "%s ", ODEModel_getVariableName(ii->om, vi));
    VariableIndex_free(vi);
  }
  fprintf(fp, "\n");
  
  for (n=0; n<CvodeResults_getNout(results); n++){
    fprintf(fp, "%g ", CvodeResults_getTime(results, n));
    for (j=0; j<ii->om->neq; j++){
      vi = ODEModel_getOdeVariableIndex(ii->om, j);
      fprintf(fp, "%g ", CvodeResults_getValue(results, vi, n));
      VariableIndex_free(vi);
    }
    fprintf(fp, "\n");
  }
  
  CvodeResults_free(results);
  
}

/**  Writes current simulation data to the original model.
 */

SBML_ODESOLVER_API int IntegratorInstance_updateModel(integratorInstance_t *engine)
{
  int i;
  Species_t *s;
  Compartment_t *c;
  Parameter_t *p;
  
  odeModel_t *om = engine->om;
  cvodeData_t *data = engine->data;
  cvodeResults_t *results = engine->results;

  int nout = results->nout;
  int nvalues = data->nvalues;
  Model_t *m = om->m;

  
  for ( i=0; i<nvalues; i++ )
  {
    if ( (s = Model_getSpeciesById(m, om->names[i])) != NULL )
    {
      c = Model_getCompartmentById(m, Species_getCompartment(s));
      if ( !Species_getHasOnlySubstanceUnits(s) &&
	   Compartment_getSpatialDimensions != 0 )
	Species_setInitialConcentration(s, results->value[i][nout]);
      else
	Species_setInitialAmount(s, results->value[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(m, om->names[i])) != NULL ) 
      Compartment_setSize(c, results->value[i][nout]);    
    else if ( (p = Model_getParameterById(m, om->names[i])) !=  NULL ) 
      Parameter_setValue(p, results->value[i][nout]);
    else
      return 0;
  }

  return 1;

}


/**  Handles the simple case of models that contain no ODEs
 */

SBML_ODESOLVER_API int IntegratorInstance_simpleOneStep(integratorInstance_t *engine)
{
  /* just increase the time */
  engine->solver->t = engine->solver->tout;

  if ( engine->processEvents && engine->opt->compileFunctions &&
       !engine->om->compiledEventFunction )
    ODEModel_compileCVODEFunctions(engine->om);

  /* ... and call the default update function */
  return IntegratorInstance_updateData(engine);  
}

/* Executes assignment rules and event assignments (if fired)
   to establish the correct state of observables,
   sets trigger flags and returns the number of events that have fired.
*/
static int
IntegratorInstance_processEventsAndAssignments(integratorInstance_t *engine)
{
  int i, j, fired;
  ASTNode_t *trigger, *assignment;
  Event_t *e;
  EventAssignment_t *ea;
  variableIndex_t *vi;

  cvodeData_t *data = engine->data;
  odeModel_t *om = engine->om;

  /** evaluate assignments required befor trigger evaluation; \n */
  for ( i=0; i<om->nass; i++ )
    if ( om->assignmentsBeforeEvents[i] )
      data->value[om->neq+i] = evaluateAST(om->assignment[i], data);

  fired = 0;

  /** now go through all events: \n */
  for ( i=0; i<data->nevents; i++ )
  {
    e = Model_getEvent(om->simple, i);
    trigger = (ASTNode_t *) Trigger_getMath(Event_getTrigger(e));
    /** if a trigger has not been fired in the last time step \n */
    if ( data->trigger[i] == 0 )
    {
      /** check if it is fired now \n */
      if ( evaluateAST(trigger, data) )
      {
	fired++;
	data->trigger[i] = 1;
	/** and if yes, execute the events assignments; \n*/
	for ( j=0; j<Event_getNumEventAssignments(e); j++ )
	{
	  ea = Event_getEventAssignment(e, j);
	  assignment = (ASTNode_t *) EventAssignment_getMath(ea);
	  vi = ODEModel_getVariableIndex(om,
					 EventAssignment_getVariable(ea));
	  IntegratorInstance_setVariableValue(engine, vi,
					      evaluateAST(assignment, data));
	  VariableIndex_free(vi);
	}
      }
    }
    /** if the trigger has already been fired in the past,
        check if it still in fired state and reset flag if not; \n */
    else if ( !evaluateAST(trigger, data) )      
      data->trigger[i] = 0;
  }

  /** finally, evaluate assignments required after event execution; \n */
  for ( i=0; i<om->nass; i++ )
    if (om->assignmentsAfterEvents[i])
      data->value[om->neq+i] = evaluateAST(om->assignment[i], data);

  /** and return the number of fired events. */
  return fired;
}

/** Default function for updating data, to be used by solvers after
    they have calculate x(t) and updated the time.

    The function updates assigned variables, checks for event
    triggers and steady state, increases loop variables, stores
    results and sets next output time.
*/

int IntegratorInstance_updateData(integratorInstance_t *engine)
{
  int i, flag = 1, fired;
  char *buffer;
  cvodeSolver_t *solver = engine->solver;
  cvodeData_t *data = engine->data;
  cvodeSettings_t *opt = engine->opt;
  cvodeResults_t *results = engine->results;
  odeModel_t *om = engine->om;
  div_t d;   

  /* update rest of cvodeData_t **/
  data->currenttime = solver->t;

  if ( engine->processEvents )
  {
    if ( opt->compileFunctions ) 
      fired = om->compiledEventFunction(data, &(engine->isValid));   
    else
      fired = IntegratorInstance_processEventsAndAssignments(engine);

    if ( fired && opt->ResetCvodeOnEvent )
      engine->isValid = 0;

    if ( fired && opt->HaltOnEvent )
    {
      for ( i=0; i!= data->nevents; i++ )
      {
	if ( data->trigger[i] )
	{
	  buffer =
	    SBML_formulaToString((ASTNode_t *)
				 Trigger_getMath(Event_getTrigger( \
				 Model_getEvent(om->simple, i))));
	  SolverError_error(
			    ERROR_ERROR_TYPE,
			    SOLVER_ERROR_EVENT_TRIGGER_FIRED,
			    "Event Trigger %d (%s) fired at time %g. "
			    "Aborting simulation.",
			    i, buffer, data->currenttime);
	  free(buffer);
	}
      }
      flag = 0;
    }
  }

  /* store results */
  if ( opt->StoreResults )
  {
    results->nout = solver->iout;
    results->time[solver->iout] = solver->t;

    for ( i=0; i<data->nvalues; i++ )
      results->value[i][solver->iout] = data->value[i]; 
  }
          
  /* check for steady state if requested by cvodeSettings
     and stop integration if an approximate steady state is
     found   */
  if ( opt->SteadyState == 1 ) 
    if ( IntegratorInstance_checkSteadyState(engine) )
      flag = 0;  /* stop integration */


  /* if discrete data is observed, compute step contribution to
     the objective as well as the forward sensitivity */
 if ( (opt->observation_data_type == 1)  &&
      ((solver->iout==opt->OffSet) ||
       ((solver->iout+opt->OffSet) % (1+opt->InterStep)) == 0) )
  {
    /* set current time and state values for evaluating vector_v  */
    data->currenttime = solver->t;
  
    /* update objective quadrature  */
    if ( om->ObjectiveFunction )
    {
      om->compute_vector_v=1;
      d = div(solver->iout, 1+opt->InterStep);
      data->TimeSeriesIndex = opt->OffSet + d.quot;
      
      NV_Ith_S(solver->q, 0) = NV_Ith_S(solver->q, 0)
	+ evaluateAST( data->model->ObjectiveFunction, data);
      om->compute_vector_v=0;
    }

  } /* if (opt->observation_data_type == 1) */


  /* increase integration step counter */
  solver->iout++;
    
  /* ... and set next output time */
  if ( opt->Indefinitely )
    solver->tout += opt->Time;
  else if ( solver->iout <= solver->nout )
    solver->tout = opt->TimePoints[solver->iout];
  return flag;
}


/** Default function for updating adjoint data

    The function increases loop variables, stores
    results, increment jumps in adjoint of discrete data is observed, 
    and sets next output time.
*/

int IntegratorInstance_updateAdjData(integratorInstance_t *engine)
{
  int i, j, flag = 1, found = 0;
  cvodeSolver_t *solver = engine->solver;
  cvodeData_t *data = engine->data;
  cvodeSettings_t *opt = engine->opt;
  cvodeResults_t *results = engine->results;
  odeModel_t *om = engine->om;
  odeSense_t *os = engine->os;
  div_t d;

  /* update rest of cvodeData_t **/
  data->currenttime = solver->t;

  /* store results */
  if ( opt->AdjStoreResults )
  { 
    for ( i=0; i<data->neq; i++ ) 
      results->adjvalue[i][solver->iout] = data->adjvalue[i];
  }
            

  /* update adjoint state if discrete experimental data is observed */
  if ( (opt->observation_data_type == 1)  &&
       ((solver->iout==opt->OffSet)  ||
       ((solver->iout+opt->OffSet) % (1+opt->InterStep)) == 0) )
  {    
    /* set current time and state values for evaluating vector_v  */ 
    data->currenttime = solver->t;
   
    /* */
    if ( fabs(results->time[opt->PrintStep - solver->iout] - solver->t)
	 < 1e-3)  
    {
      found++;
      for ( j=0; j<om->neq; j++ )
	data->value[j] = results->value[j][opt->PrintStep-solver->iout];
    }      
    
    if ( found != 1 )
    {
      fprintf(stderr, "ERROR in update adjoint data: found none or more ");
      fprintf(stderr, "than one matchings in results data.\n");
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_UPDATE_ADJDATA,
			"Update Adjoint data Adjoint: ",
			"Failed to get state value at time %g.", solver->t);
        return 0;
    }

    om->compute_vector_v=1;
    d = div(solver->iout, 1+opt->InterStep);
    data->TimeSeriesIndex =
      data->model->time_series->n_time-1-(opt->OffSet + d.quot);
    for ( i=0; i<om->neq; i++ )
    { 
       data->adjvalue[i] = data->adjvalue[i] -
	 evaluateAST(data->model->vector_v[i], data);
       /* also need to update solver->yA */
        NV_Ith_S(solver->yA, i) = data->adjvalue[i];    
    }
    om->compute_vector_v=0;

    /* compute quadrature: quad for the computed step is now added to qA  */
    flag = CVodeGetQuadB(solver->cvadj_mem, solver->qA);
  
    if (flag != CV_SUCCESS)
    {  
      CVODE_HANDLE_ERROR(&flag, "CVodeGetQuadB", 1); 
      return 0;
    }

    /* reinit solvers */   
    flag = CVodeReInitB(solver->cvadj_mem, om->current_AdjRHS,
			data->currenttime,
			solver->yA, CV_SV, solver->reltolA, solver->abstolA);
    if (flag != CV_SUCCESS)
    { 
      CVODE_HANDLE_ERROR(&flag, "CVodeReInitB", 1);
      return 0; 
    } 

    /*  om->adjointQuadFunction */
    flag = CVodeQuadReInitB(solver->cvadj_mem, os->current_AdjQAD, solver->qA);
   if (flag != CV_SUCCESS)
   {   
      CVODE_HANDLE_ERROR(&flag, "CVodeQuadReInitB", 1);
      return 0;
   }

  } /* if (opt->observation_data_type == 1) */

  /* increase integration step counter */
  solver->iout++;
    
  /* ... and set next output time */
  if ( opt->Indefinitely )
    solver->tout += opt->Time;
  else if ( solver->iout <= solver->nout )
    solver->tout = opt->AdjTimePoints[solver->iout];

return 1;
}



/**************** Internal Checks During Integration Step *******************/

/**  Evaluates event trigger expressions and executes event assignments
     for those triggers that are true.

     Results are stored appropriately  in engine->data->value.
     Recreation of new solver structures (if an ODE variable is changed)
     is handled by integratorInstance_setVariableValue(engine).
     Returns the number of triggers that fired.
*/

SBML_ODESOLVER_API int IntegratorInstance_checkTrigger(integratorInstance_t *engine)
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

  for ( i=0; i<data->nevents; i++ )
  {
    e = Model_getEvent(om->simple, i);
    trigger = (ASTNode_t *) Trigger_getMath(Event_getTrigger(e));
    /* check each trigger flag */
    if ( data->trigger[i] == 0 )
    {
      /* if flag was zero, check if the trigger is fired now */
      if ( evaluateAST(trigger, data) )
      {
	if (opt->HaltOnEvent)
	  SolverError_error(ERROR_ERROR_TYPE,
			    SOLVER_ERROR_EVENT_TRIGGER_FIRED,
			    "Event Trigger %d (%s) fired at time %g. "
			    "Aborting simulation.",
			    i, SBML_formulaToString(trigger),
			    data->currenttime);
	/* removed AMF 08/11/05
	   else 
	   SolverError_error(WARNING_ERROR_TYPE,
	   SOLVER_ERROR_EVENT_TRIGGER_FIRED,
	   "Event Trigger %d (%s) fired at time %g. ",
	   i, SBML_formulaToString(trigger),
	   data->currenttime); */
	fired++;
	data->trigger[i] = 1;      
	for ( j=0; j<Event_getNumEventAssignments(e); j++ )
	{
	  ea = Event_getEventAssignment(e, j);
	  assignment = (ASTNode_t *) EventAssignment_getMath(ea);
	  vi = ODEModel_getVariableIndex(om,
					 EventAssignment_getVariable(ea));
	  IntegratorInstance_setVariableValue(engine, vi,
					      evaluateAST(assignment, data));
	  VariableIndex_free(vi);
	}
      }
    }
    else
      /* set trigger flag to zero */
      if ( !evaluateAST(trigger, data) ) data->trigger[i] = 0;
  }

  return fired;

}

/** Approximate identification of a steady state, returns 1 upon
    detection.

    Evaluates mean and standard deviation of rates and returns 1 if a
    "steady state" is reached. This function is also called internally
    by the integrator if steady state detection is switched on via
    CvodeSettings_setSteadyState! In this case detection of a steady
    state will stop the integrator. The threshold for steady state
    detection can be set via CvodeSettings_setSteadyStateThreshold.
*/
/* NOTE: provisional steady state finding! */
SBML_ODESOLVER_API int IntegratorInstance_checkSteadyState(integratorInstance_t *engine)
{
  int i;
  double dy_mean, dy_var, dy_std;
  cvodeData_t *data = engine->data;
  odeModel_t *om = engine->om;
  cvodeSettings_t *opt= engine->opt;
  
  /* calculate the mean and standard deviation of rates of change and
     store in cvodeData_t * */
  dy_mean = 0.0;
  dy_var = 0.0;
  dy_std = 0.0;
  
  for ( i=0; i<om->neq; i++ ) 
    dy_mean += fabs(evaluateAST(om->ode[i],data));

  dy_mean = dy_mean / om->neq;
  for ( i=0; i<om->neq; i++ ) 
    dy_var += MySQR(evaluateAST(om->ode[i],data) - dy_mean);

  dy_var = dy_var / (om->neq -1);
  dy_std = MySQRT(dy_var);

  /* stop integrator if mean + std of rates of change are lower than
     1e-11 */
  if ( (dy_mean + dy_std) < opt->ssThreshold )
  {
    data->steadystate = 1;
    /* issue warning only if steady state detection is one, and
       integration will stop */
    if ( opt->SteadyState )
      SolverError_error(MESSAGE_ERROR_TYPE,
			SOLVER_MESSAGE_STEADYSTATE_FOUND,
			"Steady state found. "
			"Simulation aborted at %g seconds. "
			"Mean of rates: %g, std %g",
			data->currenttime, dy_mean, dy_std);
    return(1) ;
  }
  else
  {
    data->steadystate = 0;
    return(0);
  }  
}


/**************** functions that switch between solvers *****************/

/** Sets the value of a variable or parameter during an
    integration via its variableIndex.

    This function also takes care of creating and freeing solver
    structures and ODE variables are changed!  The variableIndex can
    be retrieved from the odeModel with ODEModel_getVariable via the
    variable's or the parameter's ID symbol in the input SBML model
    (can be SBML compartments, species and parameters).

    NOTE: that discontinuities in the ODE RHS functions might be ignored by
    the SUNDIALS CVODES solver, as it can internally integrate beyond the
    next time step and miss updates. Thus, it is highly recommended to
    enforce the solver to only integrate to the next requested time step
    via CvodeSettings_setTStop(cvodeSettings_t *set, 1).
*/

SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *engine, variableIndex_t *vi, double value)
{
  int i;
  odeModel_t *om;
  cvodeData_t *data;
  
  om = engine->om;
  data = engine->data;

  data->value[vi->index] = value;

  /*!!! variable compartments:  here setVariableValue for species X
    should be interpreted as concetration, unless species in amounts,
    and thus for ODE variable species X_amount the input value needs
    to be divided by current compartment size !!!*/
  /* initial values need to be set in results,
     because they had already been initialized */
  if ( engine->solver->t == 0.0 && data->results != NULL )
    data->results->value[vi->index][0] = value;

  /* 'solver' is no longer consistant with 'data' if the event changed
     the value of an ODE variable */
  if ( vi->index < om->neq )
    engine->isValid = 0; 
  /* optimize ODEs for evaluation again, if a constant has been reset */
  else if (!engine->opt->compileFunctions &&  vi->index >= om->neq+om->nass ) 
    IntegratorInstance_optimizeOdes(engine);

  /* and finally assignment rules, potentially depending on that variable
     but otherwise rarely executed need to be evaluated */
  /*!!! this could use only dependent assignments ? */
  for ( i=0; i<om->nass; i++ )
    data->value[om->neq+i] = evaluateAST(om->assignment[i], data);

}

/* internal function used for optimization of ODEs; will handle the
   case of sensitivity analysis, where ODEs can not be optimized; */
/*!!! get rid of this, not required anymore as ODEs consist of assigned
  flux rates and otherwise only compartment (not even that when accounting
  for variable compartments) , and as compilation is
  used for fast integration anyways. */
void IntegratorInstance_optimizeOdes(integratorInstance_t *engine)
{
  int i, j;
  ASTNode_t *tmp;
  cvodeData_t *data;
  cvodeSettings_t *opt;
  odeModel_t *om;
  odeSense_t *os;

  os = engine->os;
  om = engine->om;
  opt = engine->opt;
  data = engine->data;
  
  for ( i=0; i<om->neq; i++ )
  {
    /* optimize ODE only if no sensitivity was requested OR no
       sensitivity matrix was constructed */
    if ( !opt->Sensitivity  || os->sensitivity )
    {
      /* optimize each ODE: replace nconst and simplifyAST */
      tmp = copyAST(om->ode[i]);
      for ( j=0; j<om->nconst; j++ ) 
	AST_replaceNameByValue(tmp,
			       om->names[om->neq+om->nass+j],
			       data->value[om->neq+om->nass+j]);
      
      if ( data->ode[i] != NULL )
	ASTNode_free(data->ode[i]);	
      data->ode[i] = simplifyAST(tmp);
      ASTNode_free(tmp);
    }
    else
    {
      if ( data->ode[i] != NULL )
	ASTNode_free(data->ode[i]);
      data->ode[i] = copyAST(om->ode[i]);
    }
  }
}

/** Moves the current integration one step forward and switches
    between different solvers.

    Solvers are currently CVODES for models with ODEs or an internal
    evaluation of rules for models without ODEs (rate rules).  Returns
    1 if integration can continue, 0 otherwise. A return value of 0
    can also be caused by a steady state or an event if steady state
    detection or HaltOnEvent options are activated, respectively.
*/

SBML_ODESOLVER_API int IntegratorInstance_integrateOneStep(integratorInstance_t *engine)
{
  engine->processEvents = 1;
     
  /* switch between solvers, the called functions are
     required to update ODE variables, that is data->values
     with index i:  0 <= i < neq
     and use the default update IntegratorInstance_updateData(engine)
     afterwards */
    
  /* for models without ODEs, we just need to increase the time */
  if ( engine->om->neq == 0 ) 
    return IntegratorInstance_simpleOneStep(engine);
  /* call CVODE Solver */
  else 
    return IntegratorInstance_cvodeOneStep(engine);
  
  /* upcoming solvers */
  /* if (om->algebraic) IntegratorInstance_idaOneStep(engine); */
}


/** Same as above, but without event processing!
    
    Returns 1 if integration can continue, 0 otherwise.
*/

SBML_ODESOLVER_API int IntegratorInstance_integrateOneStepWithoutEventProcessing(integratorInstance_t *engine)
{
  engine->processEvents = 0;
  /* switch between solvers, the called functions are
     required to update ODE variables, that is data->values
     with index i:  0 <= i < neq
     and use the default update IntegratorInstance_updateData(engine)
     afterwards */
    

  /* for models without ODEs, we just need to increase the time */
  if ( engine->om->neq == 0 ) 
    return IntegratorInstance_simpleOneStep(engine);
  /* call CVODE Solver */
  else 
    return IntegratorInstance_cvodeOneStep(engine);

  /* upcoming solvers */
  /* if (om->algebraic) IntegratorInstance_idaOneStep(engine); */
}

/**  Prints the current state of the solver
 */

SBML_ODESOLVER_API void IntegratorInstance_dumpSolver(integratorInstance_t *engine)
{
  odeModel_t *om = engine->om;
  cvodeSolver_t *solver = engine->solver;

  /* should be common to all solvers */
  printf("\n");
  printf("INTEGRATOR STATE:\n\n");
  printf("Current Time Settings:\n");
  printf("start time:          %g\n", solver->t0);
  printf("current time:        %g\n", solver->t);
  printf("next time:           %g\n", solver->tout);
  printf("current step number: %d\n", solver->iout);
  printf("total step number:   %d\n", solver->nout);
  printf("\n");

  /* solver specific switches */
  /* CVODE */
  if (om->neq)
  {
    printf("CVODE Error Settings:\n");
    /* currently the same abs. error for all y */
    printf("absolute error tolerance: %g\n", engine->opt->Error);
    printf("relative error tolerance: %g\n", solver->reltol);
    printf("max. internal step nr.:   %d\n", engine->opt->Mxstep);
  }
  
  /* if (om->algebraic) ?? */
  /* if (opt->Sensitivity) ?? */ 
  printf("\n");
}


/**  Frees an integratorInstance, including cvodeData
 */

SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *engine)
{
  /* solver specific switches */
  if (engine->om->neq) 
    IntegratorInstance_freeCVODESolverStructures(engine);

  /* if (om->algebraic) ?? */
  /* if (opt->Sensitivity) ?? */

  ODESense_free(engine->os);
  CvodeData_free(engine->data);
  free(engine->solver);
  free(engine);

}


/**  Standard handler for when the integrate function fails.
 */

SBML_ODESOLVER_API int IntegratorInstance_handleError(integratorInstance_t *engine)
{
  cvodeData_t *data;
  cvodeSettings_t *opt;
  int errorCode;

  if ( SolverError_getNum(ERROR_ERROR_TYPE) == 0 )
    return SolverError_getLastCode(WARNING_ERROR_TYPE);
  
  errorCode = SolverError_getLastCode(ERROR_ERROR_TYPE);
  data = engine->data;
  opt = engine->opt;

  /* if (om->algebraic) ?? */
  /* if (opt->Sensitivity) ?? */
  
  if ( errorCode )
  {        
    /* on flag CV_CONV_FAILURE
       try again, but now with/without generated Jacobian matrix  */
    if ( errorCode == CV_CONV_FAILURE && engine->run == 1 &&
	 opt->StoreResults)
    {      
      SolverError_error(MESSAGE_ERROR_TYPE,
			SOLVER_MESSAGE_RERUN_WITH_OR_WO_JACOBIAN,
			"Try to rerun with %s Jacobian matrix.",
			opt->UseJacobian ?
			"CVODE's internal approximation of the" :
			"analytic version of the");

      /* the following doesnt work anymore, as options
       should be treated as const! */
      /*       /\* integrate again *\/ */
/*       engine->UseJacobian = !engine->UseJacobian; */
/*       IntegratorInstance_reset(engine); */
/*       return IntegratorInstance_integrate(engine); */
    }
  }
  return errorCode;
}


/**  Prints some final statistics of the solver
 */

SBML_ODESOLVER_API void IntegratorInstance_printStatistics(integratorInstance_t *engine, FILE *f)
{
  odeModel_t *om = engine->om;

  /* if (om->algebraic) IntegratorInstance_printIDAStatistics(engine, f); */
    
  if (!om->neq)
    fprintf(f, "## No statistics available for models without ODEs.\n");
  else 
    IntegratorInstance_printCVODEStatistics(engine, f);
}

/** returns the time elapsed in seconds since the start of integration
 */
SBML_ODESOLVER_API double IntegratorInstance_getIntegrationTime(integratorInstance_t *engine)
{
  if (engine->clockStarted)
    return ((double)(clock() - engine->startTime)) / CLOCKS_PER_SEC; 
  else
    return 0;
}

double *IntegratorInstance_getValues(integratorInstance_t *engine)
{
  return engine->data->value;
}

/*@}*/
