/*
  Last changed Time-stamp: <2005-12-16 13:01:58 raim>
  $Id: integratorInstance.c,v 1.48 2005/12/16 18:32:15 afinney Exp $
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
    \brief This module contains all interfaces to an integratorInstance
    
    SOSlib allows to get and set model data during an integration run.
    This groups contain all functions available for this purpose.
*/
/*@{*/

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cvodes.h>
#include <kinsol.h>
#include <cvdense.h>
#include <nvector_serial.h>

#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/cvodeSolver.h"
#include "sbmlsolver/sensSolver.h"
#include "sbmlsolver/modelSimplify.h"
/* #include "sbmlsolver/daeSolver.h" */ /* !!! not yet working !!! */



/* local integratorInstance allocation and initialization */ 
static int
IntegratorInstance_initializeSolver(integratorInstance_t *,
				    cvodeData_t *,
				    cvodeSettings_t *,
				    odeModel_t *);
/* the following functions contain solver specific switches */
static int
IntegratorInstance_initializeSolverStructures(integratorInstance_t *);
static integratorInstance_t *
IntegratorInstance_allocate(cvodeData_t *, cvodeSettings_t *, odeModel_t *);



/***************** functions common to all solvers ************************/

/* allocate memory for a new integrator, initialize cvodeSolver
   structures from cvodeData, cvodeSettings and the odeModel */
static integratorInstance_t *IntegratorInstance_allocate(cvodeData_t *data,
							 cvodeSettings_t *opt,
							 odeModel_t *om)
{
  integratorInstance_t *engine;

  data->run = 0;
    
  ASSIGN_NEW_MEMORY(engine, struct integratorInstance, NULL);
  ASSIGN_NEW_MEMORY(engine->solver, struct cvodeSolver, 0);


  if (IntegratorInstance_initializeSolver(engine, data, opt, om))
    return engine;
  else
    return NULL;
}


/* initializes the solver */
static int IntegratorInstance_initializeSolver(integratorInstance_t *engine,
					cvodeData_t *data,
					cvodeSettings_t *opt, odeModel_t *om)
{
  int i;
  cvodeSolver_t *solver = engine->solver;
  cvodeResults_t *results = data->results;; 

  /* irreversibly linking the engine to its input model */
  engine->om = om;

  /* joining option, data and result structures */
  engine->opt = opt;
  engine->data = data;
  engine->results = data->results;

  /* initialize the solver's time settings */
  
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
  solver->iout=1;         /* counts integration steps, start with 1 */

  /* write initial conditions to results structure */
  if ( opt->StoreResults ) {
    results->time[0] = data->currenttime;
    for ( i=0; i<data->nvalues; i++ )
      results->value[i][0] = data->value[i];
  }
  
  /* count integration runs with this integratorInstance */
  data->run++;

  /* initialize specific solver structures */
  return IntegratorInstance_initializeSolverStructures(engine);

}


/** \brief Creates an new integratorInstance
     
    reads initial values from odeModel and integration settings from
    cvodeSettings to create integration data cvodeData and
    cvodeResults and initializes cvodeSolver structures.
*/

SBML_ODESOLVER_API integratorInstance_t *IntegratorInstance_create(odeModel_t *om, cvodeSettings_t *opt)
{
  cvodeData_t *data;
  integratorInstance_t *engine;
    
  data = CvodeData_create(om);
  RETURN_ON_FATALS_WITH(NULL);

  CvodeData_initialize(data, opt, om);
  RETURN_ON_FATALS_WITH(NULL);
  
  return IntegratorInstance_allocate(data, opt, om);
}


/** \brief Resets and existing integratorInstance with new settings.
    
    The instance can the be used for further integration runs
    with these new settings. Don't use during an integration run!
*/

SBML_ODESOLVER_API int IntegratorInstance_set(integratorInstance_t *engine, cvodeSettings_t *opt)
{
  CvodeData_initialize(engine->data, opt, engine->om);
  RETURN_ON_FATALS_WITH(0);
  return IntegratorInstance_initializeSolver(engine,
					     engine->data, opt, engine->om);
}


