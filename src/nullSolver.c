/*
  Last changed Time-stamp: <2005-12-16 11:05:41 raim>
  $Id: nullSolver.c,v 1.7 2005/12/16 15:04:44 raimc Exp $
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
 *     Lukas Endler
 */
/*! \defgroup nullSolver KINSOL Root Finder:  f(x,p,t) = dx/dt = 0
    \ingroup integrator
    \brief NOT FUNCTIONAL YET: An interface to SUNDIALS KinSolver
    to find a local root of a system of non-linear equations.

    This code is working. It is, however, not functional. The
    KinSolver interface could be used to locally search for steady
    states. It would need better input settings. The example file
    `findRoot' in the examples folder can be used to play with
    settings and develop this functionality. Contact us, if you want
    to help!
*/
/** @{ */

#include <stdio.h>
#include <stdlib.h>

/* Header Files for CVODE */
#include "kinsol.h"
#include "kinspgmr.h"
#include "cvdense.h"
#include "nvector_serial.h"  

#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/cvodeSolver.h"
#include "sbmlsolver/nullSolver.h"


/* Prototypes of functions called by KINSOL */
static void func(N_Vector y, N_Vector dy, void *data);
static int JacV(N_Vector v, N_Vector Jv, N_Vector y,
		booleantype *new_u, void *data);


/* The Hot Stuff! */
/** 
*/

SBML_ODESOLVER_API int IntegratorInstance_nullSolver(integratorInstance_t *engine)
{
    int i, flag;
    realtype *ydata = NULL;
    
    cvodeSolver_t *solver = engine->solver;
    cvodeData_t *data = engine->data;
    cvodeSettings_t *opt = engine->opt;
    cvodeResults_t *results = engine->results;
    odeModel_t *om = engine->om;

    /* IntegratorInstance_freeCVODESolverStructures(engine); */
    printf("HALLO NULLSTELLE\n");
    if (!IntegratorInstance_createKINSolverStructures(engine))
      return 0;
    printf("HALLO KINSOL\n");
    
    /* !!!! calling KINSOL !!!! */
    flag = KINSol(solver->cvode_mem, solver->y,
		  KIN_LINESEARCH, 
		  solver->abstol, solver->abstol);
    /* !!! should use different scalings, first is D*y second
       is D*f(y) !!!*/
    printf("THX KINSOL\n");

    if ( flag != KIN_SUCCESS )
      {SolverError_error(ERROR_ERROR_TYPE,
			 SOLVER_ERROR_INTEGRATION_NOT_SUCCESSFUL,	 
			 "Null Solver not successful with flag %d.", flag);
	/* return 0 ;  *//* Error */
      }
    
    ydata = NV_DATA_S(solver->y);

    
    /* update cvodeData with foun steady state values */    
    for ( i=0; i<om->neq; i++ ) {
      data->value[i] = ydata[i];
      printf("%s = %g,  f(%s): %g\n",
	     om->names[i], data->value[i], om->names[i],
	     evaluateAST(data->model->ode[i], data));
    }
    /* IntegratorInstance_freeKINSolverStructures(engine); */
    return 1/* IntegratorInstance_updateData(engine) */; /* correct ?*/

}


/************* CVODES integrator setup functions ************/


