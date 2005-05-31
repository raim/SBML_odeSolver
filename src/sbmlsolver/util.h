/*
 * Filename    : util.c
 * Revision    : $Id: util.h,v 1.1 2005/05/31 13:54:01 raimc Exp $
 * Source      : $Source: /home/raim/programs/SBML_odeSolver/SBML_odeSolver/src/sbmlsolver/util.h,v $
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
