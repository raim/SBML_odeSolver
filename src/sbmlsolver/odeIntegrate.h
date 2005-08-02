/*
  Last changed Time-stamp: <2005-08-02 17:22:18 raim>
  $Id: odeIntegrate.h,v 1.5 2005/08/02 15:47:59 raimc Exp $
*/
#ifndef _INTEGRATOR_H_
#define _INTEGRATOR_H_

#include "sbmlsolver/integratorInstance.h"

int integrator(cvodeData_t *data, int PrintMessage,
	       int PrintOnTheFly, FILE *outfile);
int handleError(integratorInstance_t *engine,
		int PrintMessage, int PrintOnTheFly, FILE *f);
#endif

/* End of file */