/* creates CVODES structures and fills cvodeSolver 
   return 1 => success
   return 0 => failure
*/
int
IntegratorInstance_createKINSolverStructures(integratorInstance_t *engine)
{
    int i, j, flag, neq;
    realtype *ydata, *scale, *constr;
    N_Vector constraints;
    
    odeModel_t *om = engine->om;
    cvodeData_t *data = engine->data;
    cvodeSolver_t *solver = engine->solver;
    cvodeSettings_t *opt = engine->opt;

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
      IntegratorInstance_freeKINSolverStructures(engine);
    
    /*
     * Allocate y, abstol vectors, abstol is used as a scaling vector
     * for KINSol
     */
    solver->y = N_VNew_Serial(neq);
    if (check_flag((void *)solver->y, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "N_VNew_Serial for vector y failed");
      return 0; /* error */
    }

    /* scaling factor for y, diagonal elements of a matrix Du,
       such that Du*u vector has all components roughly of the
       same magnitude as y close to a solution */
    solver->abstol = N_VNew_Serial(neq);
    if (check_flag((void *)solver->abstol, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "N_VNew_Serial for scaling vector abstol failed");
      return 0; /* error */
    }
    
    /* scaling factor for f(y), diagonal elements of a matrix Df,
       such that Df*f(u) vector has all components of roughly the
       same magnitude as y (?)not too close(?) to a solution  */
    solver->abstol = N_VNew_Serial(neq);
    if (check_flag((void *)solver->abstol, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "N_VNew_Serial for scaling vector abstol failed");
      return 0; /* error */
    }
    
    /* constraints for solutions */
    constraints = N_VNew_Serial(neq);
    if (check_flag((void *)constraints, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "N_VNew_Serial for constraints vector dy failed");
      return 0; /* error */
    }
    

    /*
     * Initialize y, scale and constraint vectors
     */
    ydata       = NV_DATA_S(solver->y);    
    scale       = NV_DATA_S(solver->abstol);    
    constr      = NV_DATA_S(constraints);    
    for ( i=0; i<neq; i++ ) {
      /* Set initial value vector components of y and scaling factor
       */
      ydata[i]  = data->value[i];
      scale[i]  = 0.138; /* !!!good scaling factors required!!! */
      constr[i] = 0; /* !!!does not fit to kin_guide instructions,
		      where 1 is claimed to been y>0, while
		     2 should mean y >= 0. Two gives however an error
		     message !!!*/

    }
    /*
     * Call KINCreate to create the solver memory:
     *
     */
    solver->cvode_mem = KINCreate();
    if (check_flag((void *)(solver->cvode_mem), "KINCreate", 0, stderr)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "KINCreate failed");
    }

    /*
     * Call KINMalloc to initialize the integrator memory:
     * cvode_mem  pointer to the KINSOL memory block returned by KINCreate
     * func       user's right hand side function
     * y          the dependent variable vector
     */
    flag = KINMalloc(solver->cvode_mem, func, solver->y);
    if (check_flag(&flag, "KINMalloc", 1, stderr)) {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
                        "KINMalloc failed");
      return 0; /* error ??? not required, handled by solverError ???  */
    }

    /* for debugging */
    KINSetPrintLevel(solver->cvode_mem, 1);

    /* set constraints for solutions */
    flag = KINSetConstraints(solver->cvode_mem, constraints);
    if (check_flag(&flag, "KINSetConstraints", 1, stderr)) {
      /* ERROR HANDLING CODE if KINSetFdata failes */
      return 0;
    }
    N_VDestroy_Serial(constraints);
   
    /* 
     * Link the solver with data for right-hand side function
     */ 
    flag = KINSetFdata(solver->cvode_mem, engine->data);
    if (check_flag(&flag, "KINSetFdata", 1, stderr)) {
      /* ERROR HANDLING CODE if KINSetFdata failes */
       return 0;
   }
    
   /* Call KINSpgmr to specify the linear solver KINSPGMR  */
    flag = KINSpgmr(solver->cvode_mem, 100);
    if (check_flag(&flag, "KINSpgmr", 1, stderr)) {
      /* ERROR HANDLING CODE if KINSetFdata failes */
      return 0;
    }    
    
   /*
     * Set the routine used by the KINDense linear solver
     * to approximate the Jacobian matrix to ...
     */
    if ( opt->UseJacobian == 1 ) {
      /* ... user-supplied routine JacV when working */
      /* flag = KINSpgmrSetJacTimesVecFn(solver->cvode_mem, JacV, data); */
    }
    else {
      /* ... the internal default difference quotient routine KINDenseDQJac */
      
    }
    
    if ( check_flag(&flag, "KINSpgmrSetJacTimesVecFn", 1, stderr) ) {
      /* ERROR HANDLING CODE if KINDenseSetJacFn failes */
      return 0;
    }
     
    return 1; /* OK */
}

/* frees N_V vector structures, and the cvode_mem solver */
void IntegratorInstance_freeKINSolverStructures(integratorInstance_t *engine)
{
  N_VDestroy_Serial(engine->solver->y);
  N_VDestroy_Serial(engine->solver->abstol);
  KINFree(engine->solver->cvode_mem);
}

/** \brief Prints some final statistics of the calls to CVODE routines
*/

SBML_ODESOLVER_API void IntegratorInstance_printKINSOLStatistics(integratorInstance_t *engine, FILE *f)
{
  /* print KIN statistics */
}





/***************** Functions Called by the CVODE Solver ******************/

/**
   f routine. Compute the system R.H.S. f(y)
   This function is called by KIN's solving routines every time
   needed. 
*/
static void func(N_Vector y, N_Vector dydt, void *f_data)
{
  int i;
  realtype *ydata, *dydata;
  cvodeData_t *data;
  data   = (cvodeData_t *) f_data;
  ydata  = NV_DATA_S(y);
  dydata = NV_DATA_S(dydt);
  
  /* update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) 
    data->value[i] = ydata[i];
  /* update assignment rules */
  for ( i=0; i<data->model->nass; i++ ) 
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);
  
  /* !!! update time : null solver doesn't work for time dependent
     ODEs !!!*/
  /* data->currenttime = 0.0; */

  /* evaluate f(y) = dy/dt */
  for ( i=0; i<data->model->neq; i++ ) 
    dydata[i] = evaluateAST(data->model->ode[i],data);
}


/*
   Jacobian Vector function. Compute J x v
   This function is (optionally) called by KIN's integration routines
   every time needed.
*/
static int JacV(N_Vector v, N_Vector Jv, N_Vector y,
		booleantype *new_u, void *f_data)
{  
  int i, j;
  realtype *ydata, *JvData, *vdata;
  cvodeData_t *data;
  data  = (cvodeData_t *) f_data;
  ydata = NV_DATA_S(y);
  vdata = NV_DATA_S(v);
  JvData = NV_DATA_S(Jv);

  /* update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ )
    data->value[i] = ydata[i];

  /* update assignment rules */
  for ( i=0; i<data->model->nass; i++ )
     data->value[data->model->neq+i] =
       evaluateAST(data->model->assignment[i], data);

  /* evaluate Jacobian */
  for ( i=0; i<data->model->neq; i++ ) {
    JvData[i] = 0.0;
    for ( j=0; j<data->model->neq; j++ )
       JvData[j] += evaluateAST(data->model->jacob[i][j], data) * vdata[j];
    /*!!! not sure whether this is correct, needs checking !!!*/
  }
  *new_u = TRUE;      
  return 0;
}



/* End of file */
