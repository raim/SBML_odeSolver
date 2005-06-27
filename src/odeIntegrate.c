/*
<<<<<<< odeIntegrate.c
  Last changed Time-stamp: <2005-06-14 11:29:00 raim>
  $Id: odeIntegrate.c,v 1.5 2005/06/27 15:12:19 afinney Exp $
=======
  Last changed Time-stamp: <2005-05-31 12:26:59 raim>
  $Id: odeIntegrate.c,v 1.5 2005/06/27 15:12:19 afinney Exp $
>>>>>>> 1.3
  Last changed Time-stamp: <2004-12-23 16:16:19 xtof>
  $Id: odeIntegrate.c,v 1.5 2005/06/27 15:12:19 afinney Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sbml/SBMLTypes.h>
#include <sbml/common/common.h> 

/* Header Files for CVODE */
#include "cvode.h"    
#include "cvdense.h"  
#include "dense.h"

/* own header files */
#include "sbmlsolver/util.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/processAST.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"

/* static void */
/* checkData(CvodeData data); */
static int
calculateResults(CvodeData data);
static int
updateModel(CvodeData data, int nout);

/************************ Integrator Program *************************/
/**
  The function "static int integrator(CvodeData data)" gets a filled
  structure "CvodeData" and calls CVODE to integrate the ODEs,
  represented as libsbml Abstract Syntax Trees.  The CvodeData also
  contains the number of equations neq, the nout to which CVODE should
  integrate (starting from time 0), an array of neq species names for
  which ODEs are defined and an array of neq, intial values for the
  species.  
*/

int
integrator(CvodeData data)
{
  integratorInstance_t *engine;
  
  /*
       This now handled simply by not performing the cvode integration calls in the
       integrateOneStep function
  /* models can only be defined by assignments, or constants.
     Such models don't need integration!     
  
  if ( data->neq == 0 ) {
    return calculateResults(data);
  }
  */

  engine = IntegratorInstance_createFromCvodeData(data);

  RETURN_ON_ERRORS_WITH(1);

  /*
    In loop over output points, call CVode, test for error
    and print results at specified intervals into results
    structure. If option '-f' or '--onthefly' was set, then
    print results immediately to the given outfile (default:
    stdout).
  */
  
  while (!IntegratorInstance_timeCourseCompleted(engine))
  {
      if (!IntegratorInstance_integrateOneStep(engine))
          return IntegratorInstance_handleError(engine);
  }

  if ( !data->PrintOnTheFly && data->PrintMessage ) {
    fprintf(stderr,
	    "finished. Results stored.\n");
  }

  if ( data->PrintMessage )
    IntegratorInstance_printStatistics(engine);        /* Print some final statistics   */

  IntegratorInstance_freeExcludingCvodeData(engine);

  return 0;
} 

/** writes current simulation data to
    original model */
static int
updateModel(CvodeData data, int nout) {

  int i;
  Species_t *s;
  Compartment_t *c;
  Parameter_t *p;  
  
  for ( i=0; i<data->model->neq; i++ ) {
    if ( (s = Model_getSpeciesById(data->model->m, data->model->species[i])) != NULL ) {
      Species_setInitialConcentration(s,
				      data->results->value[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->model->m, data->model->species[i])) !=
	      NULL ) {
      Compartment_setSize(c, data->results->value[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->model->m, data->model->species[i])) !=
	      NULL ) {
      Parameter_setValue(p, data->results->value[i][nout]);
    }
  }
  for ( i=0; i<data->model->nass; i++ ) {
    if ( (s = Model_getSpeciesById(data->model->m, data->model->ass_parameter[i]))
	 != NULL ) {
      Species_setInitialConcentration(s,
				      data->results->avalue[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->model->m, data->model->ass_parameter[i]))
	      != NULL ) {
      Compartment_setSize(c, data->results->avalue[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->model->m, data->model->ass_parameter[i]))
	      != NULL ) {
      Parameter_setValue(p, data->results->avalue[i][nout]);
    }
  }
  for ( i=0; i<data->model->nconst; i++ ) {
    if ( (s = Model_getSpeciesById(data->model->m, data->model->parameter[i]))
	 != NULL ) {
      Species_setInitialConcentration(s,
				      data->results->pvalue[i][nout]);
    }
    else if ( (c = Model_getCompartmentById(data->model->m, data->model->parameter[i]))
	      != NULL ) {
      Compartment_setSize(c, data->results->pvalue[i][nout]);
    }
    else if ( (p = Model_getParameterById(data->model->m, data->model->parameter[i]))
	      != NULL ) {
      Parameter_setValue(p, data->results->pvalue[i][nout]);
    }
  }
  return 1;

}
/* static void */
/* checkData(CvodeData data) { */

/*   int i, j; */
/*   double result; */
  
/*   for ( i=0; i<data->neq; i++ ) { */
/*     result = evaluateAST(data->ode[i], data); */
/*     if ( isnan(result) ) { */
/*       fprintf(stderr,
	 "ODE for species %s is not a number.\n", data->species[i]); */
/*     } */
/*   } */
/*   if ( data->jacob != NULL ){ */
/*     for ( i=0; i<data->neq; i++ ) { */
/*       for ( j=0; j<data->neq; j++ ) { */
/* 	result = evaluateAST(data->jacob[i][j], data); */
/* 	if ( isnan(result) ) { */
/* 	  fprintf(stderr, "Jacobian (d[%s]/dt) / d[%s] is not a number.\n", */
/* 		  data->species[i], */
/* 		  data->species[j]); */
/* 	} */
/*       } */
/*     } */
/*   } */
/* } */
  


/* End of file */
