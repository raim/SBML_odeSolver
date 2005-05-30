/*
  Last changed Time-stamp: <2005-05-30 14:28:53 raim>
  $Id: batchIntegrator.c,v 1.1 2005/05/30 19:49:12 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sbml/SBMLTypes.h>

#include "util.h"
#include "options.h"
#include "cvodedata.h"
#include "odeConstruct.h"
#include "odeIntegrate.h"
#include "odeSolver.h"


void 
batchIntegrator(Model_t *m) {

  int j, l, steps, flag;
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
    
    Model_setValue(m, Opt.Parameter, l*(value/steps));
    data = constructODEs(m);

    if ( data->errors > 0 ) {
      fatal(stderr, "%s:%d batchIntegrator.c(): "
	    "Can't construct ODEs for Model >%s/%s<",
	    __FILE__, __LINE__, Opt.ModelPath, Opt.ModelFile);
    }

    if ( l == 0 ) {
      fprintf(outfile, "#%s ", Opt.Parameter);
      
      for ( j=0; j<data->neq; j++ ) {
        fprintf(outfile, "%s ", data->species[j]);
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

    flag = integrator(data);

    if ( flag > -1 ) {
      /* print out values at last time step */
      fprintf(stderr, "Batch run nr. %d finished.\n", l+1);
      fprintf(stderr, "Writing results to %s.\n", filename);
      fprintf(outfile, "%g ", l*(value/steps));
      for ( j=0; j<data->neq; j++ ) {
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
    else {
      Warn(stderr, "CVODE failed with flag %d for %s = %g !",
	   flag, Opt.Parameter, l*(value/steps));
      fprintf(outfile, "# CVODE failed with flag %d for %s = %g\n",
	      flag, Opt.Parameter, l*(value/steps));
    }
    CvodeData_free(data);
  }
  return;
  
}




/* End of file */
