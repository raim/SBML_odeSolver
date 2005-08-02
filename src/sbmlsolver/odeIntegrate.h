/*
  Last changed Time-stamp: <2005-08-01 16:52:21 raim>
  $Id: odeIntegrate.h,v 1.4 2005/08/02 13:20:32 raimc Exp $
*/
#ifndef _INTEGRATOR_H_
#define _INTEGRATOR_H_

#include "sbmlsolver/integratorInstance.h"

int
integrator(cvodeData_t *data, int PrintMessage,
	   int PrintOnTheFly, FILE *outfile);

#endif

/* End of file */
