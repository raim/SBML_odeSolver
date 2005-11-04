/*
  Last changed Time-stamp: <2005-11-04 17:16:02 raim>
  $Id: sensSolver.c,v 1.7 2005/11/04 16:23:44 raimc Exp $
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
#include "sbmlsolver/cvodeSolver.h"
#include "sbmlsolver/sensSolver.h"


/* 
 * fS routine. Compute sensitivity r.h.s. for param[iS]
 */
static void fS(int Ns, realtype t, N_Vector y, N_Vector ydot, 
               int iS, N_Vector yS, N_Vector ySdot, 
               void *fS_data, N_Vector tmp1, N_Vector tmp2);


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
    
    /* getting sensitivities */
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

    /* realtype pbar[data->nsens+1]; */
    int plist[data->nsens+1];
    realtype pbar[data->nsens+1];

    /*
     * creating CVODE structures
     */
    if (!IntegratorInstance_createCVODESolverStructures(engine))
      return 0;

    /*****  adding sensitivity specific structures ******/

    /*
     * construct sensitivity related structures
     */
    /* free sensitivity from former runs (changed for non-default cases!) */
    if ( om->jacob_sens != NULL ) 
      ODEModel_freeSensitivity(om);
    
    /* this function will require additional input for
       non-default case, via sensitivity input settings! */
    ODEModel_constructSensitivity(om);
    
    engine->solver->nsens = data->nsens;

    solver->yS = N_VNewVectorArray_Serial(data->nsens, data->neq);      
    if (check_flag((void *)solver->yS, "N_VNewVectorArray_Serial",
		   1, stderr))
      return(0);

    /*
     * (re)initialize ySdata sensitivities
     */
    for ( j=0; j<data->nsens; j++ ) {
      ySdata = NV_DATA_S(solver->yS[j]);
      for ( i=0; i<data->neq; i++ ) 
	ySdata[i] = data->sensitivity[i][j];
    }

    /*
     * set method
     */
    if ( opt->SensMethod == 0 ) 
      flag =CVodeSensMalloc(solver->cvode_mem,data->nsens,
			    CV_SIMULTANEOUS,solver->yS);
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

    /***** set parameter values or R.H.S function fS *****/
    /* NOTES: */
    /* !!! plist could later be used to specify requested parameters
       for sens.analysis !!! */
    /* !!! data->p is only required if R.H.S. fS cannot be supplied !!! */
    
    /* was construction of parametric matrix successfull ? */
    if ( om->sensitivity ) {
      flag = CVodeSetSensRhs1Fn(solver->cvode_mem, fS);
      if (check_flag(&flag, "CVodeSetSensRhs1Fn", 1, stderr)) {
	return 0;
	/* ERROR HANDLING CODE if failes */
      }
      data->p = NULL;
      flag = CVodeSetSensParams(solver->cvode_mem, NULL, NULL, NULL);
      if (check_flag(&flag, "CVodeSetSensParams", 1, stderr))  {
	return 0;
	/* ERROR HANDLING CODE if  failes */
      }
    }
    else {
      /*!!! ??? DOESNT WORK CURRENTLY ??? !!!*/
      /* SIGSEV: ... at ./cvodes.c:6501  pbari = pbar[is];*/
      /* return 0; */
      ASSIGN_NEW_MEMORY_BLOCK(data->p, data->nsens, realtype, 0);
      for ( i=0; i<data->nsens; i++ ) {
	plist[i] = i+1;
	data->p[i] = data->value[om->index_sens[i]];
/* 	pbar[i] = abs(data->p[i]);  */ /*??? WHAT IS PBAR ???*/
      }
      flag = CVodeSetSensParams(solver->cvode_mem, data->p, NULL, plist);
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

    /* difference FALSE/TRUE ? */
    flag = CVodeSetSensErrCon(solver->cvode_mem, TRUE);
    if (check_flag(&flag, "CVodeSetSensFdata", 1, stderr)) {
      return 0;
      /* ERROR HANDLING CODE if failes */
    } 
      
    flag = CVodeSetSensFdata(solver->cvode_mem, data);
    if (check_flag(&flag, "CVodeSetSensFdata", 1, stderr))  {
      return 0;
      /* ERROR HANDLING CODE if  failes */
    }      
     
    return 1; /* OK */
}


/** \brief Prints some final statistics of the calls to CVODE routines
*/

SBML_ODESOLVER_API void IntegratorInstance_printCVODESStatistics(integratorInstance_t *engine, FILE *f)
{
  int flag;
  long int nfSe, nfeS, nsetupsS, nniS, ncfnS, netfS;
  cvodeSolver_t *solver = engine->solver;

  /* print CVODE statistics */
  IntegratorInstance_printCVODEStatistics(engine, f);
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


/************* Additional Function for Sensitivity Analysis **************/

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

  /* !!! update parameters: is p modified by CVODES??? !!! */
  /* !!! is only required if fS could not be generated !!! */
   if ( data->p != NULL )
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
