/*
  Last changed Time-stamp: <2006-02-24 14:17:35 raim>
  $Id: sensSolver.c,v 1.23 2006/03/02 16:15:27 raimc Exp $
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
/*! \defgroup sensi CVODES Forward Sensitivity:  dx(t)/dp
    \ingroup cvode
    \brief This module contains the functions that set up and
    call SUNDIALS CVODES forward sensitivity analysis routines.
    

*/
/*@{*/
#include <stdio.h>
#include <stdlib.h>

/* Header Files for CVODE */
#include "cvodes.h"    
#include "cvodea.h"  
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


/* 
 * fS routine. Compute sensitivity r.h.s. for param[iS]
 */
void fS(int Ns, realtype t, N_Vector y, N_Vector ydot,
	int iS, N_Vector yS, N_Vector ySdot,
	void *fS_data, N_Vector tmp1, N_Vector tmp2);


void fA(realtype t, N_Vector y, N_Vector yA, N_Vector yAdot,
	 void *fA_data);


void JacA(long int NB, DenseMat JA, realtype t,
                 N_Vector y, N_Vector yA, N_Vector fyA, void *jac_dataA,
                 N_Vector tmp1A, N_Vector tmp2A, N_Vector tmp3A);


void fQA(realtype t, N_Vector y, N_Vector yA, 
                N_Vector qAdot, void *fQ_dataA);

void fQ(realtype t, N_Vector y, N_Vector qdot, void *fQ_data);


/* The Hot Stuff! */
/** \brief Calls CVODES to provide forward sensitivities after a call to
    cvodeOneStep.

    produces appropriate error messages on failures and returns 1 if
    the integration can continue, 0 otherwise.  
*/

int IntegratorInstance_getForwardSens(integratorInstance_t *engine)
{
  int i, j, flag;
  realtype *ySdata = NULL;
   
  cvodeSolver_t *solver;
  cvodeData_t *data;
  cvodeSettings_t *opt;
  cvodeResults_t *results;
    
 solver = engine->solver;
 data = engine->data;
 opt = engine->opt;
 results = engine->results;
 
  /* getting sensitivities */
  flag = CVodeGetSens(solver->cvode_mem, solver->t, solver->yS);
    
  if ( flag != CV_SUCCESS )
    return 0; /* !!! CVODES specific error handling !!! */    
  else {
    for ( j=0; j<data->nsens; j++ ) {
      ySdata = NV_DATA_S(solver->yS[j]);
      for ( i=0; i<data->neq; i++ ) {
	data->sensitivity[i][j] = ySdata[i];
	/* store results */
	if ( opt->StoreResults )
	  results->sensitivity[i][j][solver->iout-1] = ySdata[i]; 
      }
    }
  }
    
  return 1;
}





/* The Hot Stuff! */
/** \brief Calls CVODES to provide adjoint sensitivities after a call to
    cvodeOneStep.

    NOTE: does not do interpolation based on CVodeGetSens. 

    produces appropriate error messages on failures and returns 1 if
    the integration can continue, 0 otherwise.  
*/

int IntegratorInstance_getAdjSens(integratorInstance_t *engine)
{
  int i;
  realtype *yAdata = NULL;
   
  cvodeSolver_t *solver;
  cvodeData_t *data;
  cvodeSettings_t *opt;
  cvodeResults_t *results;
    
  solver = engine->solver;
  data = engine->data;
  opt = engine->opt;
  results = engine->results;
  
  yAdata = NV_DATA_S(solver->yA);
  for ( i=0; i<data->neq; i++ ) {
    data->adjvalue[i] = yAdata[i];

    /* store results */
    if ( opt->AdjStoreResults )
      results->adjvalue[i][solver->iout-1] = yAdata[i];
  }
    
  return 1;
}






/************* CVODES integrator setup functions ************/


