/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#ifndef _UNITTEST_UNITTEST_H_
#define _UNITTEST_UNITTEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

Suite *create_suite_ASTIndexNameNode(void);
Suite *create_suite_charBuffer(void);
Suite *create_suite_solverError(void);
Suite *create_suite_util(void);

#endif
