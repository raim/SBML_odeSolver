/*
  Last changed Time-stamp: <2005-08-26 16:39:17 xtof>
  $Id: odeIntegrate.c,v 1.10 2005/09/01 15:31:04 chfl Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/integratorInstance.h"
#include "sbmlsolver/solverError.h"
#include "sbmlsolver/odeIntegrate.h"

/* static void */
/* checkData(cvodeData_t *data); */
/* static int */
/* calculateResults(cvodeData_t *data); */


/************************ Integrator Program *************************/
/**
  The function "static int integrator(cvodeData_t *data)" gets a filled
  structure "CvodeData" and calls CVODE via the integratorInstance group
  of functions, to integrate the ODEs.
*/

int
integrator(cvodeData_t *data, int PrintMessage, int PrintOnTheFly,
	   FILE *outfile)
{
  int i;
  integratorInstance_t *engine;
  
  engine = IntegratorInstance_createFromCvodeData(data);
  RETURN_ON_ERRORS_WITH(1);
  
 /** Command-line option -f/--onthefly:
      print initial values, if on-the-fly printint is set
  */
  if ( PrintOnTheFly && data->run == 0 ) {
	if ( data->t0 == 0.0 )
	{
	  fprintf(stderr, "\nPrinting results on the fly !\n");
      fprintf(stderr, "Overruling all other print options!!\n\n");      
      fprintf(outfile, "#t ");
      for ( i=0; i<data->nvalues; i++ )
        fprintf(outfile, "%s ", data->model->names[i]);
      fprintf(outfile, "\n");
	}

    fprintf(outfile, "%g ", data->t0);
    for ( i=0; i<data->nvalues; i++ )
      fprintf(outfile, "%g ", data->value[i]);
    fprintf(outfile, "\n");
  }
  else {
    if ( PrintMessage )
      fprintf(stderr,"Integrating        ");
  }

  /*
    In loop over output points, call CVode, test for error
    and print results at specified intervals into results
    structure. If option '-f' or '--onthefly' was set, then
    print results immediately to the given outfile (default:
    stdout).
  */
  
  while (!IntegratorInstance_timeCourseCompleted(engine)) {
    if (!IntegratorInstance_integrateOneStep(engine))
      return handleError(engine, PrintMessage, PrintOnTheFly, outfile);
          
    /* print immediately if PrintOnTheFly was set
       with '-d' or '--onthefly'
    */
    if ( PrintOnTheFly ) {
      fprintf(outfile, "%g ", engine->t + engine->data->t0);
      /* fprintf(stdout, "%g ", t + data->t0); */
      for ( i=0; i<engine->data->nvalues; i++ )
	fprintf(outfile, "%g ", engine->data->value[i]);
      fprintf(outfile, "\n");
    }
    else if ( PrintMessage ) {
      const  char chars[5] = "|/-\\";
      fprintf(stderr, "\b\b\b\b\b\b");
      fprintf(stderr, "%.2f %c",
	      (float)(engine->iout-1)/(float)engine->nout,
	      chars[(engine->iout-1) % 4]);
    }
  }
  if ( !PrintOnTheFly && PrintMessage ) {
    fprintf(stderr,
	    "finished. Results stored.\n");
  }

  /* Print some final statistics   */
  if ( PrintMessage )
    IntegratorInstance_printStatistics(engine); 

  IntegratorInstance_freeExcludingCvodeData(engine);

  return 0;

} 
/* standard handler for when the integrate function fails */
int handleError(integratorInstance_t *engine,
		int PrintMessage, int PrintOnTheFly, FILE *outfile) {
    cvodeData_t *data = engine->data;
    int i;
    int errorCode = SolverError_getLastCode(ERROR_ERROR_TYPE) ;

    if ( errorCode ) {
      fprintf(outfile, "# CVode failed, with flag=%d, at time %g\n",
	      errorCode, data->currenttime);        
        /* on flag -7/CV_CONV_FAILURE
        try again, but now with/without generated Jacobian matrix  */
        if ( errorCode == CV_CONV_FAILURE && engine->data->run == 0 &&
	     engine->data->opt->StoreResults) {
            fprintf(
                stderr,
                "Trying again; now with %s Jacobian matrix\n",
                engine->data->opt->UseJacobian ?
                    "CVODE's internal approximation of the" :
                    "automatically generated");
            engine->data->opt->UseJacobian = !engine->data->opt->UseJacobian;
            engine->data->run++;
            for ( i=0; i<engine->data->nvalues; i++ ) {
                engine->data->value[i] = engine->results->value[i][0];
            }

            engine->data->currenttime = engine->data->t0;
            IntegratorInstance_freeExcludingCvodeData(engine);
            SolverError_clear();

            return integrator(data, PrintMessage, PrintOnTheFly, outfile);
        }
        else
            SolverError_dumpAndClearErrors();
    }

    return errorCode ;
}



  


/* End of file */
