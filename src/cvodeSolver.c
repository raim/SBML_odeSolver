/*
  Last changed Time-stamp: <2005-12-16 16:27:01 raim>
  $Id: cvodeSolver.c,v 1.20 2005/12/16 18:56:48 afinney Exp $
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

/*! \defgroup cvode CVODES ODE Solver:  x(t)
    \ingroup integrator     
    \brief This module contains the functions that call SUNDIALS CVODES
    solver routines for stiff and non-stiff ODE systems
    

*/
/*@{*/

#include <stdio.h>
#include <stdlib.h>

/* Header Files for CVODE */
#include "cvodes.h"    
#include "cvdense.h"
#include "nvector_serial.h"  

#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/cvodeSolver.h"
#include "sbmlsolver/sensSolver.h"
#include "sbmlsolver/modelSimplify.h"

/** The Hot Stuff!
   Calls CVODE to move the current simulation one time step; produces
   appropriate error messages on failures and returns 1 if the
   integration can continue, 0 otherwise. 
*/

SBML_ODESOLVER_API int IntegratorInstance_cvodeOneStep(integratorInstance_t *engine)
{
    int i, flag;
    realtype *ydata = NULL;
    
    cvodeSolver_t *solver = engine->solver;
    cvodeData_t *data = engine->data;
    cvodeSettings_t *opt = engine->opt;
    cvodeResults_t *results = engine->results;
    odeModel_t *om = engine->om;
    
    /* !!!! calling CVODE !!!! */
    flag = CVode(solver->cvode_mem, solver->tout,
		 solver->y, &(solver->t), CV_NORMAL);

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
	    "The solver took %d internal steps but could not "
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
			  solver->tout);
	SolverError_error(
			  WARNING_ERROR_TYPE,
			  SOLVER_ERROR_INTEGRATION_NOT_SUCCESSFUL,
		      "Integration not successful. Results are not complete.");

	return 0 ; /* Error - stop integration*/
      }
    
    ydata = NV_DATA_S(solver->y);
    
    /* update cvodeData time dependent variables */    
    for ( i=0; i<om->neq; i++ )
      data->value[i] = ydata[i];

    /* update rest of data with internal default function */
    flag = IntegratorInstance_updateData(engine);
    if ( flag != 1 )
      return 0;

    /*  calling CVODE: the same call can be used for CVODES  */
    if ( opt->Sensitivity ) 
      return IntegratorInstance_getForwardSens(engine);
    else
      return 1; /* OK */    
}


/************* CVODE integrator setup functions ************/