/**  \brief Resets and integratorInstance to its initial values

    After that, a new integration can be run. Don't use during an
    integration run!
*/

SBML_ODESOLVER_API int IntegratorInstance_reset(integratorInstance_t *engine)
{
  return IntegratorInstance_set(engine, engine->opt);
}



/**  \brief Returns the settings of this integratorInstance.

     These settings can then be change with cvodeSettings interface
     functions. The changes become only effective after
     IntegratorInstance_reset has been called. Don't use during an
     integration run!
*/

SBML_ODESOLVER_API cvodeSettings_t *IntegratorInstance_getSettings(integratorInstance_t *engine)
{
  return engine->opt;
}



/**  \brief Copies variable and parameter values between two
     integratorInstances that have been created from the same odeModel
*/

SBML_ODESOLVER_API void IntegratorInstance_copyVariableState(integratorInstance_t *target, integratorInstance_t *source)
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


/**  \brief Returns the current time of an integration
*/

SBML_ODESOLVER_API double IntegratorInstance_getTime(integratorInstance_t *engine)
{
    return engine->solver->t;
}


/** \brief Sets the next output time for infinite integration

    WARNING: the next output time must always be bigger then the
    previous. This function can only be used for infinite integration
    (CvodeSettings_setIndefinite(set, 1)).  Returns 1 if successful
    and 0 otherwise.
*/

SBML_ODESOLVER_API int IntegratorInstance_setNextTimeStep(integratorInstance_t *engine, double nexttime)
{
  if ( engine->opt->Indefinitely) {
    engine->solver->tout = nexttime;
    return 1;
  }
  return 0;
}


/**  \brief Gets the value of a variable or parameter during an integration
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


/** \brief Gets the sensitivity of variable y to parameter p at
    the current time.

    Must not be called, if sensitivity wasn't calculated!
*/

SBML_ODESOLVER_API double IntegratorInstance_getSensitivity(integratorInstance_t *engine,  variableIndex_t *y,  variableIndex_t *p)
{
  /*!!! needs better solution, if sensitivity for selected params
        will be implemented !!!*/
  return engine->data->sensitivity[y->index][p->type_index];
}


/**  \brief Prints variable names, the first value is the time,

    ODE variable values, assigned variable values and
    constant values follow. The order is the same
    as in IntegratorInstance_dumpData.
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpNames(integratorInstance_t *engine)
{
  ODEModel_dumpNames(engine->om);
}


/**  \brief Prints the current integration data,

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


/**  \brief Prints the current time, current value of variable y and 
     sensitivities to all parameters for which calculated
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpYSensitivities(integratorInstance_t *engine, variableIndex_t *y)
{
  int i;
  cvodeData_t *data = engine->data;

  if ( data->sensitivity == NULL )
    return;

  printf("%g  ", data->currenttime);
  printf("%g  ", data->value[y->index]);
  for ( i=0; i<data->nsens; i++ )
    printf("%g ", data->sensitivity[y->index][i]);
  printf("\n");
}


/**  \brief Prints the current time, the value of parameter p and 
     all variable's sensitivities Si to p, where
     i = 1 to NEQ
*/

