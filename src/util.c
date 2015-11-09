/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * Filename    : util.c
 * Revision    : $Id: util.c,v 1.4 2008/09/12 20:02:49 raimc Exp $
 * Source      : $Source: /home/raim/programs/SBML_odeSolver/SBML_odeSolver/src/util.c,v $
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

/* own header files */
#include "sbmlsolver/util.h"

/*-------------------------------------------------------------------------*/
static void nrerror(const char message[])
{
  fprintf(stderr, "\n%s\n", message);
  exit(EXIT_FAILURE);
}

#ifndef WITH_DMALLOC

void *space(unsigned size) {
  void *pointer;
  
  if ( (pointer = (void *) calloc(1, (size_t) size)) == NULL) {
#ifdef EINVAL
    if (errno==EINVAL) {
      fprintf(stderr,"SPACE: requested size: %d\n", size);
      nrerror("SPACE allocation failure -> EINVAL");
    }
    if (errno==ENOMEM)
#endif
      nrerror("SPACE allocation failure -> no memory");
  }
  return  pointer;
}

#undef xrealloc
/* dmalloc.h #define's xrealloc */
void *xrealloc (void *p, unsigned size) {
  if (p == 0)
    return space(size);
  p = (void *) realloc(p, size);
  if (p == NULL) {
#ifdef EINVAL
    if (errno==EINVAL) {
      fprintf(stderr,"xrealloc: requested size: %d\n", size);
      nrerror("xrealloc allocation failure -> EINVAL");
    }
    if (errno==ENOMEM)
#endif
      nrerror("xrealloc allocation failure -> no memory");  
  }
  return p;
}

#endif

/*-------------------------------------------------------------------------*/
char *get_line(FILE *fp)
{
  static const size_t BUFFER_SIZE = 512;
  char *line, *cp, *ep;
  size_t len;

  line = space(BUFFER_SIZE);
  line[0] = '\0';
  ep = line;
  len = BUFFER_SIZE;
  if ( fgets(ep, BUFFER_SIZE, fp) == NULL) {
    xfree(line);
    return NULL;
  }
  do {
    cp = strchr(ep, '\n');
    if (cp) {
      *cp = '\0';
      break;
    }
    if (strlen(ep) < BUFFER_SIZE - 1) break;
    line = xrealloc(line, len + BUFFER_SIZE - 1);
    ep = &line[len - 1];
    len += BUFFER_SIZE - 1;
  } while ( fgets(ep, BUFFER_SIZE, fp) != NULL);
  return line;
}

/*-------------------------------------------------------------------------*/

char*
concat(const char *a, const char *b)
{
  size_t alen, blen;
  char *tmp;

  alen = strlen(a);
  blen = strlen(b);
  tmp = (char *)xalloc(alen+blen+2, sizeof(char));
  strcpy(tmp, a);
  if (alen == 0 || tmp[alen-1] != '/') tmp[alen] = '/';
  strcat(tmp, b);
  return (tmp);
}

void fatal(FILE *hdl, const char *fmt, ...)
{
  va_list args;
  if ( hdl == NULL ) hdl = stderr;
  va_start(args, fmt);
  if ( errno != 0 )
    fprintf( hdl, "FATAL ERROR: %s: ", strerror(errno));
  else fprintf(hdl, "FATAL ERROR: ");
  vfprintf(hdl, fmt, args);
  fprintf(hdl,"\n");
  fflush(hdl);
  va_end(args);
  exit(EXIT_FAILURE);
}
                                                                               
void Warn(FILE *hdl, const char *fmt, ...)
{
  va_list args;
  if ( hdl == NULL ) hdl = stderr;
  va_start(args, fmt);
  fprintf(hdl, "WARNING: ");
  vfprintf(hdl, fmt, args);
  fprintf(hdl,"\n");
  fflush(hdl);
  va_end(args);
}

void *xalloc (size_t nmemb, size_t size)
{
  void *tmp;
  if ( (tmp = calloc(nmemb, size)) == NULL )
    fatal(stderr, "xalloc(): %s\n", strerror(ENOMEM));
  return (tmp);
}

/*
 * @return void
 */
void xfree (void *ptr)
{
  if ( ptr == NULL ) {
#ifdef MEMDEBUG
    Warn(stderr, "xfree(): arg 1 is NULL");
#endif
    return;
  }
  free (ptr);
}

/* End of file */
