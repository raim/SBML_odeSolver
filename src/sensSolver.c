/*
  Last changed Time-stamp: <2005-11-02 17:25:24 raim>
  $Id: sensSolver.c,v 1.1 2005/11/02 17:32:13 raimc Exp $
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
 *
 */

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
#include "sbmlsolver/sensSolver.h"
#include "sbmlsolver/cvodeSolver.h"

static int
check_flag(void *flagvalue, char *funcname, int opt, FILE *f);

/*
 * CVode solver: function computing the ODE rhs for a given value
 * of the independent variable t and state vector y.
 */
static void
f(realtype t, N_Vector y, N_Vector ydot, void *f_data);

/*
 * CVode solver: function computing the dense Jacobian J of the ODE system
 */
static void
JacODE(long int N, DenseMat J, realtype t,
       N_Vector y, N_Vector fy, void *jac_data,
       N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3);

/* 
 * fS routine. Compute sensitivity r.h.s. for param[iS]
 */
static void fS(int Ns, realtype t, N_Vector y, N_Vector ydot, 
               int iS, N_Vector yS, N_Vector ySdot, 
               void *fS_data, N_Vector tmp1, N_Vector tmp2);

static void
IntegratorInstance_freeSensSolverStructures(integratorInstance_t *);

/* The Hot Stuff! */
/** \brief Calls CVODES to move the current simulation, including
    sensitivity analysis one time step;

    produces appropriate error messages on failures and returns 1 if
    the integration can continue, 0 otherwise.  The function also
    checks for events and steady states and stores results if
    requested by cvodeSettings.  It also handles models without ODEs
    (only assignment rules or constant parameters).
*/

SBML_ODESOLVER_API int IntegratorInstance_cvodesOneStep(integratorInstance_t *engine)
{
    int i, j, flag;
    realtype *ydata = NULL;
    realtype *ySdata = NULL;
   
    cvodeSolver_t *solver = engine->solver;
    cvodeData_t *data = engine->data;
    cvodeSettings_t *opt = engine->opt;
    cvodeResults_t *results = engine->results;
    
    /*  calling CVODE: the same call can be used for CVODES  */
    flag = IntegratorInstance_cvodeOneStep(engine);
    if ( flag != 1 )
      return 0;
    RETURN_ON_FATALS_WITH(0);
    
    /* getting sensitivity */
    flag = CVodeGetSens(solver->cvode_mem, solver->t, solver->yS);
    if ( flag != CV_SUCCESS )
      return 0;
    else {
      for ( j=0; j<data->nsens; j++ ) {
	ySdata = NV_DATA_S(solver->yS[j]);
	for ( i=0; i<data->neq; i++ ) {
	  data->sensitivity[i][j] = ySdata[i];
          /* store results */
	  if (opt->StoreResults);
	    results->sensitivity[i][j][solver->iout-1] = ySdata[i]; 
	}
      }
    }
    
    return 1;
}


/************* CVODES integrator setup functions ************/


