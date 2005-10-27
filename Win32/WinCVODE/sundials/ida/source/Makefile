# -----------------------------------------------------------------
# $Revision: 1.1 $
# $Date: 2005/10/27 13:42:25 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see sundials/ida/LICENSE.
# -----------------------------------------------------------------
# Makefile for IDA module
#
# ida/source/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/bash

srcdir       = .
builddir     = .
abs_builddir = /cygdrive/h/SFORGE_CVS/SBML_odeSolver/Win32/WinCVODE/sundials/ida/source
top_builddir = ../../
prefix       = /cygdrive/h/SFORGE_CVS/SBML_odeSolver/Win32/WinCVODE/sundials
exec_prefix  = ${prefix}
includedir   = ${prefix}/include
libdir       = ${exec_prefix}/lib

INSTALL        = /usr/bin/install -c
INSTALL_LIB    = ${INSTALL}
INSTALL_HEADER = ${INSTALL} -m 644

LIBTOOL      = $(SHELL) $(top_builddir)/libtool
LIBTOOL_DEPS = config/ltmain.sh

CPP      = /usr/bin/cc -E
CPPFLAGS = 
CC       = /usr/bin/cc
CFLAGS   = -g -O2
LDFLAGS  = 
LIBS     = -lm 

SHARED_INC_DIR   = $(srcdir)/../../shared/include
IDA_INC_DIR      = $(srcdir)/../include

IDA_LIB       = libsundials_ida.la
IDA_LIB_FILES = ida.lo idaic.lo idaio.lo idadense.lo idaband.lo idaspgmr.lo idabbdpre.lo

OBJECT_FILES = ida.o idaic.o idaio.o idadense.o idaband.o idaspgmr.o idabbdpre.o

all: update $(IDA_LIB_FILES)

update:
	${INSTALL_HEADER} ${top_builddir}/config/sundials_config.h ${builddir}

install: update $(IDA_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(IDA_LIB) $(IDA_LIB_FILES) -rpath $(libdir) $(LDFLAGS) $(LIBS) -avoid-version
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(IDA_LIB) $(libdir)
	$(INSTALL_HEADER) $(IDA_INC_DIR)/ida.h $(includedir)
	$(INSTALL_HEADER) $(IDA_INC_DIR)/idadense.h $(includedir)
	$(INSTALL_HEADER) $(IDA_INC_DIR)/idaband.h $(includedir)
	$(INSTALL_HEADER) $(IDA_INC_DIR)/idaspgmr.h $(includedir)
	$(INSTALL_HEADER) $(IDA_INC_DIR)/idabbdpre.h $(includedir)

clean:
	$(LIBTOOL) --mode=clean rm -f $(IDA_LIB)
	rm -f $(IDA_LIB_FILES)
	rm -f $(OBJECT_FILES)
	rm -f sundials_config.h

uninstall: clean
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(IDA_LIB)
	rm -f $(includedir)/ida.h
	rm -f $(includedir)/idadense.h
	rm -f $(includedir)/idaband.h
	rm -f $(includedir)/idaspgmr.h
	rm -f $(includedir)/idabbdpre.h

ida.lo: $(srcdir)/ida.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/ida.c
idaic.lo: $(srcdir)/idaic.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/idaic.c
idaio.lo: $(srcdir)/idaio.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/idaio.c
idadense.lo: $(srcdir)/idadense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/idadense.c
idaband.lo: $(srcdir)/idaband.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/idaband.c
idaspgmr.lo: $(srcdir)/idaspgmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/idaspgmr.c
idabbdpre.lo: $(srcdir)/idabbdpre.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(IDA_INC_DIR) $(CFLAGS) -c $(srcdir)/idabbdpre.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd $(top_builddir) ;                 \
	 $(SHELL) ./config.status --recheck ; \
	 cd $(abs_builddir)