/* creates CVODES forward sensitivity solver structures
   return 1 => success
   return 0 => failure
*/
int
IntegratorInstance_createCVODESSolverStructures(integratorInstance_t *engine)
{
  int i, j, flag;
  realtype *abstoldata, *ySdata;

  odeModel_t *om = engine->om;
  cvodeData_t *data = engine->data;
  cvodeSolver_t *solver = engine->solver;
  cvodeSettings_t *opt = engine->opt;

  /* adjoint specific*/
  int method, iteration;
  /* N_Vector qA; */
  realtype *ydata;


  if( !opt->AdjointPhase ) {

    /* realtype pbar[data->nsens+1]; */
    /*int *plist; removed by AMF 8/11/05
      realtype *pbar;

      ASSIGN_NEW_MEMORY_BLOCK(plist, data->nsens+1, int, 0)
      ASSIGN_NEW_MEMORY_BLOCK(pbar, data->nsens+1, realtype, 0)*/


    /*****  adding sensitivity specific structures ******/

    /**
     * construct sensitivity related structures
     */
    /* free sensitivity from former runs (changed for non-default cases!) */
    ODEModel_freeSensitivity(om);

    /* if jacobian matrix has been constructed successfully,
       construct sensitivity matrix dx/dp, sets om->sensitivity
       to 1 if successful, 0 otherwise */
    /*!!! this function will require additional input for
      non-default case, via sensitivity input settings! !!!*/

    if ( om->jacobian ) 
      ODEModel_constructSensitivity(om);
    else {
      om->sensitivity = 0;
      om->jacob_sens = NULL;
      om->nsens = om->nconst;

      ASSIGN_NEW_MEMORY_BLOCK(om->index_sens, om->nsens, int, 0);
      /* !!! non-default case:
	 these values should be passed for other cases !!!*/
      for ( i=0; i<om->nsens; i++ )
	om->index_sens[i] = om->neq + om->nass + i;
    }
    
    engine->solver->nsens = data->nsens;

    /*!!! valgrind memcheck sensitivity: 1,248 (32 direct, 1,216 indirect)
      bytes in 1 blocks are definitely lost !!!*/
    solver->yS = N_VNewVectorArray_Serial(data->nsens, data->neq);      
    if (check_flag((void *)solver->yS, "N_VNewVectorArray_Serial",
		   1, stderr))
      return(0);

    /*
     * (re)initialize ySdata sensitivities
     */
    /* absolute tolerance for sensitivity error control */
    solver->senstol = N_VNew_Serial(data->nsens);
    abstoldata = NV_DATA_S(solver->senstol);
    for ( j=0; j<data->nsens; j++ ) {
      abstoldata[j] = 1e-4;
      ySdata = NV_DATA_S(solver->yS[j]);
      for ( i=0; i<data->neq; i++ ) {
	ySdata[i] = data->sensitivity[i][j];
	/* 	printf("  data->sensitivity[%d][%d] = %.3g  ", i, j, data->sensitivity[i][j]); */
      }
    }

    /*
     * set method
     */
    if ( opt->SensMethod == 0 ) 
      flag =CVodeSensMalloc(solver->cvode_mem,data->nsens,
			    CV_SIMULTANEOUS, solver->yS);
    else if ( opt->SensMethod == 1 )
      flag = CVodeSensMalloc(solver->cvode_mem, data->nsens,
			     CV_STAGGERED, solver->yS);
    else if ( opt->SensMethod == 2 )
      flag = CVodeSensMalloc(solver->cvode_mem, data->nsens,
			     CV_STAGGERED1, solver->yS);
    if(check_flag(&flag, "CVodeSensMalloc", 1, stderr)) {
      return 0;
      /* ERROR HANDLING CODE if failes */
    }



    


    /* *** set parameter values or R.H.S function fS *****/
    /* NOTES: */
    /* !!! plist could later be used to specify requested parameters
       for sens.analysis !!! */
    
    /* was construction of Jacobian and
       parametric matrix successfull ? */
    if ( om->sensitivity && om->jacobian ) {
      flag = CVodeSetSensRhs1Fn(solver->cvode_mem, fS);
      if (check_flag(&flag, "CVodeSetSensRhs1Fn", 1, stderr)) {
	return 0;
	/* ERROR HANDLING CODE if failes */
      }
      flag = CVodeSetSensFdata(solver->cvode_mem, data);
      if (check_flag(&flag, "CVodeSetSensFdata", 1, stderr))  {
	return 0;
	/* ERROR HANDLING CODE if  failes */
      }  
      data->p = NULL;
    }
    else {
      ASSIGN_NEW_MEMORY_BLOCK(data->p, data->nsens, realtype, 0);
      for ( i=0; i<data->nsens; i++ ) {
        /* data->p is only required if R.H.S. fS cannot be supplied */
	/* plist[i] = i+1; */
	data->p[i] = data->value[om->index_sens[i]]; 
	/* pbar[i] = abs(data->p[i]);  */ /*??? WHAT IS PBAR ???*/ 
      }
      flag = CVodeSetSensParams(solver->cvode_mem, data->p, NULL, NULL);
      if (check_flag(&flag, "CVodeSetSensParams", 1, stderr))  {
	return 0;
	/* ERROR HANDLING CODE if  failes */
      }
      flag = CVodeSetSensRho(solver->cvode_mem, 0.0); /* what is it? */
      if (check_flag(&flag, "CVodeSetSensRhs1Fn", 1, stderr)) {
	/* ERROR HANDLING CODE if  failes */
	return 0;
      }
    }
    /*     CVodeSetSensTolerances(solver->cvode_mem, CV_SS, */
    /* 			   solver->reltol, &solver->senstol); */
    
    /* difference FALSE/TRUE ? */
    flag = CVodeSetSensErrCon(solver->cvode_mem, FALSE);
    if (check_flag(&flag, "CVodeSetSensFdata", 1, stderr)) {
      return 0;
      /* ERROR HANDLING CODE if failes */
    }
    
    if( opt->DoAdjoint ) {
      
      solver->q = N_VNew_Serial(om->nconst);
      if (check_flag((void *) solver->q, "N_VNew_Serial", 0, stderr)) {
	/* Memory allocation of vector abstol failed */
	SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			  "N_VNew_Serial for vector q failed");
	return 0; /* error */
      }

      /* Init solver->qA = 0.0;*/
      for(i=0; i<om->nconst; i++)
	NV_Ith_S(solver->q, i) = 0.0;
 
      flag = CVodeQuadMalloc(solver->cvode_mem, fQ, solver->q);
      if (check_flag(&flag, "CVodeQuadMalloc", 1, stderr)) return(1);

      flag = CVodeSetQuadFdata(solver->cvode_mem, engine);
      if (check_flag(&flag, "CVodeSetQuadFdata", 1, stderr)) return(1);


      /*   flag = CVodeSetQuadErrCon(solver->cvode_mem, TRUE, CV_SS, reltolQ, &abstolQ); */
      /*     if (check_flag(&flag, "CVodeSetQuadErrCon", 1)) return(1); */
    }

    return 1; /* OK */

  } /* if (!opt->ReadyForAdjoint) */
  else{


    if (  om->jacob_sens == NULL ) 
      ODEModel_constructSensitivity(om);

    if (  om->jacob == NULL ) 
      opt->UseJacobian = ODEModel_constructJacobian(om);
    

    /*  Allocate yA, abstolA vectors */
    solver->yA = N_VNew_Serial(engine->om->neq);
    if (check_flag((void *)solver->yA, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector y failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector y failed");
      return 0; /* error */
    }

    solver->abstolA = N_VNew_Serial(engine->om->neq);
    if (check_flag((void *)solver->abstolA, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector abstol failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector abstol failed");
      return 0; /* error */
    }

    /**
     * Initialize y, abstol vectors
     */
    ydata      = NV_DATA_S(solver->yA);
    abstoldata = NV_DATA_S(solver->abstolA);
    for ( i=0; i<engine->om->neq; i++ ) {
      /* Set initial value vector components of yAdj */
      ydata[i] = data->adjvalue[i];
      /* Set absolute tolerance vector components,
         currently the same absolute error is used for all y */ 
      abstoldata[i] = opt->AdjError;       
    }
    
    /* scalar relative tolerance: the same for all y */
    solver->reltolA = opt->AdjRError;

    /* Adjoint specific allocations   */
    /**
     * Call CVodeCreateB to create the non-linear solver memory:\n
     *
     Nonlinear Solver:\n
     * CV_BDF         Backward Differentiation Formula method\n
     * CV_ADAMS       Adams-Moulton method\n
     Iteration Method:\n
     * CV_NEWTON      Newton iteration method\n
     * CV_FUNCTIONAL  functional iteration method\n
     */
    if ( opt->CvodeMethod == 0 )
      method = CV_BDF;
    else
      method = CV_ADAMS;
    if ( opt->IterMethod == 0 )
      iteration = CV_NEWTON;
    else
      iteration = CV_FUNCTIONAL;

    flag = CVodeCreateB(solver->cvadj_mem, method, iteration);
    if (check_flag(&flag, "CVodeCreateB", 1, stderr)) return(1);

    flag = CVodeMallocB(solver->cvadj_mem, fA, solver->t0, solver->yA, CV_SV, solver->reltolA, solver->abstolA);
    if (check_flag(&flag, "CVodeMallocB", 1, stderr)) return(1);

    flag = CVodeSetFdataB(solver->cvadj_mem, engine->data);
    if (check_flag(&flag, "CVodeSetFdataB", 1, stderr)) return(1);

    flag = CVDenseB(solver->cvadj_mem, om->neq);
    if (check_flag(&flag, "CVDenseB", 1, stderr)) return(1);

    flag = CVDenseSetJacFnB(solver->cvadj_mem, JacA, engine->data);
    if (check_flag(&flag, "CVDenseSetJacFnB", 1, stderr)) return(1);

    solver->qA = N_VNew_Serial(om->n_adj_sens);
    if (check_flag((void *) solver->qA, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector abstol failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector qA failed");
      return 0; /* error */
    }

    /* Init solver->qA = 0.0;*/
    for(i=0; i<om->n_adj_sens; i++)
      NV_Ith_S(solver->qA, i) = 0.0;
  
    /*  Allocate abstolQA vector */
    solver->abstolQA = N_VNew_Serial(engine->om->neq);
    if (check_flag((void *)solver->abstolQA, "N_VNew_Serial", 0, stderr)) {
      /* Memory allocation of vector abstol failed */
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"N_VNew_Serial for vector quad abstol failed");
      return 0; /* error */
    } 

    abstoldata = NV_DATA_S(solver->abstolQA);
    for ( i=0; i<engine->om->neq; i++ ) {
      /* Set absolute tolerance vector components,
	 currently the same absolute error is used for all y */ 
      abstoldata[i] = opt->AdjError;       
    } 

    solver->reltolQA = solver->reltolA;
 
    flag = CVodeQuadMallocB(solver->cvadj_mem, fQA, solver->qA);
    if (check_flag(&flag, "CVodeQuadMallocB", 1, stderr)) return(1);

    flag = CVodeSetQuadFdataB(solver->cvadj_mem, data);
    if (check_flag(&flag, "CVodeSetQuadFdataB", 1, stderr)) return(1);

    /*  flag = CVodeSetQuadErrConB(solver->cvadj_mem, TRUE, CV_SV, solver->reltolQA, solver->abstolQA); */
    /*   if (check_flag(&flag, "CVodeSetQuadErrConB", 1, stderr)) return(1); */

    /*   flag = CVodeSetQuadErrConB(solver->cvadj_mem, TRUE, CV_SS, &(solver->reltolQA), solver->abstolQA );  */
    /*     if (check_flag(&flag, "CVodeSetQuadErrConB", 1, stderr)) return(1); */

    return 1; /* OK */

  } /* if (opt->ReadyForAdjoint) */

}




/** \brief Prints some final statistics of the calls to CVODES forward
    sensitivity analysis routines
*/

SBML_ODESOLVER_API void IntegratorInstance_printCVODESStatistics(integratorInstance_t *engine, FILE *f)
{
  int flag;
  long int nfSe, nfeS, nsetupsS, nniS, ncfnS, netfS;
  cvodeSolver_t *solver = engine->solver;

  /* print additional CVODES statistics ..TODO...*/
  flag = CVodeGetNumSensRhsEvals(solver->cvode_mem, &nfSe);
  check_flag(&flag, "CVodeGetNumSensRhsEvals", 1, f);
  flag = CVodeGetNumRhsEvalsSens(solver->cvode_mem, &nfeS);
  check_flag(&flag, "CVodeGetNumRhsEvalsSens", 1, f);
  flag = CVodeGetNumSensLinSolvSetups(solver->cvode_mem, &nsetupsS);
  check_flag(&flag, "CVodeGetNumSensLinSolvSetups", 1, f);
  flag = CVodeGetNumSensErrTestFails(solver->cvode_mem, &netfS);
  check_flag(&flag, "CVodeGetNumSensErrTestFails", 1, f);
  flag = CVodeGetNumSensNonlinSolvIters(solver->cvode_mem, &nniS);
  check_flag(&flag, "CVodeGetNumSensNonlinSolvIters", 1, f);
  flag = CVodeGetNumSensNonlinSolvConvFails(solver->cvode_mem, &ncfnS);
  check_flag(&flag, "CVodeGetNumSensNonlinSolvConvFails", 1, f);

  fprintf(f, "## CVode Statistics:\n");
  fprintf(f, "## nfSe    = %5ld    nfeS     = %5ld\n", nfSe, nfeS);
  fprintf(f, "## netfs   = %5ld    nsetupsS = %5ld\n", netfS, nsetupsS);
  fprintf(f, "## nniS    = %5ld    ncfnS    = %5ld\n", nniS, ncfnS);    

}


/* Perform quadrature of forward and/or adjoint sensitivity  */
int IntegratorInstance_CVODEQuad(integratorInstance_t *engine)
{
   int flag;
   cvodeSolver_t *solver = engine->solver;
   cvodeSettings_t *opt = engine->opt;

   if( opt->AdjointPhase ){
     flag = CVodeGetQuadB(solver->cvadj_mem, solver->qA);
     if (check_flag(&flag, "CVodeGetQuadB", 1, stderr)) 
       return(1);
   }
   else{
     flag = CVodeGetQuad(solver->cvode_mem, solver->tout, solver->q);
     if (check_flag(&flag, "CVodeGetQuad", 1, stderr)) 
       return(1);
   }

  return 0;
}






/************* Additional Function for Sensitivity Analysis **************/

/**
 * fS routine: Called by CVODES to compute the sensitivity RHS for one
 * parameter.
 *
    CVODES sensitivity analysis calls this function any time required,
    with current values for variables x, time t and sensitivities
    s. The function evaluates df/dx * s + df/dp for one p and writes the
    results back to CVODE's N_Vector(ySdot) vector. The function is
    not `static' only for including it in the documentation!
 */

void fS(int Ns, realtype t, N_Vector y, N_Vector ydot, 
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

  /** update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = ydata[i];
  }
  /** update assignment rules : NOT needed!!!! */
  for ( i=0; i<data->model->nass; i++ ) {
    data->value[data->model->neq+i] = 
      evaluateAST(data->model->assignment[i],data); 
  }
  /** update time */
  data->currenttime = t;

  /** evaluate sensitivity RHS: df/dx * s + df/dp for one p */
  for(i=0; i<data->model->neq; i++) {
    dySdata[i] = 0;
    for (j=0; j<data->model->neq; j++) {
      dySdata[i] += evaluateAST(data->model->jacob[i][j], data) * ySdata[j];
    }
    dySdata[i] +=  evaluateAST(data->model->jacob_sens[i][iS], data);
  }  

}



/************* Additional Function for Adjoint Sensitivity Analysis **************/

/**
 * fA routine: Called by CVODES to compute the adjoint sensitivity RHS for one
 * parameter.
 *
    CVODES adjoint sensitivity analysis calls this function any time required,
    with current values for variables y, yA, and time t.
    The function evaluates -[df/dx]^T * yA + v
    and writes the results back to CVODE's N_Vector yAdot.
 */

 void fA(realtype t, N_Vector y, N_Vector yA, N_Vector yAdot, void *fA_data)
{
  int i, j;
  realtype *ydata, *yAdata, *dyAdata;
  cvodeData_t *data;
  data  = (cvodeData_t *) fA_data;
  
  ydata = NV_DATA_S(y);
  yAdata = NV_DATA_S(yA);
  dyAdata = NV_DATA_S(yAdot);

  /* update ODE variables from CVODE  */  
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = ydata[i];
  }
 
  /* update time */
  data->currenttime = t;


  /* evaluate adjoint sensitivity RHS: [df/dx]^T * yA + v */
  for(i=0; i<data->model->neq; i++) {
    dyAdata[i] = 0;
    for (j=0; j<data->model->neq; j++) {
      dyAdata[i] -= evaluateAST(data->model->jacob[j][i], data) * yAdata[j];        
    }

    /*  Vector v contribution */
    dyAdata[i] +=   evaluateAST( data->model->vector_v[i], data);
    
  }
}