/* creates CVODES structures and fills cvodeSolver 
   return 1 => success
   return 0 => failure
*/
int
IntegratorInstance_createCVODESSolverStructures(integratorInstance_t *engine)
{
    int i, j, flag, neq, ns;
    realtype *ydata, *abstoldata, *ySdata;

    odeModel_t *om = engine->om;
    cvodeData_t *data = engine->data;
    cvodeSolver_t *solver = engine->solver;
    cvodeSettings_t *opt = engine->opt;

    realtype pbar[data->nsens+1];
    int plist[data->nsens+1];
    neq = engine->om->neq; /* number of equations */

     /* construct jacobian, if wanted and not yet existing */
    if ( opt->UseJacobian && om->jacob == NULL ) 
      /* reset UseJacobian option, depending on success */
      opt->UseJacobian = ODEModel_constructJacobian(om);
    else if ( !opt->UseJacobian ) {
      /* free jacobian from former runs (not necessary, frees also
         unsuccessful jacobians from former runs ) */
      if ( om->jacob != NULL) {
        for ( i=0; i<om->neq; i++ )
          free(om->jacob[i]);
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
      IntegratorInstance_freeCVODESSolverStructures(engine);

    
    /*
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
    
    /*
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
    
    /* scalar relative tolerance: the same for all y */
    solver->reltol = opt->RError;

    /*
     * Call CVodeCreate to create the solver memory:
     *
     * CV_BDF     specifies the Backward Differentiation Formula
     * CV_NEWTON  specifies a Newton iteration
     */
    solver->cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);
    if (check_flag((void *)(solver->cvode_mem), "CVodeCreate", 0, stderr)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "CVodeCreate failed");
    }

    /*
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
    /*
     * Link the main integrator with data for right-hand side function
     */ 
    flag = CVodeSetFdata(solver->cvode_mem, engine->data);
    if (check_flag(&flag, "CVodeSetFdata", 1, stderr)) {
      /* ERROR HANDLING CODE if CVodeSetFdata failes */
    }
    
    /*
     * Link the main integrator with the CVDENSE linear solver
     */
    flag = CVDense(solver->cvode_mem, neq);
    if (check_flag(&flag, "CVDense", 1, stderr)) {
      /* ERROR HANDLING CODE if CVDense fails */
    }

    /*
     * Set the routine used by the CVDENSE linear solver
     * to approximate the Jacobian matrix to ...
     */
    if ( opt->UseJacobian == 1 ) {
      /* ... user-supplied routine Jac */
      flag = CVDenseSetJacFn(solver->cvode_mem, JacODE, data);
    }
    else {
      /* ... the internal default difference quotient routine CVDenseDQJac */
      flag = CVDenseSetJacFn(solver->cvode_mem, NULL, NULL);
    }
    
    if ( check_flag(&flag, "CVDenseSetJacFn", 1, stderr) ) {
      /* ERROR HANDLING CODE if CVDenseSetJacFn failes */
    }

    /*
     * Set maximum number of internal steps to be taken
     * by the solver in its attempt to reach tout
     */
    CVodeSetMaxNumSteps(solver->cvode_mem, opt->Mxstep);



    /* construct sensitivity related structures */
    /* free sensitivity from former runs (might have changed
       for non-default cases!) */
    if ( om->jacob_sens != NULL ) 
      ODEModel_freeSensitivity(om);
    /* this function will require additional input for
       non-default case, via sensitivity input settings! */
    ODEModel_constructSensitivity(om);
    
    ns = data->nsens;
    engine->solver->nsens = data->nsens;
    /* constructParametricJacobian(om) */


    solver->yS = N_VNewVectorArray_Serial(ns, neq);      
    if (check_flag((void *)solver->yS, "N_VNewVectorArray_Serial",
		   1, stderr))
      return(0);

    /* (re)initialize sensitivities */
    for ( j=0; j<data->nsens; j++ ) {
      ySdata = NV_DATA_S(solver->yS[j]);
      for ( i=0; i<data->neq; i++ ) 
	ySdata[i] = data->sensitivity[i][j];
    }

    flag = CVodeSensMalloc(solver->cvode_mem, ns, CV_STAGGERED1, solver->yS);
    if(check_flag(&flag, "CVodeSensMalloc", 1, stderr)) {
      /* ERROR HANDLING CODE if failes */
    }

    /* setting parameter values and R.H.S function fS */
   
    /* was construction of parametric matrix successfull ? */
    if ( om->sensitivity ) {
      flag = CVodeSetSensRhs1Fn(solver->cvode_mem, fS);
      if (check_flag(&flag, "CVodeSetSensRhs1Fn", 1, stderr)) {
	/* ERROR HANDLING CODE if failes */
      }
    }
    else {
      /*!!! ???!!DOESNT WORK CURRENTLY!!??? !!!*/
      return 0;
      flag = CVodeSetSensRho(solver->cvode_mem, 0.0);
      if (check_flag(&flag, "CVodeSetSensRhs1Fn", 1, stderr)) {
	/* ERROR HANDLING CODE if failes */
      }      
    }
    
    /* plist could later be used to specify parameters for sens.analysis */
    ASSIGN_NEW_MEMORY_BLOCK(data->p, ns, realtype, 0);
    for ( i=0; i<ns; i++ ) {
      plist[i] = i+1;
      data->p[i] = data->value[om->index_sens[i]];
      pbar[i] = abs(data->p[i]);
    }

    /* data->p is actually only required if R.H.S. cannot be supplied */     
    flag = CVodeSetSensParams(solver->cvode_mem, data->p, pbar, plist);
    if (check_flag(&flag, "CVodeSetSensParams", 1, stderr))  {
      /* ERROR HANDLING CODE if  failes */
    }
    
    flag = CVodeSetSensErrCon(solver->cvode_mem, TRUE);
    if (check_flag(&flag, "CVodeSetSensFdata", 1, stderr)) {
      /* ERROR HANDLING CODE if failes */
    } 
      
    flag = CVodeSetSensFdata(solver->cvode_mem, data);
    if (check_flag(&flag, "CVodeSetSensFdata", 1, stderr))  {
      /* ERROR HANDLING CODE if  failes */
    }
      
     
    return 1; /* OK */
}

