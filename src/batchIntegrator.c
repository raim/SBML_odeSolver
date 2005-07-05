/*
  Last changed Time-stamp: <2005-06-08 11:09:42 raim>
  $Id: batchIntegrator.c,v 1.5 2005/07/05 15:30:26 afinney Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>

/* own header files */
#include "sbmlsolver/util.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/odeConstructUsingOptions.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/odeSolver.h"
#include "sbmlsolver/solverError.h"


void 
batchIntegrator(Model_t *m) {

  int j, l, steps;
  double value;
  CvodeData data;
  FILE *outfile;
  char *filename;

  filename = (char *) calloc(strlen(Opt.ModelPath)+
			     strlen(Opt.ModelFile)+5, sizeof(char));
  sprintf(filename, "%s%s.dat", Opt.ModelPath, Opt.ModelFile);

  if ( Opt.Write ) {
    outfile = fopen(filename, "w");
  }
  else {
    outfile = stdout;
  }
  
  Opt.PrintOnTheFly = 0;
  value = Model_getValueById(m, Opt.Parameter);
  steps = 50;
  
  if ( value == 0 ) {
    fatal(stderr, "%s:%d batchIntegrator.c(): "
	  "Parameter not found or (initial) value = 0. >%s/%s<",
	  __FILE__, __LINE__, Opt.ModelPath, Opt.ModelFile);
  }

  fprintf(stderr, "Varying %s %g\n", Opt.Parameter, value);
  
  for ( l=0; l<=steps; l++ ) {
    
    Model_setValue(m, Opt.Parameter, "", l*(value/steps));
    data = constructODEs(m);

    SolverError_haltOnErrors();

    if ( l == 0 ) {
      fprintf(outfile, "#%s ", Opt.Parameter);
      
      for ( j=0; j<data->model->neq; j++ ) {
        fprintf(outfile, "%s ", data->model->species[j]);
      }
      fprintf(outfile, "time ");
      fprintf(outfile, "\n");

      fprintf(outfile, "# Batch Integration to time %g, increasing %s ",
              Opt.Time, Opt.Parameter);
      fprintf(outfile, "from 0 to %g in %d steps\n", value, steps);

    }
    
    /** Now that we have arrived here, we can set the parameters
	for integration, like the end time and the number of
	steps to be printed out. And then ..
    */
    data->tout  = Opt.Time;
    data->nout  = Opt.PrintStep;
    data->tmult = data->tout / data->nout;
    data->currenttime = 0.0;
    data->t0 = 0.0;    
    data->Error = Opt.Error;
    data->RError = Opt.RError;
    data->Mxstep = Opt.Mxstep;
    data->PrintOnTheFly = Opt.PrintOnTheFly;
    data->PrintMessage = Opt.PrintMessage;
    data->HaltOnEvent = Opt.HaltOnEvent;
    data->SteadyState = Opt.SteadyState;
    data->EnableVariableChanges = 0; 

    /* allow setting of Jacobian,
       only if its construction was succesfull */
    if ( data->UseJacobian == 1 ) {
      data->UseJacobian = Opt.Jacobian;
    }
    
    /** .... we can call the integrator function,
	that invokeds CVODE and stores results.
	The function will also handle events and
	check for steady states.
    */

    fprintf(stderr, "Batch run nr. %d; starting integration with ", l+1);
    fprintf(stderr, "Parameter %s = %g\n",
	    Opt.Parameter, l*(value/steps));

    integrator(data);

    if (SolverError_getNum(ERROR_ERROR_TYPE) || SolverError_getNum(FATAL_ERROR_TYPE))
    {
        SolverError_dumpAndClearErrors();
        Warn(stderr, "CVODE failed for %s = %g !",
            Opt.Parameter, l*(value/steps));
        fprintf(outfile, "# CVODE failed for %s = %g\n",
            Opt.Parameter, l*(value/steps));
    }
    else
    {
        /* print out values at last time step */
        fprintf(stderr, "Batch run nr. %d finished.\n", l+1);
        fprintf(stderr, "Writing results to %s.\n", filename);
        fprintf(outfile, "%g ", l*(value/steps));
        for ( j=0; j<data->model->neq; j++ ) {
            fprintf(outfile, "%g ",
                data->results->value[j][data->results->nout]);
        }
        fprintf(outfile, "%g ", data->currenttime);
        fprintf(outfile, "\n");
        if ( data->steadystate == 1 ) {
            fprintf(outfile,
                "# Found steady state; aborted at time %g\n",
                data->currenttime);
        }
        fprintf(stderr, "\n\n");
    }
    CvodeData_free(data);
  }
  return;
  
}




/* End of file */
