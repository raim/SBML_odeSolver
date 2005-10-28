/*
  Last changed Time-stamp: <2005-10-26 17:38:04 raim>
  $Id: commandLine.h,v 1.3 2005/10/28 09:05:53 afinney Exp $
*/
/* 
 *
 * This application is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This application  is distributed in the hope that it will be useful, but
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
 *     Christoph Flamm
 */

#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#include <sbml/SBMLTypes.h>

#include "../src/sbmlsolver/exportdefs.h"
#include "../src/sbmlsolver/integratorInstance.h"

SBMLDocument_t* parseModelWithArguments(char *file);
int integrator(integratorInstance_t *engine, int PrintMessage,
	       int PrintOnTheFly, FILE *outfile);

int odeSolver (int argc, char *argv[]);

#endif

/* End of file */