/* frees N_V vector structures, and the cvode_mem solver */
void IntegratorInstance_freeCVODESSolverStructures(integratorInstance_t *engine)
{
    /* Free CVODE structures */ 
    IntegratorInstance_freeCVODESolverStructures(engine);
    
    /* Free sensitivity vector yS */
    IntegratorInstance_freeSensSolverStructures(engine);
 
}

/* frees N_V vector structures, and the cvode_mem solver */
static void IntegratorInstance_freeSensSolverStructures(integratorInstance_t *engine)
{
    /* Free sensitivity vector yS */
    N_VDestroyVectorArray_Serial(engine->solver->yS, engine->solver->nsens);
}

/** \brief Prints some final statistics of the calls to CVODE routines
*/

SBML_ODESOLVER_API void IntegratorInstance_printCVODESStatistics(integratorInstance_t *engine, FILE *f)
{
  /* print CVODE statistics */
  IntegratorInstance_printCVODEStatistics(engine, f);
  /* print additional CVODES statistics ...*/
}


/*
 * check return values of SUNDIALS functions
 */
static int check_flag(void *flagvalue, char *funcname, int opt, FILE *f)
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

static void f(realtype t, N_Vector y, N_Vector ydot, void *f_data)
{
  
  int i;
  realtype *ydata, *dydata;
  cvodeData_t *data;
  data   = (cvodeData_t *) f_data;
  ydata  = NV_DATA_S(y);
  dydata = NV_DATA_S(ydot);

  /* update parameters */
  for ( i=0; i<data->nsens; i++ )
    data->value[data->model->index_sens[i]] = data->p[i];

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


/*
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

  /* update parameters */
  for ( i=0; i<data->nsens; i++ )
    data->value[data->model->index_sens[i]] = data->p[i];
  
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
 * fS routine. Compute sensitivity r.h.s. for param[iS]
 */

static void fS(int Ns, realtype t, N_Vector y, N_Vector ydot, 
               int iS, N_Vector yS, N_Vector ySdot, 
               void *fS_data, N_Vector tmp1, N_Vector tmp2)
{
  int i, j;
  realtype *ydata, *ySdata, *dySdata;
  cvodeData_t *data;
  data  = (cvodeData_t *) fS_data;
  
  ydata = NV_DATA_S(y);
  ySdata = NV_DATA_S(yS);
  
  dySdata = NV_DATA_S(ySdot);

  /* update parameters */
  for ( i=0; i<data->nsens; i++ )
    data->value[data->model->index_sens[i]] = data->p[i];
  
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

  /* evaluate parametric `jacobian' */
  for(i=0; i<data->model->neq; i++) {
    dySdata[i] = 0;
    for (j=0; j<data->model->neq; j++) {
      dySdata[i] += evaluateAST(data->model->jacob[i][j], data) * ySdata[j];
    }
    dySdata[i] +=  evaluateAST(data->model->jacob_sens[i][iS], data);
  }  

}
/* End of file */
