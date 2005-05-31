/**
 * Filename    : util.c
 * Revision    : $Id: util.c,v 1.2 2005/05/31 13:54:00 raimc Exp $
 * Source      : $Source: /home/raim/programs/SBML_odeSolver/SBML_odeSolver/src/util.c,v $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

/* own header files */
#include "sbmlsolver/util.h"

/*-------------------------------------------------------------------------*/
void nrerror(const char message[])
{
  fprintf(stderr, "\n%s\n", message);
  exit(EXIT_FAILURE);
}

/*-------------------------------------------------------------------------*/
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

#ifdef WITH_DMALLOC
#include "dmalloc.h"
#define space(S) calloc(1,(S))
#endif

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

/*-------------------------------------------------------------------------*/
char *get_line(FILE *fp)
{
  char s[512], *line, *cp;
  
  line = NULL;
  do {
    if (fgets(s, 512, fp)==NULL) break;
    cp = strchr(s, '\n');
    if (cp != NULL) *cp = '\0';
    if (line==NULL)
      line = space(strlen(s)+1);
    else
      line = (char *) xrealloc(line, strlen(s)+strlen(line)+1);
    strcat(line, s);
  } while(cp==NULL);
  
  return line;
}

/*-------------------------------------------------------------------------*/

char*
concat (char *a, char *b)
{
  char *tmp = NULL;
  tmp = (char *)xalloc(strlen(a)+strlen(b)+2, sizeof(char));
  strcpy(tmp, a);
  if (tmp[strlen(a)-1] != '/') strcat(tmp, "/");
  strcat(tmp, b);
  return (tmp);
}

void fatal (FILE *hdl, char *fmt, ...)
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
                                                                               
void Warn (FILE *hdl, char *fmt, ...)
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
  ptr = NULL;
}

/* End of file */