/* creates CVODE structures and fills cvodeSolver 
   return 1 => success
   return 0 => failure
*/
int
IntegratorInstance_createCVODESolverStructures(integratorInstance_t *engine)
{
    int i, j, flag, neq, method, iteration, maxorder;
    realtype *ydata, *abstoldata;

    ASTNode_t *tmp;
    odeModel_t *om = engine->om;
    cvodeData_t *data = engine->data;
    cvodeSolver_t *solver = engine->solver;
    cvodeSettings_t *opt = engine->opt;

    neq = engine->om->neq; /* number of equations */

    /*!!! should use simplified ASTs for construction !!!*/
    /* construct jacobian, if wanted and not yet existing */
    if ( opt->UseJacobian && om->jacob == NULL ) 
      /* reset UseJacobian option, depending on success */
      opt->UseJacobian = ODEModel_constructJacobian(om);
    else if ( !opt->UseJacobian ) {
      /* free jacobian from former runs (not necessary, frees also
         unsuccessful jacobians from former runs ) */
      if ( om->jacob != NULL) {
	for ( i=0; i<om->neq; i++ ) {
	  for ( j=0; j<om->neq; j++ )
	    ASTNode_free(om->jacob[i][j]);
	  free(om->jacob[i]);
	}
	free(om->jacob);
	om->jacob = NULL;
      }
      SolverError_error(WARNING_ERROR_TYPE,
			SOLVER_ERROR_MODEL_NOT_SIMPLIFIED,
			"Jacobian matrix construction skipped.");
      om->jacobian = opt->UseJacobian;
    }


    
    /* CVODESolverStructures from former runs must be freed */
    if ( data->run > 1 )
      IntegratorInstance_freeCVODESolverStructures(engine);

    
    /**
     * Allocate y, abstol vectors
     */
    solver->y = N_VNew_Serial(neq);
    if (check_flag((void *)solver->y, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector y failed");
      return 0; /* error */
    }
    solver->abstol = N_VNew_Serial(neq);
    if (check_flag((void *)solver->abstol, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector abstol failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector abstol failed");
      return 0; /* error */
    }

    
    /**
     * Initialize y, abstol vectors
     */
    ydata      = NV_DATA_S(solver->y);
    abstoldata = NV_DATA_S(solver->abstol);
    for ( i=0; i<neq; i++ ) {
      /* Set initial value vector components of y and y' */
      ydata[i] = data->value[i];
      /* Set absolute tolerance vector components,
         currently the same absolute error is used for all y */ 
      abstoldata[i] = opt->Error;       
    }

    /* set sensitivity structure to NULL */
    solver->yS = NULL;
    solver->senstol = NULL;
    
    /* scalar relative tolerance: the same for all y */
    solver->reltol = opt->RError;

    /**
     * Call CVodeCreate to create the non-linear solver memory:
     *
     * CV_BDF         Backward Differentiation Formula method
     * CV_ADAMS       Adams-Moulton method
     * CV_NEWTON      Newton iteration method
     * CV_FUNCTIONAL  functional iteration method
     */
    if ( opt->CvodeMethod == 0 )
      method = CV_BDF;
    else
      method = CV_ADAMS;
    if ( opt->IterMethod == 0 )
      iteration = CV_NEWTON;
    else
      iteration = CV_FUNCTIONAL;
    
    solver->cvode_mem = CVodeCreate(method, iteration);
    if (check_flag((void *)(solver->cvode_mem), "CVodeCreate", 0, stderr)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"CVodeCreate failed");
      return 0; /* error */
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
    flag = CVodeMalloc(solver->cvode_mem, f, solver->t0, solver->y,
                       CV_SV, solver->reltol, solver->abstol);
    if (check_flag(&flag, "CVodeMalloc", 1, stderr)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"CVodeMalloc failed");
      return 0; /* error ??? not required, handled by solverError ???  */
    }

    /**
     * Link the main integrator with data for right-hand side function
     */ 
    flag = CVodeSetFdata(solver->cvode_mem, engine->data);
    if (check_flag(&flag, "CVodeSetFdata", 1, stderr)) {
      /* ERROR HANDLING CODE if CVodeSetFdata failes */
       return 0; /* error */
    }
    
    /**
     * Link the main integrator with the CVDENSE linear solver
     */
    flag = CVDense(solver->cvode_mem, neq);
    if (check_flag(&flag, "CVDense", 1, stderr)) {
      /* ERROR HANDLING CODE if CVDense failes */
      return 0; /* error */
    }

    /**
     * Set the routine used by the CVDENSE linear solver
     * to approximate the Jacobian matrix to ...
     */
    if ( opt->UseJacobian == 1 ) {
      /* ... user-supplied routine Jac */
      flag = CVDenseSetJacFn(solver->cvode_mem, JacODE, engine->data);
    }
    else {
      /* ... the internal default difference quotient routine CVDenseDQJac */
      flag = CVDenseSetJacFn(solver->cvode_mem, NULL, NULL);
    }
    
    if ( check_flag(&flag, "CVDenseSetJacFn", 1, stderr) ) {
      return 0; /* error */
      /* ERROR HANDLING CODE if CVDenseSetJacFn failes */
    }

    /**
     * Set maximum number of internal steps to be taken
     * by the solver in its attempt to reach tout
     */
    
    CVodeSetMaxNumSteps(solver->cvode_mem, opt->Mxstep);

    if ( opt->Sensitivity ) {
      flag = IntegratorInstance_createCVODESSolverStructures(engine);
      if ( flag == 0 ) {
	return 0;/* error */
	/* ERROR HANDLING CODE if SensSolver construction failed */
      }
    }
    
    /* set ODEs for evaluation */
    /*!!! will need adaptation to selected sens.analysis !!!*/    
    for ( i=0; i<om->neq; i++ ) {
      /* optimize ODEs if no sensitivities are requested,
         or if the construction of the parametric matrix was
         successfull */
      if ( !opt->Sensitivity || om->sensitivity ) {
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

    return 1; /* 1 == OK */
}

/* frees N_V vector structures, and the cvode_mem solver */
void IntegratorInstance_freeCVODESolverStructures(integratorInstance_t *engine)
{
    /* Free the y, abstol vectors */ 
    N_VDestroy_Serial(engine->solver->y);
    N_VDestroy_Serial(engine->solver->abstol);

    /* Free the integrator memory */
    CVodeFree(engine->solver->cvode_mem);

    /* Free sensitivity vector yS */
    if (engine->solver->yS != NULL)
      N_VDestroyVectorArray_Serial(engine->solver->yS, engine->solver->nsens);

    /* Free sensitivity vector yS */
    if (engine->solver->senstol != NULL)
      N_VDestroy_Serial(engine->solver->senstol);

    /* Free IDA vector dy */
    if (engine->solver->dy != NULL)
      N_VDestroy_Serial(engine->solver->dy);
      
}



/** Prints some final statistics of the calls to CVODE routines, that
    are located in CVODE's iopt array.
*/

SBML_ODESOLVER_API void IntegratorInstance_printCVODEStatistics(integratorInstance_t *engine, FILE *f)
{
    int flag;
    long int nst, nfe, nsetups, nje, nni, ncfn, netf;

    cvodeSettings_t *opt = engine->opt;
    cvodeSolver_t *solver = engine->solver;

    flag = CVodeGetNumSteps(solver->cvode_mem, &nst);
    check_flag(&flag, "CVodeGetNumSteps", 1, f);
    CVodeGetNumRhsEvals(solver->cvode_mem, &nfe);
    check_flag(&flag, "CVodeGetNumRhsEvals", 1, f);
    flag = CVodeGetNumLinSolvSetups(solver->cvode_mem, &nsetups);
    check_flag(&flag, "CVodeGetNumLinSolvSetups", 1, f);
    flag = CVDenseGetNumJacEvals(solver->cvode_mem, &nje);
    check_flag(&flag, "CVDenseGetNumJacEvals", 1, f);
    flag = CVodeGetNonlinSolvStats(solver->cvode_mem, &nni, &ncfn);
    check_flag(&flag, "CVodeGetNonlinSolvStats", 1, f);
    flag = CVodeGetNumErrTestFails(solver->cvode_mem, &netf);
    check_flag(&flag, "CVodeGetNumErrTestFails", 1, f);

    fprintf(f, "\n## Integration Parameters:\n");
    fprintf(f, "## mxstep   = %d rel.err. = %g abs.err. = %g \n",
	    opt->Mxstep, opt->RError, opt->Error);
    fprintf(f, "## CVode Statistics:\n");
    fprintf(f, "## nst = %-6ld nfe  = %-6ld nsetups = %-6ld nje = %ld\n",
	    nst, nfe, nsetups, nje); 
    fprintf(f, "## nni = %-6ld ncfn = %-6ld netf = %ld\n",
	    nni, ncfn, netf);
    
    if (opt->Sensitivity)
      IntegratorInstance_printCVODESStatistics(engine, f);
}


/*
 * check return values of SUNDIALS functions
 */
int check_flag(void *flagvalue, char *funcname, int opt, FILE *f)
{

  int *errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL) {
    fprintf(f, "\n## SUNDIALS_ERROR: %s() failed - returned NULL pointer\n",
            funcname);
    return(1); }

  /* Check if flag < 0 */
  else if (opt == 1) {
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(f, "\n## SUNDIALS_ERROR: %s() failed with flag = %d\n",
              funcname, *errflag);
      return(1); }}

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && flagvalue == NULL) {
    fprintf(f, "\n## MEMORY_ERROR: %s() failed - returned NULL pointer\n",
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

void f(realtype t, N_Vector y, N_Vector ydot, void *f_data)
{
  
  int i;
  realtype *ydata, *dydata;
  cvodeData_t *data;
  data   = (cvodeData_t *) f_data;
  ydata  = NV_DATA_S(y);
  dydata = NV_DATA_S(ydot);

  /* update parameters: p is modified by CVODES,
     if fS could not be generated  */
  if ( data->p != NULL && data->opt->Sensitivity  )
    for ( i=0; i<data->nsens; i++ )
      data->value[data->model->index_sens[i]] = data->p[i];

  /* update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) 
    data->value[i] = ydata[i];

  /* update assignment rules */
  for ( i=0; i<data->model->nass; i++ ) 
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);

  /* update time  */
  data->currenttime = t;

  /* evaluate ODEs */
  for ( i=0; i<data->model->neq; i++ ) 
    dydata[i] = evaluateAST(data->ode[i],data);

}

/**
   Jacobian routine. Compute J(t,y).
   This function is (optionally) called by CVODE's integration routines
   every time needed.
   Very similar to the f routine, it evaluates the Jacobian matrix
   equations with CVODE's current values and writes the results
   back to CVODE's internal vector DENSE_ELEM(J,i,j).
*/

void
JacODE(long int N, DenseMat J, realtype t,
       N_Vector y, N_Vector fy, void *jac_data,
       N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3)
{
  
  int i, j;
  realtype *ydata;
  cvodeData_t *data;
  data  = (cvodeData_t *) jac_data;
  ydata = NV_DATA_S(y);
  
  /* update parameters: p is modified by CVODES,
     if fS could not be generated  */
  if ( data->p != NULL && data->opt->Sensitivity )
    for ( i=0; i<data->nsens; i++ )
      data->value[data->model->index_sens[i]] = data->p[i];

  /* update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) 
    data->value[i] = ydata[i];

  /* update assignment rules */
  for ( i=0; i<data->model->nass; i++ ) 
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);

  /* update time */
  data->currenttime = t;

  /* evaluate Jacobian*/
  for ( i=0; i<data->model->neq; i++ ) 
    for ( j=0; j<data->model->neq; j++ ) 
      DENSE_ELEM(J,i,j) = evaluateAST(data->model->jacob[i][j], data);
}


 /*! @} */
/* End of file */