/**
   Adjoint Jacobian routine: Compute JB(t,x) = -[df/dx]^T
   
   This function is (optionally) called by CVODES integration routines
   every time as required.

   Very similar to the fA routine, it evaluates the Jacobian matrix
   equations with CVODE's current values and writes the results
   back to CVODE's internal vector DENSE_ELEM(J,i,j).
*/

void JacA(long int NB, DenseMat JB, realtype t,
                 N_Vector y, N_Vector yB, N_Vector fyB, void *jac_dataB,
                 N_Vector tmp1B, N_Vector tmp2B, N_Vector tmp3B){

 int i, j;
  realtype *ydata;
  cvodeData_t *data;
  data  = (cvodeData_t *) jac_dataB;
  ydata = NV_DATA_S(y);
  

  /** update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) 
    data->value[i] = ydata[i];

  /** update assignment rules */
  for ( i=0; i<data->model->nass; i++ ) 
    data->value[data->model->neq+i] =
      evaluateAST(data->model->assignment[i],data);

  /** update time */
  data->currenttime = t;

  /** evaluate Jacobian JB = -[df/dx]^T */
  for ( i=0; i<data->model->neq; i++ ) 
    for ( j=0; j<data->model->neq; j++ ) 
      DENSE_ELEM(JB,i,j) = - evaluateAST(data->model->jacob[j][i], data);
}


