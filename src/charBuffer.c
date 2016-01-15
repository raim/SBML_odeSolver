/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sbmlsolver/charBuffer.h"

#include "private/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** a buffer of unbounded length */
struct charBuffer
{
  char *s;
  char *p;
  size_t length;
};

static void *realloc_or_die(void *ptr, size_t size)
{
  void *p = realloc(ptr, size);
  if (!p) report_error_and_die("failed to realloc");
  return p;
}

/** create an unbounded buffer */
charBuffer_t *CharBuffer_create(void)
{
  charBuffer_t *buf;

  buf = calloc(1, sizeof(*buf));
  if (!buf) report_error_and_die("failed to calloc");
  return buf;
}

/** free an unbounded buffer */
void CharBuffer_free(charBuffer_t *buffer)
{
  if (!buffer) return;
  free(buffer->s);
  free(buffer->p);
  free(buffer);
}

/** add the given string to the end of the unbounded buffer */
void CharBuffer_append(charBuffer_t *buffer, const char *s)
{
  size_t len;
  char *p;

  if (!s) {
    /* nothing to do */
    return;
  }
  len = strlen(s);
  if (len == 0) return;
  p = realloc_or_die(buffer->p, buffer->length + len);
  memcpy(p + buffer->length, s, len);
  buffer->p = p;
  buffer->length += len;
}

/** add the given integer in decimal string form to the end of the given unbounded buffer */
void CharBuffer_appendInt(charBuffer_t *buffer, int i)
{
  int len;
  char *p;

  p = realloc_or_die(buffer->p, buffer->length + 32);
  len = sprintf(p + buffer->length, "%d", i);
  buffer->p = p;
  buffer->length += len;
}

/** add the given double in scientific string form to the end of the given unbounded buffer */
void CharBuffer_appendDouble(charBuffer_t *buffer, double f)
{
  int len;
  char *p;

  p = realloc_or_die(buffer->p, buffer->length + 32);
  len = sprintf(p + buffer->length, "%g", f);
  buffer->p = p;
  buffer->length += len;
}

/** return the string contained in the buffer.  This string must not be freed */
const char *CharBuffer_getBuffer(charBuffer_t *buffer)
{
  char *s;

  if (!buffer->p) return "";
  s = realloc_or_die(buffer->s, buffer->length + 1);
  memcpy(s, buffer->p, buffer->length);
  s[buffer->length] = '\0';
  buffer->s = s;
  return buffer->s;
}
