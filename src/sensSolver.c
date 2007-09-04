/*
  Last changed Time-stamp: <2007-06-12 14:42:34 xtof>
  $Id: sensSolver.c,v 1.59 2007/09/04 13:42:27 stefan_tbi Exp $
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
 *     Rainer Machne, James Lu and Stefan Müller
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
#include <string.h>
#include <math.h>

/* Header Files for CVODE */
#include "cvodes/cvodes.h"    
#include "cvodes/cvodes_dense.h"
#include "nvector/nvector_serial.h"  

#include "sbmlsolver/cvodeData.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/odeModel.h"
#include "sbmlsolver/variableIndex.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/cvodeSolver.h"
#include "sbmlsolver/sensSolver.h"
#include "sbmlsolver/odeSolver.h"
#include "sbmlsolver/interpol.h"
#include "sbmlsolver/variableIndex.h"

#include <sbml/SBMLTypes.h>
#include "sbmlsolver/ASTIndexNameNode.h"

/* 
 * fS routine. Compute sensitivity r.h.s. for param[iS]
 */
static int fS(int Ns, realtype t, N_Vector y, N_Vector ydot,
	      int iS, N_Vector yS, N_Vector ySdot,
	      void *fS_data, N_Vector tmp1, N_Vector tmp2);


static int fA(realtype t, N_Vector y, N_Vector yA, N_Vector yAdot,
	      void *fA_data);


static int JacA(long int NB, DenseMat JA, realtype t,
		N_Vector y, N_Vector yA, N_Vector fyA, void *jac_dataA,
		N_Vector tmp1A, N_Vector tmp2A, N_Vector tmp3A);

static int fQA(realtype t, N_Vector y, N_Vector yA, N_Vector qAdot,
	       void *fQ_dataA);

static int fQS(realtype t, N_Vector y, N_Vector qdot, void *fQ_data);

static int ODEModel_construct_vector_v_FromObjectiveFunction(odeModel_t *);