void fQA(realtype t, N_Vector y, N_Vector yA, 
                N_Vector qAdot, void *fA_data)
{ 
  int i, j;
  realtype *ydata, *yAdata, *dqAdata;
  cvodeData_t *data;
  data  = (cvodeData_t *) fA_data;

  ydata = NV_DATA_S(y);
  yAdata = NV_DATA_S(yA);
  dqAdata = NV_DATA_S(qAdot);

  /* update ODE variables from CVODE  */  
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = ydata[i];
  }
 
  /* update time */
  data->currenttime = t;

  /* evaluate quadrature integrand: yA^T * df/dp */
  for(i=0; i<data->model->nconst; i++) {
    dqAdata[i] = 0.0;
     for(j=0; j<data->model->neq; j++) {
       dqAdata[i] += yAdata[j] * evaluateAST(data->model->jacob_sens[j][i], data);
     }
  }

}


void fQ(realtype t, N_Vector y, N_Vector qdot, void *fQ_data)
{
  int i, j, flag;
  realtype *ydata, *dqdata;
  cvodeData_t *data;
  cvodeSolver_t *solver; 
  integratorInstance_t *engine;
  N_Vector *yS;
  
  engine = (integratorInstance_t *) fQ_data;
  solver = engine->solver;
  data  =  engine->data;

  ydata = NV_DATA_S(y);
  dqdata = NV_DATA_S(qdot);

  /* update ODE variables from CVODE  */  
  for ( i=0; i<data->model->neq; i++ ) {
    data->value[i] = ydata[i];
  }
 
  /* update time */
  data->currenttime = t;

  /* update sensitivities */
  yS = N_VNewVectorArray_Serial(data->model->nconst, data->model->neq);

  /*  At t=0, yS is initialized to 0. In this case, CvodeGetSens shouldn't be used as it gives nan's */
  if(t != 0){
    flag = CVodeGetSens(solver->cvode_mem, t, yS);
    if (check_flag(&flag, "CVodeGetSens", 1, stderr))
      exit(EXIT_FAILURE);
  }  


  /* evaluate quadrature integrand: (y-ydata) * yS_i for each i */
  for(i=0; i<data->model->nconst; i++) {
    dqdata[i] = 0.0;
    for(j=0; j<data->model->neq; j++) 
      dqdata[i] += evaluateAST(engine->om->vector_v[j], data) * NV_Ith_S(yS[i], j);
  }

  N_VDestroyVectorArray_Serial(yS, data->model->nconst);

}


/** @} */
/* End of file */










