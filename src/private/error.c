/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "private/error.h"

#include <stdio.h>
#include <stdlib.h>

void report_error_and_die(const char *message)
{
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}