static ASTNode_t *copyRevertDataAST(const ASTNode_t *);


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
  else
  {
    for ( j=0; j<data->nsens; j++ )
    {
      ySdata = NV_DATA_S(solver->yS[j]);
      for ( i=0; i<data->neq; i++ )
      {
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
  
  for ( i=0; i<data->neq; i++ )
  {
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
  int i, j, reinit, flag, sensMethod;
 /*  realtype *abstoldata, *ySdata; */
  int found;

  odeModel_t *om = engine->om;
  cvodeData_t *data = engine->data;
  cvodeSolver_t *solver = engine->solver;
  cvodeSettings_t *opt = engine->opt;
  CVSensRhs1Fn sensRhsFunction = NULL;
  CVDenseJacFnB jacA = NULL;
  CVQuadRhsFnB adjointQuadFunction = NULL;
  CVRhsFnB adjointRHSFunction = NULL;
  
  /* adjoint specific*/
  int method, iteration;
 
  
  if( !opt->AdjointPhase )
  {

    /*****  adding sensitivity specific structures ******/

    /* set rhs function for sensitivity */
    if ( om->jacobian && om->sensitivity )
    {
      if ( opt->compileFunctions )
      {
	sensRhsFunction = ODEModel_getCompiledCVODESenseFunction(om);
	if ( !sensRhsFunction ) return 0;  /* error */
      }
      else
	sensRhsFunction = fS ;
    }
    
    /* if the sens. problem dimension has changed since
       the last run, free all sensitivity structures */
    if ( engine->solver->nsens != om->nsens )
      IntegratorInstance_freeForwardSensitivity(engine);
      
    engine->solver->nsens = om->nsens;

    /*
     * construct sensitivity yS and  absolute tolerance senstol
     * structures if they are not available from a previous run
     * with the same sens. problem dimension nsens
     */
    
    if ( solver->senstol == NULL )
    {
      solver->senstol = N_VNew_Serial(data->nsens);
      CVODE_HANDLE_ERROR((void *)solver->senstol,
			 "N_VNewSerial for senstol", 0);
    }

    /* remember: if yS had to be reconstructed, then also
       the sense solver structure CVodeSens needs reconstruction
       rather then mere re-initiation */
    reinit = 1;
    if ( solver->yS == NULL )
    {
      N_Vector y;

      y = N_VNew_Serial(data->neq);
      solver->yS = N_VCloneVectorArray_Serial(data->nsens, y);
      CVODE_HANDLE_ERROR((void *)solver->yS, "N_VNewVectorArray_Serial", 0);
      reinit = 0;
      N_VDestroy_Serial(y);
    }

    /* fill sens. and senstol data, yS and senstol:
       yS are 0.0 in a new run or
       old values in case of events */    
    for ( j=0; j<data->nsens; j++ )
    {
      NV_Ith_S(solver->senstol,j) =  NV_Ith_S(solver->abstol,0);
      for ( i=0; i<data->neq; i++ )
       NV_Ith_S(solver->yS[j], i) = data->sensitivity[i][j]; 
    }  

    /*
     * set forward sensitivity method
     */
    sensMethod = 0;
    if ( opt->SensMethod == 0 ) sensMethod = CV_SIMULTANEOUS;
    else if ( opt->SensMethod == 1 ) sensMethod = CV_STAGGERED;
    else if ( opt->SensMethod == 2 ) sensMethod = CV_STAGGERED1;

    /*!!! valgrind memcheck sensitivity: 1,248 (32 direct, 1,216 indirect)
      bytes in 1 blocks are definitely lost !!!*/
    if ( reinit == 0 )
    {
      flag = CVodeSensMalloc(solver->cvode_mem, data->nsens,
			     sensMethod, solver->yS);
      CVODE_HANDLE_ERROR(&flag, "CVodeSensMalloc", 1);    
    }
    else
    {
      flag = CVodeSensReInit(solver->cvode_mem, sensMethod, solver->yS);
      CVODE_HANDLE_ERROR(&flag, "CVodeSensReInit", 1);	  
    }

    /* *** set parameter values or R.H.S function fS *****/
    /* NOTES: */
    /* !!! plist could later be used to specify requested parameters
       for sens.analysis !!! */
    
    /* was construction of Jacobian and parametric matrix successfull ? */
    if ( om->sensitivity && om->jacobian )
    {
      flag = CVodeSetSensRhs1Fn(solver->cvode_mem, sensRhsFunction, data);
      CVODE_HANDLE_ERROR(&flag, "CVodeSetSensRhs1Fn Matrix", 1);

    }
    else
    {
      flag = CVodeSetSensRhs1Fn(solver->cvode_mem, NULL, NULL);
      CVODE_HANDLE_ERROR(&flag, "CVodeSetSensRhs1Fn NULL", 1);
      
    }

    /* initializing and setting data->p */
    for ( i=0; i<data->nsens; i++ )
      data->p[i] = data->p_orig[i] = data->value[om->index_sens[i]];

    flag = CVodeSetSensParams(solver->cvode_mem, data->p, NULL, NULL);
    CVODE_HANDLE_ERROR(&flag, "CVodeSetSensParams", 1);
    
    /* set tolerances for sensitivity equations */
   /*  CVodeSetSensTolerances(solver->cvode_mem, CV_SS, */
/*     			   NV_Ith_S(solver->senstol,0), &(opt->AdjError)); */
    
    /* difference FALSE/TRUE ? */
    flag = CVodeSetSensErrCon(solver->cvode_mem, FALSE);
    CVODE_HANDLE_ERROR(&flag, "CVodeSetSensErrCon", 1);
    

    /*  If linear functional exists, initialize quadrature computation  */
    if ( (om->ObjectiveFunction == NULL)  && (om->vector_v != NULL) )
    {

      if ( engine->solver->nsens != om->nsens && solver->qS != NULL ) 
      {
	N_VDestroy_Serial(solver->qS);
	CVodeQuadFree(solver->cvode_mem);

        /* Set solver->qS to NULL after calling N_VDestroy */
        solver->qS = NULL; 
      }

      if ( solver->qS == NULL )
      {
	solver->qS = N_VNew_Serial(om->nsens);
	CVODE_HANDLE_ERROR((void *) solver->qS,
			   "N_VNew_Serial for vector q", 0);
	
	/* Init solver->qS = 0.0;*/
	for(i=0; i<om->nsens; i++) NV_Ith_S(solver->qS, i) = 0.0;
        
	/* If quadrature memory has not been allocated (in either
	   of CreateCVODE(S)SolverStructures) */
	if ( solver->q == NULL )
	{
	  flag = CVodeQuadMalloc(solver->cvode_mem, fQS, solver->qS);
	  CVODE_HANDLE_ERROR(&flag, "CVodeQuadMalloc", 1);
	}
	else 
	{
	  if ( engine->solver->nsens != om->nsens )
	  {
	   /* need to do a CVodeQuadFree and CvodeQuadMalloc
              if nsens has changed  */ 
	    CVodeQuadFree(solver->cvode_mem);
	    flag = CVodeQuadMalloc(solver->cvode_mem, fQS, solver->qS);
	    CVODE_HANDLE_ERROR(&flag, "CVodeQuadMalloc", 1);
	  }
	  else
	  {
	    /* if nsens has not changed, simply CvodeQuadReInit suffices 
               since a  CVodeQuadMalloc has been called using solver->q, 
               as confirmed by solver->q != NULL  */
	    flag = CVodeQuadReInit(solver->cvode_mem, fQS, solver->qS);
	    CVODE_HANDLE_ERROR(&flag, "CVodeQuadReInit", 1);
	  }
	}
	  }
      else
      {
	/* Init solver->qS = 0.0;*/
	for(i=0; i<om->nsens; i++) 
	  NV_Ith_S(solver->qS, i) = 0.0;
	
	flag = CVodeQuadReInit(solver->cvode_mem, fQS, solver->qS);
	CVODE_HANDLE_ERROR(&flag, "CVodeQuadReInit", 1);
        
      }
      
      flag = CVodeSetQuadFdata(solver->cvode_mem, engine);
      CVODE_HANDLE_ERROR(&flag, "CVodeSetQuadFdata", 1);

    }
    
  } 
  else
    /* Adjoint Phase */
  {
    /* set rhs function for sensitivity */
    if ( opt->compileFunctions )
    {
      jacA = ODEModel_getCompiledCVODEAdjointJacobianFunction(om);
      if ( !jacA ) return 0;  /* error */
      /* set adjoint quadrature function for sensitivity */
      if ( om->sensitivity )
      {
	adjointQuadFunction = ODEModel_getCompiledCVODEAdjointQuadFunction(om);
	if ( !adjointQuadFunction ) return 0;  /* error */
      }

      adjointRHSFunction = ODEModel_getCompiledCVODEAdjointRHSFunction(om);
      if ( !adjointRHSFunction ) return 0;  /* error */

     }
    else
    {
      jacA = JacA;
      adjointQuadFunction = fQA ;
      adjointRHSFunction = fA;
    }
    /* remember which of the adj RHS function is being used */
    om->adjointRHSFunction = adjointRHSFunction;
    om->adjointQuadFunction = adjointQuadFunction;

      /*  If ObjectiveFunction exists, compute vector_v from it */ 
    if ( om->ObjectiveFunction != NULL ) 
    {
      flag = ODEModel_construct_vector_v_FromObjectiveFunction(om);
      if (flag != 1){
	fprintf(stderr, "error in constructing vector_v\n");
	return flag;
      }
    }

    /* update initial adjoint state if discrete experimental data is observed */
    if (opt->observation_data_type == 1)
    {
      /* set current time and state values for evaluating vector_v  */
      data->currenttime = solver->t;
    
        found = 0;
	if ( fabs(engine->results->time[opt->PrintStep] - solver->t) < 1e-5)
	{
	    found++;
	    for ( j=0; j<om->neq; j++ )
	      data->value[j] = engine->results->value[j][opt->PrintStep];

	}

	if (found != 1){
	  fprintf(stderr, "ERROR in updating the initial adjoint data.\n");
	  SolverError_error(FATAL_ERROR_TYPE,
			    SOLVER_ERROR_INITIALIZE_ADJDATA,
			    "Failed to get state value at time %g.", solver->t);
        return 0;
      }

      /* in discrete data case, set the initial adjoint solution to the evaluated value of vector_v */
      om->compute_vector_v=1;
      data->TimeSeriesIndex = data->model->time_series->n_time-1 ;
      for ( i=0; i<om->neq; i++ )
      {
	  data->adjvalue[i] = -evaluateAST( data->model->vector_v[i], data);
      }
      om->compute_vector_v=0;

    } 


    /*  Allocate yA, abstolA vectors */
    if ( solver->yA == NULL )
    {
      solver->yA = N_VNew_Serial(om->neq);
      CVODE_HANDLE_ERROR((void *)solver->yA,
			 "N_VNew_Serial for vector yA", 0);
    }

    if ( solver->abstolA == NULL )
    {
      solver->abstolA = N_VNew_Serial(engine->om->neq);
      CVODE_HANDLE_ERROR((void *)solver->abstolA,
			 "N_VNew_Serial for vector abstolA", 0);
    }

    /**
     * Initialize y, abstol vectors
     */
    for ( i=0; i<om->neq; i++ )
    {
      /* Set initial value vector components of yA */
          NV_Ith_S(solver->yA,i) = data->adjvalue[i]; 

      /* Set absolute tolerance vector components,
	 currently the same absolute error is used for all y */ 
         NV_Ith_S(solver->abstolA, i) = opt->AdjError; 
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
    if ( opt->CvodeMethod == 1 ) method = CV_ADAMS;
    else method = CV_BDF;
    
    if ( opt->IterMethod == 1 ) iteration = CV_FUNCTIONAL;
    else iteration = CV_NEWTON;

    /* Error if neither ObjectiveFunction nor vector_v has been set  */
    if( (om->ObjectiveFunction == NULL) && (om->vector_v == NULL)   ){
      return 0;
    }

   
    if ( data->adjrun == 1 )
      {
      flag = CVodeCreateB(solver->cvadj_mem, method, iteration);
      CVODE_HANDLE_ERROR(&flag, "CVodeCreateB", 1);

      flag = CVodeMallocB(solver->cvadj_mem, adjointRHSFunction, solver->t0,
			  solver->yA, CV_SV, solver->reltolA,
			  solver->abstolA);
      CVODE_HANDLE_ERROR(&flag, "CVodeMallocB", 1);
    }
    else
    { 
      flag = CVodeReInitB(solver->cvadj_mem, adjointRHSFunction, solver->t0,
			  solver->yA, CV_SV, solver->reltolA,
			  solver->abstolA);
      CVODE_HANDLE_ERROR(&flag, "CVodeReInitB", 1);
    }
     
    flag = CVodeSetFdataB(solver->cvadj_mem, engine->data);
    CVODE_HANDLE_ERROR(&flag, "CVodeSetFdataB", 1);

    flag = CVDenseB(solver->cvadj_mem, om->neq);
    CVODE_HANDLE_ERROR(&flag, "CVDenseB", 1);

    /*!!! could NULL be passed here if jacobian is not available ??*/
    flag = CVDenseSetJacFnB(solver->cvadj_mem, jacA, engine->data);
    CVODE_HANDLE_ERROR(&flag, "CVDenseSetJacFnB", 1);

    /* set adjoint max steps to be same as that for forward */
    flag = CVodeSetMaxNumStepsB(solver->cvadj_mem, opt->Mxstep);
    CVODE_HANDLE_ERROR(&flag, "CVodeSetMaxNumStepsB", 1);

   
    if ( solver->qA == NULL )
    {
      solver->qA = N_VNew_Serial(om->nsens);
      CVODE_HANDLE_ERROR((void *) solver->qA,
			 "N_VNew_Serial for vector qA failed", 0);

      /* Init solver->qA = 0.0;*/
      for( i=0; i<om->nsens; i++ )
	NV_Ith_S(solver->qA, i) = 0.0;
  
      flag = CVodeQuadMallocB(solver->cvadj_mem, adjointQuadFunction,
			      solver->qA);
      CVODE_HANDLE_ERROR(&flag, "CVodeQuadMallocB", 1);

    }
    else
    {
      /* Init solver->qA = 0.0;*/
      for( i=0; i<om->nsens; i++ )
	NV_Ith_S(solver->qA, i) = 0.0;
  
      flag = CVodeQuadReInitB(solver->cvadj_mem, adjointQuadFunction,
			      solver->qA);
      CVODE_HANDLE_ERROR(&flag, "CVodeQuadReInitB", 1);
    }
 
    /*  Allocate abstolQA vector */
    if ( solver->abstolQA == NULL )
    {
/*       solver->abstolQA = N_VNew_Serial(engine->om->neq); */
      solver->abstolQA = N_VNew_Serial(engine->om->nsens);
      CVODE_HANDLE_ERROR((void *)solver->abstolQA,
			 "N_VNew_Serial for vector quad abstol failed", 0);
    }
      
   /*  for ( i=0; i<engine->om->neq; i++ ) */
    for ( i=0; i<engine->om->nsens; i++ )
    {
      /* Set absolute tolerance vector components,
	 currently the same absolute error is used for all y */ 
      NV_Ith_S(solver->abstolQA, i) = opt->AdjError;
    } 

    solver->reltolQA = solver->reltolA;
 
    flag = CVodeSetQuadFdataB(solver->cvadj_mem, data);
    CVODE_HANDLE_ERROR(&flag, "CVodeSetQuadFdataB", 1);

    flag = CVodeSetQuadErrConB(solver->cvadj_mem, TRUE,
			       CV_SS, solver->reltolA, &(opt->AdjError) );
    CVODE_HANDLE_ERROR(&flag, "CVodeSetQuadErrConB", 1);

  
    /* END adjoint phase */
  } 

  return 1; /* OK */

}

/** \brief Prints some final statistics of the calls to CVODES forward and adjoint
    sensitivity analysis routines
*/

SBML_ODESOLVER_API int IntegratorInstance_printCVODESStatistics(integratorInstance_t *engine, FILE *f)
{
  int flag;
  long int nfSe, nfeS, nsetupsS, nniS, ncfnS, netfS;
  long int nstA, nfeA, nsetupsA, njeA, nniA, ncfnA, netfA;
  cvodeSolver_t *solver = engine->solver;
  void *cvode_memB;

  if (engine->opt->Sensitivity){ 
     /* print additional CVODES forward sensitivity statistics */
    fprintf(f, "##\n## CVodes Forward Sensitivity Statistics:\n");

    flag = CVodeGetNumSensRhsEvals(solver->cvode_mem, &nfSe);
    CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSensRhsEvals", 1);
    flag = CVodeGetNumRhsEvalsSens(solver->cvode_mem, &nfeS);
    CVODE_HANDLE_ERROR(&flag, "CVodeGetNumRhsEvalsSens", 1);
    flag = CVodeGetNumSensLinSolvSetups(solver->cvode_mem, &nsetupsS);
    CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSensLinSolvSetups", 1);
    flag = CVodeGetNumSensErrTestFails(solver->cvode_mem, &netfS);
    CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSensErrTestFails", 1);
    flag = CVodeGetNumSensNonlinSolvIters(solver->cvode_mem, &nniS);
    CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSensNonlinSolvIters", 1);
    flag = CVodeGetNumSensNonlinSolvConvFails(solver->cvode_mem, &ncfnS);
    CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSensNonlinSolvConvFails", 1);

    fprintf(f, "## nfSe    = %5ld    nfeS     = %5ld\n", nfSe, nfeS);
    fprintf(f, "## netfs   = %5ld    nsetupsS = %5ld\n", netfS, nsetupsS);
    fprintf(f, "## nniS    = %5ld    ncfnS    = %5ld\n", nniS, ncfnS);
  }
  
  if ((engine->opt->DoAdjoint) && (solver->cvadj_mem != NULL)){ 
      /* print additional CVODES adjoint sensitivity statistics */
     fprintf(f, "##\n## CVode Adjoint Sensitivity Statistics:\n");
     cvode_memB = CVadjGetCVodeBmem(solver->cvadj_mem);
     flag = CVodeGetNumSteps(cvode_memB, &nstA);
     CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSteps", 1);
     flag = CVodeGetNumRhsEvals(cvode_memB, &nfeA);
     CVODE_HANDLE_ERROR(&flag, "CVodeGetNumSensRhsEvals", 1);
     flag = CVodeGetNumLinSolvSetups(cvode_memB, &nsetupsA);
     CVODE_HANDLE_ERROR(&flag, "CVodeGetNumLinSolvSetups", 1);
     flag = CVDenseGetNumJacEvals(cvode_memB, &njeA);
     CVODE_HANDLE_ERROR(&flag, "CVDenseGetNumJacEvals", 1);
     flag = CVodeGetNonlinSolvStats(cvode_memB, &nniA, &ncfnA);
     CVODE_HANDLE_ERROR(&flag, "CVodeGetNonlinSolvStats", 1);
     flag = CVodeGetNumErrTestFails(cvode_memB, &netfA);
     CVODE_HANDLE_ERROR(&flag, "CVodeGetNumErrTestFails", 1);
     fprintf(f, "## nstA = %-6ld nfeA  = %-6ld nsetupsA = %-6ld njeA = %ld\n",
	  nstA, nfeA, nsetupsA, njeA); 
     fprintf(f, "## nniA = %-6ld ncfnA = %-6ld netfA = %ld\n",
	  nniA, ncfnA, netfA);
     fprintf(f, "## ncheck = %-6d\n", engine->opt->ncheck);
  }

  return(1);

}

/** \brief Sets a linear objective function (for sensitivity solvers) via a text input file 
*/
SBML_ODESOLVER_API int IntegratorInstance_setLinearObjectiveFunction(integratorInstance_t *engine, char *v_file)
{
  FILE *fp;
  char *line, *token;
  int i;
  ASTNode_t **vector_v, *tempAST;
  odeModel_t *om = engine->om;
 
  if ( om->vector_v != NULL  )
  {     
    for (i=0; i<om->neq; i++)
      ASTNode_free(om->vector_v[i]);
    free(om->vector_v);
  }

  ASSIGN_NEW_MEMORY_BLOCK(vector_v, om->neq, ASTNode_t *, 0);

  if ((fp = fopen(v_file, "r")) == NULL)
    SolverError_error(   FATAL_ERROR_TYPE,
			 SOLVER_ERROR_VECTOR_V_FAILED,
			 "File not found "
			 "in reading vector_v");  

  /* loop over lines */
  for (i=0; (line = get_line(fp)) != NULL; i++){
    /* read column 0 */
    token = strtok(line, " ");
    /* skip empty lines and comment lines */
    if (token == NULL || *token == '#'){
      free(line);
      i--;
      continue;
    }
    /* check variable order */
    if ( i == om->neq )
       SolverError_error(   FATAL_ERROR_TYPE,
			    SOLVER_ERROR_VECTOR_V_FAILED,
			    "Inconsistent number of variables (>) "
			    "in setting vector_v");
    
    if ( strcmp(token, om->names[i]) != 0 )
        SolverError_error(  FATAL_ERROR_TYPE,
			    SOLVER_ERROR_VECTOR_V_FAILED,
			    "Inconsistent variable order "
			    "in setting vector_v"); 
    
    /* read column 1 */
    token = strtok(NULL, "");
    tempAST = SBML_parseFormula(token);

    vector_v[i] = indexAST(tempAST, om->neq, om->names);
    ASTNode_free(tempAST);
    free(line);
  }

  if (i < om->neq)
    fatal(stderr, "read_v_file(): inconsistent number of variables (<)");

  om->vector_v = vector_v;
  
  return 1;
}


/** \brief Sets a general objective function (for ODE solution) via a
    text input file */
SBML_ODESOLVER_API int IntegratorInstance_setObjectiveFunction(integratorInstance_t *engine, char *ObjFunc_file)
{
  int i;
  FILE *fp;
  char *line = NULL, *line_formula = NULL, *token;
  ASTNode_t *ObjectiveFunction, *tempAST;
  odeModel_t *om = engine->om;

  /* If objective function exists, free it */
  if ( om->ObjectiveFunction != NULL  )
    ASTNode_free(om->ObjectiveFunction);

  if ( (fp = fopen(ObjFunc_file, "r")) == NULL )
    SolverError_error(FATAL_ERROR_TYPE,
		      SOLVER_ERROR_OBJECTIVE_FUNCTION_FAILED,
		      "File not found "
		      "in reading objective function"); 

  /* read line */
  for (i=0; (line = get_line(fp)) != NULL; i++)
  {   
    token = strtok(line, "");
    if (token == NULL || *token == '#')
    {
      free(line);
      i--;
    }
    else
    {
      if ( line_formula != NULL  )
        free(line_formula);
      ASSIGN_NEW_MEMORY_BLOCK(line_formula, strlen(line)+1, char, 0); 
      strcpy(line_formula, line); 
       if ( line != NULL  )
        free(line); 
    }
  }


  if( i > 1)
  {
   SolverError_error(FATAL_ERROR_TYPE,
		     SOLVER_ERROR_OBJECTIVE_FUNCTION_FAILED,
		     "Error in processing objective function file"); 
   return 0;
  }

  tempAST = SBML_parseFormula(line_formula);

  ObjectiveFunction = indexAST(tempAST, om->neq, om->names);
  ASTNode_free(tempAST);
 
  if ( line != NULL  )
    free(line);
  if ( line_formula != NULL  )
    free(line_formula);  

  om->ObjectiveFunction = ObjectiveFunction;
  
  return 1;
}



/** \brief Sets a general objective function (for ODE solution) via a string
*/
SBML_ODESOLVER_API int IntegratorInstance_setObjectiveFunctionFromString(integratorInstance_t *engine, char *str)
{
  ASTNode_t *ast, *temp_ast;
  odeModel_t *om = engine->om;
  
  if ( om->ObjectiveFunction != NULL  )
    ASTNode_free(om->ObjectiveFunction);
  
  temp_ast = SBML_parseFormula(str);
  ast = indexAST(temp_ast, om->neq, om->names);
  om->ObjectiveFunction = ast;
  
  ASTNode_free(temp_ast);
  
  return 1;
    
}

static int ODEModel_construct_vector_v_FromObjectiveFunction(odeModel_t *om)
{  
  int i, j, failed;
  ASTNode_t *fprime, *ObjFun;
  List_t *names;

  if ( om == NULL ) 
    return 0;
  
  if ( om->ObjectiveFunction == NULL ) 
    return 0;  
   
  /* if vector_v exists, free it */
  if ( om->vector_v != NULL )
  {
    for (  i=0; i<om->neq; i++  )
       if ( om->vector_v[i] != NULL )
       ASTNode_free(om->vector_v[i]);  

   free(om->vector_v);   
  }

  /******************** Calculate dJ/dx ************************/
  failed = 0; 
  ASSIGN_NEW_MEMORY_BLOCK(om->vector_v, om->neq, ASTNode_t *, 0);

  ObjFun = copyAST(om->ObjectiveFunction);

  for ( i=0; i<om->neq; i++ )
  {   
    fprime = differentiateAST(ObjFun, om->names[i]);
    om->vector_v[i] = fprime;

    /* check if the AST contains a failure notice */
    names = ASTNode_getListOfNodes(fprime,
				   (ASTNodePredicate) ASTNode_isName);

      for ( j=0; j<List_size(names); j++ ) 
	if ( strcmp(ASTNode_getName(List_get(names,j)),
		    "differentiation_failed") == 0 ) 
	  failed++;

      List_free(names); 
  }

  ASTNode_free(ObjFun);

  if ( failed != 0 )
  {
    SolverError_error(WARNING_ERROR_TYPE,
		      SOLVER_ERROR_ENTRIES_OF_THE_JACOBIAN_MATRIX_COULD_NOT_BE_CONSTRUCTED,
		      "%d entries of vector_v could not be "
		      "constructed, due to failure of differentiation. " ,
		      failed);   
  }
 
  return 1;
}



/** \brief Sets a general objective function (for ODE solution) via a
    text input file
*/
SBML_ODESOLVER_API int IntegratorInstance_readTimeSeriesData(integratorInstance_t *engine, char *TimeSeriesData_file)
{
  int i;
  char *name;

  int n_data;      /* number of relevant data columns */
  int *col;        /* positions of relevant columns in data file */
  int *index;      /* corresponding indices in variable list */
  int n_time;      /* number of data rows */
 
  
  odeModel_t *om = engine->om;
  int n_var = om->neq;       /* number ofvariable names */
  char **var = om->names;      /* variable names */
  time_series_t *ts;  /*   = om->time_series; */


  if ( om->time_series != NULL )
    free_data( om->time_series );
  
  /* alloc mem */
  /* ts = space(sizeof(time_series_t)); */
  ASSIGN_NEW_MEMORY_BLOCK(ts, 1, time_series_t, 0);

  /* alloc mem for index lists */
  ts->n_var = n_var;
  ASSIGN_NEW_MEMORY_BLOCK(ts->var,   n_var, char   *, 0);
  ASSIGN_NEW_MEMORY_BLOCK(ts->data,  n_var, double *, 0); 
  ASSIGN_NEW_MEMORY_BLOCK(ts->data2, n_var, double *, 0);
    
  /* initialize index lists */
  for ( i=0; i<n_var; i++ )
  {
    ASSIGN_NEW_MEMORY_BLOCK(name,  strlen(var[i])+1, char , 0);
    strcpy(name, var[i]);
    ts->var[i]   = name;
    ts->data[i]  = NULL;
    ts->data2[i] = NULL;
  }

  /* alloc temp mem for column info */
  ASSIGN_NEW_MEMORY_BLOCK(col,   n_var, int, 0);
  ASSIGN_NEW_MEMORY_BLOCK(index, n_var, int, 0);

  /* read header line */
  n_data = read_header_line(TimeSeriesData_file, n_var, var, col, index);
  ts->n_data = n_data;
	
  /* count number of lines */
  n_time = read_columns(TimeSeriesData_file, 0, NULL, NULL, NULL);
  ts->n_time = n_time;

  /* alloc mem for data */
  for ( i=0; i<n_data; i++ )
  {
    ASSIGN_NEW_MEMORY_BLOCK(ts->data[index[i]],  n_time, double, 0);
    ASSIGN_NEW_MEMORY_BLOCK(ts->data2[index[i]],  n_time, double, 0);
  }
  /* ts->time = space(n_time * sizeof(double)); */
  ASSIGN_NEW_MEMORY_BLOCK(ts->time,  n_time, double, 0);

  /* read data */
  read_columns(TimeSeriesData_file, n_data, col, index, ts);

  /* free temp mem */
  free(col);
  free(index);

  /* initialize interpolation type */
  ts->type = 3;

  /* calculate second derivatives, if data is treated as being continuous */
  for ( i=0; i<n_var; i++ )
    if ( ts->data[i] != NULL )
      spline(ts->n_time, ts->time, ts->data[i], ts->data2[i]);

  ts->last = 0;
    
  /* alloc mem for warnings */
  ASSIGN_NEW_MEMORY_BLOCK(ts->mess,  2, char *, 0);
  ASSIGN_NEW_MEMORY_BLOCK(ts->warn,  2, int, 0); 

  /* initialize warnings */
  ts->mess[0] = "argument out of range (left) ";
  ts->mess[1] = "argument out of range (right)";
  for ( i=0; i<2; i++ )
    ts->warn[i] = 0;   

  om->time_series = ts;

 return 1;
}


/** \brief Perform necessary quadratures for ODE/forward/adjoint
          sensitivity In forward phase, if nonlinear objective is
          present (by calling II_setObjectiveFunction) it is computed;
          alternatively, if linear objective is present (prior call to
          II_setLinearObj), it is computed.

          In adjoint phase, the backward quadrature for linear
          objective is performed.
 */
SBML_ODESOLVER_API int IntegratorInstance_CVODEQuad(integratorInstance_t *engine)
{
  int flag;
  cvodeSolver_t *solver = engine->solver;
  cvodeSettings_t *opt = engine->opt;
  int iS;
  odeModel_t *om = engine->om;
  cvodeData_t *data = engine->data;
  
  if( opt->AdjointPhase )
  {
    /* if continuous data is observed, compute quadrature;
       if discrete data is observed, quadrature has been computed already */
    if ( opt->observation_data_type == 0  )
    {  
      flag = CVodeGetQuadB(solver->cvadj_mem, solver->qA);
      CVODE_HANDLE_ERROR(&flag, "CVodeGetQuadB", 1);
    }

    /* For adj sensitivity components corresponding to IC as parameter */
    for( iS=0; iS<engine->om->nsens; iS++  )
      if ( data->model->index_sensP[iS] == -1 )
      {
        NV_Ith_S(solver->qA, iS) =
	  - data->adjvalue[data->model->index_sens[iS]];
      }
  }
  else
  {
     
    if ( opt->observation_data_type == 0  )
    {  /* if continuous data is observed, compute quadrature */
      /* If an objective function exists */
      if( om->ObjectiveFunction != NULL )
	{
	  flag = CVodeGetQuad(solver->cvode_mem, solver->tout, solver->q);
	  CVODE_HANDLE_ERROR(&flag, "CVodeGetQuad", 1);
	}

      /* If doing forward sensitivity analysis and vector_v exists,
	 compute sensitivity quadrature */
      if( opt->Sensitivity && om->ObjectiveFunction == NULL &&
	  om->vector_v != NULL  )
	{
	  flag = CVodeGetQuad(solver->cvode_mem, solver->tout, solver->qS);
	  CVODE_HANDLE_ERROR(&flag, "CVodeGetQuad", 1);
	}

    }
    else{
      /* else solver->q or solver->qS already contain the objective and its sensitivity respectively */
  
    }

  }

  return(1);
}


/** \brief Prints computed quadratures for ODE/forward/adjoint
           sensitivity. In forward phase, if nonlinear objective is
           present (by calling II_setObjectiveFunction) it is printed;
           alternatively, if linear objective is present (prior call
           to II_setLinearObj), it is printed.

	   In adjoint phase, the backward quadrature for linear
	   objective is printed.
*/

SBML_ODESOLVER_API int IntegratorInstance_printQuad(integratorInstance_t *engine, FILE *f)
{
  
  int j;
  odeModel_t *om = engine->om; 
  cvodeSettings_t *opt = engine->opt;
  ASTNode_t *tempAST; 

  if(opt->AdjointPhase)
  {
  /*  fprintf(f, "\nExpression for integrand of linear objective J: \n"); */
   for(j=0;j<om->neq;j++)
   {       
/*      Append "_data" to observation data in the vector_v AST */
/*      tempAST = copyRevertDataAST(om->vector_v[j]); */
/*      fprintf(f, "%d-th component: %s \n" , j, SBML_formulaToString(tempAST) ); */
/*      ASTNode_free(tempAST); */
   }
 
   for(j=0;j<om->nsens;j++)
      fprintf(f, "dJ/dp_%d=%0.15g ", j, NV_Ith_S(engine->solver->qA, j));   
   fprintf(f, "\n");
  }
  else
  {
   if ( om->ObjectiveFunction != NULL  ) 
   {  
      /*  Append "_data" to observation data in the vector_v AST */
     /*   tempAST = copyRevertDataAST(om->ObjectiveFunction); */
/*        fprintf(f, "\nExpression for integrand of objective J: %s \n\n" , */
/* 	       SBML_formulaToString(tempAST) ); */
       fprintf(f, "Computed J=%0.15g \n", NV_Ith_S(engine->solver->q, 0));
      /*  ASTNode_free(tempAST); */
   }
   else if ( engine->om->vector_v != NULL  )
   {
      fprintf(f, "\nExpression for integrand of linear objective J: \n");
      for(j=0;j<om->neq;j++)
      {    
        /* Append "_data" to observation data in the vector_v AST  */
	tempAST = copyRevertDataAST(om->vector_v[j]); 
	fprintf(f, "%d-th component: %s \n" , j,
		SBML_formulaToString(tempAST) );
        ASTNode_free(tempAST);
      }      

      for(j=0;j<om->nsens;j++)
	fprintf(f, "dJ/dp_%d=%0.15g ", j, NV_Ith_S(engine->solver->qS, j));
      fprintf(f, "\n");
   }
   else fprintf(f, "\nNo quadrature was performed \n");
 }

  return(1);
}


/** \brief Writes computed quadratures for the computed objective, or 
           the adjoint adjoint sensitivity if the solver is in adjoint phase.
*/

SBML_ODESOLVER_API int IntegratorInstance_writeQuad(integratorInstance_t *engine, realtype *data)
{
  
  int j;
  odeModel_t *om = engine->om; 
  cvodeSettings_t *opt = engine->opt;

  data = (realtype *) data;

  if(opt->AdjointPhase)
  { 
   for(j=0;j<om->nsens;j++)
     data[j] = NV_Ith_S(engine->solver->qA, j);
  }
  else
  {
   if ( om->ObjectiveFunction != NULL  )
     data[0] = NV_Ith_S(engine->solver->q, 0); 
   else if  (engine->opt->Sensitivity)
      for(j=0;j<om->nsens;j++)
	data[j] = NV_Ith_S(engine->solver->qS, j); 
  }

  return(1);
}

/* Extension of copyAST, for adding to the AST having ASTNode_isSetData
   by attaching the string extension "_data" to variable names */
static ASTNode_t *copyRevertDataAST(const ASTNode_t *f)
{
  int i;
  ASTNode_t *copy;
  char *tempstr = NULL;
  char *tempstr2 = NULL;

  copy = ASTNode_create();

  /* DISTINCTION OF CASES */
  /* integers, reals */
  if ( ASTNode_isInteger(f) ) 
    ASTNode_setInteger(copy, ASTNode_getInteger(f));
  else if ( ASTNode_isReal(f) ) 
    ASTNode_setReal(copy, ASTNode_getReal(f));
  /* variables */
  else if ( ASTNode_isName(f) )
  {
    if ( ASTNode_isSetIndex((ASTNode_t *)f) )
    {
      ASTNode_free(copy);
      copy = ASTNode_createIndexName();
      ASTNode_setIndex(copy, ASTNode_getIndex((ASTNode_t *)f));
    }

    if ( !ASTNode_isSetData((ASTNode_t *)f) )  
         ASTNode_setName(copy, ASTNode_getName(f));
    else
    {   
        /*  ASTNode_setData(copy); */
      tempstr  = ASTNode_getName(f);
      ASSIGN_NEW_MEMORY_BLOCK(tempstr2, strlen(tempstr)+5, char, 0);
      strncpy(tempstr2, tempstr, strlen(tempstr) );
      strncat(tempstr2, "_data", 5);
      ASTNode_setName(copy, tempstr2 );
      free(tempstr);
    }
    
  }
  /* constants, functions, operators */
  else
  {
    ASTNode_setType(copy, ASTNode_getType(f));
    /* user-defined functions: name must be set */
    if ( ASTNode_getType(f) == AST_FUNCTION ) 
      ASTNode_setName(copy, ASTNode_getName(f));
    for ( i=0; i<ASTNode_getNumChildren(f); i++ ) 
      ASTNode_addChild(copy, copyRevertDataAST(ASTNode_getChild(f,i)));
  }

  return copy;
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

static int fS(int Ns, realtype t, N_Vector y, N_Vector ydot, 
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
  for ( i=0; i<data->model->neq; i++ ) data->value[i] = ydata[i];
  
  /** update time */
  data->currenttime = t;

  /** evaluate sensitivity RHS: df/dx * s + df/dp for one p */
  for(i=0; i<data->model->neq; i++)
  {
    dySdata[i] = 0;
    for (j=0; j<data->model->neq; j++) 
      dySdata[i] += evaluateAST(data->model->jacob[i][j], data) * ySdata[j];
    if ( data->model->index_sensP[iS] != -1 )
      dySdata[i] +=
	evaluateAST(data->model->sens[i][data->model->index_sensP[iS]], data);
  }
  /* printf("s"); */

  return (0);
}

/********* Additional Function for Adjoint Sensitivity Analysis **********/

/**
 * fA routine: Called by CVODES to compute the adjoint sensitivity RHS for one
 * parameter.
 *
 CVODES adjoint sensitivity analysis calls this function any time required,
 with current values for variables y, yA, and time t.
 The function evaluates -[df/dx]^T * yA + v
 and writes the results back to CVODE's N_Vector yAdot.
*/

static int fA(realtype t, N_Vector y, N_Vector yA, N_Vector yAdot,
	      void *fA_data)
{
  int i, j;
  realtype *ydata, *yAdata, *dyAdata;
  cvodeData_t *data;
  data  = (cvodeData_t *) fA_data;
  
  ydata = NV_DATA_S(y);
  yAdata = NV_DATA_S(yA);
  dyAdata = NV_DATA_S(yAdot);

  /* update ODE variables from CVODE  */
  for ( i=0; i<data->model->neq; i++ ) data->value[i] = ydata[i];
 
  /* update time */
  data->currenttime = t;

  /* evaluate adjoint sensitivity RHS: -[df/dx]^T * yA + v */
  for(i=0; i<data->model->neq; i++)
  {
    dyAdata[i] = 0;
    for (j=0; j<data->model->neq; j++)
      dyAdata[i] -= evaluateAST(data->model->jacob[j][i], data) * yAdata[j];

        
    /*  Vector v contribution, if continuous data is used */
    if(data->model->discrete_observation_data==0)
      dyAdata[i] +=   evaluateAST( data->model->vector_v[i], data);

  }
  
  return (0);
}

/**
   Adjoint Jacobian routine: Compute JB(t,x) = -[df/dx]^T
   
   This function is (optionally) called by CVODES integration routines
   every time as required.

   Very similar to the fA routine, it evaluates the Jacobian matrix
   equations with CVODE's current values and writes the results
   back to CVODE's internal vector DENSE_ELEM(J,i,j).
*/

static int JacA(long int NB, DenseMat JB, realtype t,
		N_Vector y, N_Vector yB, N_Vector fyB, void *jac_dataB,
		N_Vector tmp1B, N_Vector tmp2B, N_Vector tmp3B)
{

  int i, j;
  realtype *ydata;
  cvodeData_t *data;
  data  = (cvodeData_t *) jac_dataB;
  ydata = NV_DATA_S(y);
  
  /** update ODE variables from CVODE */
  for ( i=0; i<data->model->neq; i++ ) data->value[i] = ydata[i];

  /** update time */
  data->currenttime = t;

  /** evaluate Jacobian JB = -[df/dx]^T */
  for ( i=0; i<data->model->neq; i++ ) 
    for ( j=0; j<data->model->neq; j++ ) 
      DENSE_ELEM(JB,i,j) = - evaluateAST(data->model->jacob[j][i], data);

  return (0);
}


static int fQA(realtype t, N_Vector y, N_Vector yA, 
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
  for ( i=0; i<data->model->neq; i++ ) data->value[i] = ydata[i];
 
  /* update time */
  data->currenttime = t;

  /* evaluate quadrature integrand: yA^T * df/dp */
  for ( i=0; i<data->model->nsens; i++ )
  {
    dqAdata[i] = 0.0;

    if ( data->model->index_sensP[i] != -1 )
      for ( j=0; j<data->model->neq; j++ )
	dqAdata[i] += yAdata[j] *
	  evaluateAST(data->model->sens[j][data->model->index_sensP[i]], data);
  }

  return (0);
}


static int fQS(realtype t, N_Vector y, N_Vector qdot, void *fQ_data)
{
  int i, j, flag;
  realtype *ydata, *dqdata;
  cvodeData_t *data;
  cvodeSolver_t *solver; 
  integratorInstance_t *engine;
  N_Vector *yS;
  N_Vector yy;
  
  engine = (integratorInstance_t *) fQ_data;
  solver = engine->solver;
  data  =  engine->data;

  ydata = NV_DATA_S(y);
  dqdata = NV_DATA_S(qdot);

  /* update ODE variables from CVODE  */  
  for ( i=0; i<data->model->neq; i++ ) data->value[i] = ydata[i];
 
  /* update time */
  data->currenttime = t;

  /* update sensitivities */
  yy = N_VNew_Serial(data->model->neq);
  yS = N_VCloneVectorArray_Serial(data->model->nsens, yy);
  N_VDestroy_Serial(yy);

  /*  At t=0, yS is initialized to 0. In this case, CvodeGetSens
      shouldn't be used as it gives nan's */
  if( t != 0 )
  {
    flag = CVodeGetSens(solver->cvode_mem, t, yS);
    if ( flag < 0 )
    {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_CVODE_MALLOC_FAILED,
			"SUNDIALS_ERROR: CVodeGetSens failed "
			"with flag %d", flag);
      exit(EXIT_FAILURE);
    }
  }  


  /* evaluate quadrature integrand: (y-ydata) * yS_i for each i */
  for(i=0; i<data->model->nsens; i++)
  {
    dqdata[i] = 0.0;
    for(j=0; j<data->model->neq; j++)
      dqdata[i] += evaluateAST(engine->om->vector_v[j], data) *
	NV_Ith_S(yS[i], j);
  }

  N_VDestroyVectorArray_Serial(yS, data->model->nsens);

  return (0);
}


/** @} */
/* End of file */