SBML_ODESOLVER_API void IntegratorInstance_dumpPSensitivities(integratorInstance_t *engine, variableIndex_t *p)
{
  int j, index;
  cvodeData_t *data = engine->data;

  if ( data->sensitivity == NULL )
    return;

  printf("%g  ", data->currenttime);
  /* printf("%g  ", data->value[p->index]); */
  for ( j=0; j<data->neq; j++ )
    printf("%g ", data->sensitivity[j][p->type_index]);
  printf("\n");
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


/**  \brief Starts the default integration loop with standard error
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


/**  \brief Creates and returns a cvodeResults structure containing
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

  if ( !opt->StoreResults || iResults == NULL )
    return NULL;
  
  results = CvodeResults_create(engine->data, iResults->nout);
  RETURN_ON_FATALS_WITH(0);

  results->nout = iResults->nout;

  for ( i=0; i <=results->nout; i++ ) {
    results->time[i] = iResults->time[i];
    for ( j=0; j < iResults->nvalues; j++ )
      results->value[j][i] = iResults->value[j][i];
  }

  if ( iResults->sensitivity != NULL ) {
    CvodeResults_allocateSens(results, iResults->neq, iResults->nsens,
			      iResults->nout);
    for ( i=0; i<results->neq; i++ )
      for ( j=0; j<results->nsens; ++j )
	for ( k=0; k<=results->nout; k++ )
	  results->sensitivity[i][j][k] = iResults->sensitivity[i][j][k];
  }

  return results;  
}


/**  \brief Writes current simulation data to original model
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

  
  for ( i=0; i<nvalues; i++ ) {
    if ( (s = Model_getSpeciesById(m, om->names[i]))
	 != NULL ) {
      Species_setInitialConcentration(s, results->value[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(m, om->names[i])) != NULL ) {
      Compartment_setSize(c, results->value[i][nout]);
    }
    else if ( (p = Model_getParameterById(m, om->names[i])) !=  NULL ) {
      Parameter_setValue(p, results->value[i][nout]);
    }
    else
      return 0;
  }

  return 1;

}


/**  \brief Handles the simple case of models that contain no ODEs
*/

SBML_ODESOLVER_API int IntegratorInstance_simpleOneStep(integratorInstance_t *engine)
{
  /* just increase the time */
  engine->solver->t = engine->solver->tout;
  /* ... and call the default update function */
  return IntegratorInstance_updateData(engine);  
}


/** Default function for updating data, to be used by solvers after
    they have calculate x(t) and updated the time.

    The function updates assigned variables, checks for event
    triggers and steady state, increases loop variables, stores
    results and sets next output time.
*/

int IntegratorInstance_updateData(integratorInstance_t *engine)
{
  int i, flag = 1;
  cvodeSolver_t *solver = engine->solver;
  cvodeData_t *data = engine->data;
  cvodeSettings_t *opt = engine->opt;
  cvodeResults_t *results = engine->results;
  odeModel_t *om = engine->om;
    
  /* update rest of cvodeData_t **/
  data->currenttime = solver->t;

  for ( i=0; i<om->nass; i++ )
    data->value[om->neq+i] =
      evaluateAST(om->assignment[i], data);

  /* check for event triggers and evaluate the triggered
     events' assignments;
     stop integration if requested by cvodeSettings */
  if ( IntegratorInstance_checkTrigger(engine) )
    {
      /* recalculate assignments - they may be dependent
	 on event assignment results */
      for ( i=0; i<om->nass; i++ )
	data->value[om->neq+i] =
	  evaluateAST(om->assignment[i], data);

      if (opt->HaltOnEvent) 
	flag = 0; /* stop integration */
    }

  /* store results */
  if (opt->StoreResults)
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

  /* increase integration step counter */
  solver->iout++;
    
  /* ... and set next output time */
  if ( opt->Indefinitely )
    solver->tout += opt->Time;
  else if ( solver->iout <= solver->nout )
    solver->tout = opt->TimePoints[solver->iout];
  return flag;
}


/************* Internal Checks During Integration Step *******************/

/**  \brief Evaluates event trigger expressions and executes event assignments
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

    for ( i=0; i<Model_getNumEvents(om->simple); i++ ) {
      e = Model_getEvent(om->simple, i);
      trigger = (ASTNode_t *) Event_getTrigger(e);
      if ( data->trigger[i] == 0 && evaluateAST(trigger, data) ) {

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

/** \brief Approximate identification of a steady state
    
    Evaluates mean and std of rates and returns 1 if a "steady state"
    is reached to stop the calling integrator.  This function is only
    called by the integrator function if specified via cvodeSettings !
*/
/* NOTE: provisional steady state finding! */
SBML_ODESOLVER_API int IntegratorInstance_checkSteadyState(integratorInstance_t *engine)
{
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
    dy_var += MySQR(evaluateAST(om->ode[i],data) - dy_mean);
  }
  dy_var = dy_var / (om->neq -1);
  dy_std = MySQRT(dy_var);

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


/**************** functions that switch between solvers *****************/


/* initializes a cvodeSolver structure */
static int
IntegratorInstance_initializeSolverStructures(integratorInstance_t *engine)
{
  odeModel_t *om = engine->om;
  cvodeSettings_t *opt = engine->opt;

  /* IDA SOLVER for DAE systems */
  /* if (om->algebraic)
       return initializeIDASolverStructures; */

  /* nothing to be done for models without ODEs */
  if (!om->neq)
    return 1;

  /* CVODE SOLVER */
  if (om->neq) 
    return IntegratorInstance_createCVODESolverStructures(engine);
    
}


/** \brief Sets the value of a variable or parameter during an
    integration via its variableIndex.

    This function also takes care of creating and freeing solver
    structures and ODE variables are changed!  The variableIndex can
    be retrieved from the odeModel with ODEModel_getVariable via the
    variable's or the parameter's ID symbol in the input SBML model
    (can be SBML compartments, species and parameters).
*/

SBML_ODESOLVER_API void IntegratorInstance_setVariableValue(integratorInstance_t *engine, variableIndex_t *vi, double value)
{

  int i, j;
  ASTNode_t *tmp;
  odeModel_t *om;
  cvodeData_t *data;
  cvodeSettings_t *opt;
  cvodeSolver_t *solver;

  om = engine->om;
  opt = engine->opt;
  data = engine->data;
  solver = engine->solver;

  data->value[vi->index] = value;

  if ( vi->index < om->neq ) {
    /* if (om->algebraic) ?? */
    IntegratorInstance_freeCVODESolverStructures(engine);
    solver->t0 = solver->t;
    IntegratorInstance_createCVODESolverStructures(engine);
  }
  else if ( vi->index >= om->neq+om->nass ) {
    /* optimize ODEs for evaluation */
    /*!!! will need adaptation to selected sens.analysis !!!*/
    for ( i=0; i<om->neq; i++ ) {
      if ( !opt->Sensitivity  || om->sensitivity ) {
	/* optimize each ODE: replace nconst and simplifyAST */
	tmp = copyAST(om->ode[i]);
	for ( j=0; j<om->nconst; j++ ) {
	  AST_replaceNameByValue(tmp,
				 om->names[om->neq+om->nass+j],
				 data->value[om->neq+om->nass+j]);
	}
	if ( data->ode[i] != NULL )
	  ASTNode_free(data->ode[i]);	
 	data->ode[i] = simplifyAST(tmp);
	ASTNode_free(tmp);
      }
      else {
	if ( data->ode[i] != NULL )
	  ASTNode_free(data->ode[i]);
	data->ode[i] = copyAST(om->ode[i]);
      }
    }
  }
}	


/** \brief Moves the current integration one step forward and switches
    between different solvers for filling ODE variables.
    
    Returns 1 if integration can continue, 0 otherwise.
*/

SBML_ODESOLVER_API int IntegratorInstance_integrateOneStep(integratorInstance_t *engine)
{
     
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


/**  \brief Prints the current state of the solver
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
  if (om->neq) {
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


/**  \brief Frees an integratorInstance, including cvodeData
 */

SBML_ODESOLVER_API void IntegratorInstance_free(integratorInstance_t *engine)
{
  /* solver specific switches */
  if (engine->om->neq) {
    /* the same for CVODES and CVODE */
    IntegratorInstance_freeCVODESolverStructures(engine);
  }

  /* if (om->algebraic) ?? */
  /* if (opt->Sensitivity) ?? */
  
  CvodeData_free(engine->data);
  free(engine->solver);
  free(engine);

}


/**  \brief Standard handler for when the integrate function fails.
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
  
  if ( errorCode ) {
        
    /* on flag CV_CONV_FAILURE
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
  }
  return errorCode;
}


/**  \brief Prints some final statistics of the solver
*/

SBML_ODESOLVER_API void IntegratorInstance_printStatistics(integratorInstance_t *engine, FILE *f)
{
  odeModel_t *om = engine->om;
  cvodeSettings_t *opt = engine->opt;

  /* if (om->algebraic) IntegratorInstance_printIDAStatistics(engine, f); */
    
  if (!om->neq)
    fprintf(f, "## No statistics available for models without ODEs.\n");
  else 
    IntegratorInstance_printCVODEStatistics(engine, f);
}

/*@}*/
