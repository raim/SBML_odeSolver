# -----------------------------------------------------------------
# $Revision: 1.1 $
# $Date: 2005/10/27 13:22:58 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see sundials/cvode/LICENSE.
# -----------------------------------------------------------------
# Makefile for FCVODE module
#
# cvode/fcmix/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/bash

srcdir       = .
builddir     = .
abs_builddir = /cygdrive/h/SFORGE_CVS/SBML_odeSolver/Win32/WinCVODE/sundials/cvode/fcmix
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
CVODE_INC_DIR    = $(srcdir)/../include
FCVODE_INC_DIR   = $(srcdir)

FCVODE_LIB       = libsundials_fcvode.la
FCVODE_LIB_FILES = fcvode.lo fcvband.lo fcvdense.lo fcvjtimes.lo fcvpreco.lo fcvbbd.lo fcvbp.lo fcvroot.lo fcvewt.lo

OBJECT_FILES = fcvode.o fcvband.o fcvdense.o fcvjtimes.o fcvpreco.o fcvbbd.o fcvbp.o fcvroot.o fcvewt.o

all: update $(FCVODE_LIB_FILES)

update:
	${INSTALL_HEADER} ${top_builddir}/config/sundials_config.h ${builddir}

install: update $(FCVODE_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(FCVODE_LIB) $(FCVODE_LIB_FILES) -rpath $(libdir) $(LDFLAGS) $(LIBS) -static -avoid-version
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(FCVODE_LIB) $(libdir)

clean:
	$(LIBTOOL) --mode=clean rm -f $(FCVODE_LIB)
	rm -f $(FCVODE_LIB_FILES)
	rm -f $(OBJECT_FILES)
	rm -f sundials_config.h

uninstall: clean
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(FCVODE_LIB)

fcvode.lo: $(srcdir)/fcvode.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvode.c
fcvewt.lo: $(srcdir)/fcvewt.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvewt.c
fcvband.lo: $(srcdir)/fcvband.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvband.c
fcvdense.lo: $(srcdir)/fcvdense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvdense.c
fcvjtimes.lo: $(srcdir)/fcvjtimes.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvjtimes.c
fcvpreco.lo: $(srcdir)/fcvpreco.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvpreco.c
fcvbbd.lo: $(srcdir)/fcvbbd.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvbbd.c
fcvbp.lo: $(srcdir)/fcvbp.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvbp.c
fcvroot.lo: $(srcdir)/fcvroot.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(CVODE_INC_DIR) -I$(FCVODE_INC_DIR) $(CFLAGS) -c $(srcdir)/fcvroot.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd $(top_builddir) ;                 \
	 $(SHELL) ./config.status --recheck ; \
	 cd $(abs_builddir)
