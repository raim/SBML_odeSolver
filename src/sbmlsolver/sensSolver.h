/*
  Last changed Time-stamp: <2005-12-16 16:27:25 raim>
  $Id: sensSolver.h,v 1.5 2006/01/16 16:17:22 jamescclu Exp $
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
 *     Andrew Finney
 */

#ifndef _SENSSOLVER_H_
#define _SENSSOLVER_H_

#include "sbmlsolver/exportdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* CVODES SOLVER */  
  SBML_ODESOLVER_API void IntegratorInstance_printCVODESStatistics(integratorInstance_t *, FILE *f);

  /* internal functions that are not part of the API (yet?) */
  int IntegratorInstance_getForwardSens(integratorInstance_t *);
  int IntegratorInstance_createCVODESSolverStructures(integratorInstance_t *);

  int IntegratorInstance_getAdjSens(integratorInstance_t *);


#ifdef __cplusplus
}
#endif

#endif

/* End of file */
