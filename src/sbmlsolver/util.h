/*
 * Filename    : util.c
 * Revision    : $Id: util.h,v 1.2 2005/10/26 15:32:13 raimc Exp $
 * Source      : $Source: /home/raim/programs/SBML_odeSolver/SBML_odeSolver/src/sbmlsolver/util.h,v $
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
 *     Christoph Flamm
 *
 * Contributor(s):
 *     
 */

#ifndef _UTIL_H_
#define _UTIL_H_

extern void nrerror(const char message[]);

#ifdef WITH_DMALLOC
/* use dmalloc library to check for memory management bugs */
#include "dmalloc.h"
#define space(S) calloc(1,(S))
#else
extern void  *space(unsigned size);
extern void  *xrealloc(void *p, unsigned size);
#endif

char *get_line(FILE *fp);
                                                                               
char *concat (char *a, char *b);

void fatal (FILE *hdl, char *fmt, ...);
void Warn  (FILE *hdl, char *fmt, ...);

void *xalloc (size_t nmemb, size_t size);
void xfree (void *ptr);

#endif
                                                                               
/* End of file */
