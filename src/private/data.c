/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "private/data.h"

#include "sbmlsolver/util.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double scan_double(const char *str)
{
  char *endp;
  double v;

  errno = 0;
  v = strtod(str, &endp);
  if (str == endp) { /* no conversion is performed */
    fatal(stderr, "could not convert %s to double", str);
  }
  if (errno != 0) {
    if (v == HUGE_VAL || v == -HUGE_VAL) {
      fatal(stderr, "overflow found: %s", str);
    } else if (v == 0) {
      fatal(stderr, "underflow found: %s", str);
    }
  }
  return v;
}
