# -----------------------------------------------------------------
# $Revision: 1.1 $
# $Date: 2005/10/27 13:41:33 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see sundials/ida/LICENSE.
# -----------------------------------------------------------------
# Makefile for IDA serial examples
#
# ida/examples_ser/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/bash

srcdir       = .
builddir     = .
top_builddir = ../../
prefix       = /cygdrive/h/SFORGE_CVS/SBML_odeSolver/Win32/WinCVODE/sundials
exec_prefix  = ${prefix}
includedir   = ${prefix}/include
libdir       = ${exec_prefix}/lib



CPP      = /usr/bin/cc -E
CPPFLAGS = 
CC       = /usr/bin/cc
CFLAGS   = -g -O2
LDFLAGS  = 
LIBS     = -lm 

COMPILER_PREFIX = 
LINKER_PREFIX   = 

SUNDIALS_INC_DIR = $(includedir)
SUNDIALS_LIB_DIR = $(libdir)
SUNDIALS_LIBS    = -lsundials_ida -lsundials_nvecserial -lsundials_shared

EXEC_FILES = irobx iheatsb iheatsk iwebsb

all: $(EXEC_FILES)

irobx: $(srcdir)/irobx.c
	$(COMPILER_PREFIX) $(CC) $(CPPFLAGS) -I$(SUNDIALS_INC_DIR) $(CFLAGS) -c $(srcdir)/irobx.c
	$(LINKER_PREFIX) $(CC) -o $(builddir)/irobx $(builddir)/irobx.o $(LDFLAGS) -L$(SUNDIALS_LIB_DIR) $(SUNDIALS_LIBS) $(LIBS) 
iheatsb: $(srcdir)/iheatsb.c
	$(COMPILER_PREFIX) $(CC) $(CPPFLAGS) -I$(SUNDIALS_INC_DIR) $(CFLAGS) -c $(srcdir)/iheatsb.c
	$(LINKER_PREFIX) $(CC) -o $(builddir)/iheatsb $(builddir)/iheatsb.o $(LDFLAGS) -L$(SUNDIALS_LIB_DIR) $(SUNDIALS_LIBS) $(LIBS) 
iheatsk: $(srcdir)/iheatsk.c
	$(COMPILER_PREFIX) $(CC) $(CPPFLAGS) -I$(SUNDIALS_INC_DIR) $(CFLAGS) -c $(srcdir)/iheatsk.c
	$(LINKER_PREFIX) $(CC) -o $(builddir)/iheatsk $(builddir)/iheatsk.o $(LDFLAGS) -L$(SUNDIALS_LIB_DIR) $(SUNDIALS_LIBS) $(LIBS) 
iwebsb: $(srcdir)/iwebsb.c
	$(COMPILER_PREFIX) $(CC) $(CPPFLAGS) -I$(SUNDIALS_INC_DIR) $(CFLAGS) -c $(srcdir)/iwebsb.c
	$(LINKER_PREFIX) $(CC) -o $(builddir)/iwebsb $(builddir)/iwebsb.o $(LDFLAGS) -L$(SUNDIALS_LIB_DIR) $(SUNDIALS_LIBS) $(LIBS) 

clean:
	rm -rf .libs
	rm -f *.lo
	rm -f *.o

uninstall: clean
	rm -f $(EXEC_FILES)
