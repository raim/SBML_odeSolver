# -----------------------------------------------------------------
# $Revision: 1.1 $
# $Date: 2005/10/27 13:45:31 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see sundials/shared/LICENSE.
# -----------------------------------------------------------------
# Makefile for serial NVECTOR module
#
# nvec_ser/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/bash

srcdir       = .
builddir     = .
abs_builddir = /cygdrive/h/SFORGE_CVS/SBML_odeSolver/Win32/WinCVODE/sundials/nvec_ser
top_builddir = ../
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
LIBS     = -lm 

FCMIX_ENABLED = yes

SHARED_INC_DIR         = $(srcdir)/../shared/include
NVECTOR_SERIAL_INC_DIR = $(srcdir)

NVECTOR_SERIAL_LIB       = libsundials_nvecserial.la
NVECTOR_SERIAL_LIB_FILES = nvector_serial.lo

FNVECTOR_SERIAL_LIB       = libsundials_fnvecserial.la
FNVECTOR_SERIAL_LIB_FILES = fnvector_serial.lo

all: update $(NVECTOR_SERIAL_LIB_FILES) $(FNVECTOR_SERIAL_LIB_FILES)

update:
	${INSTALL_HEADER} ${top_builddir}/config/sundials_config.h ${builddir}

install: update $(NVECTOR_SERIAL_LIB_FILES) $(FNVECTOR_SERIAL_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(NVECTOR_SERIAL_LIB) $(NVECTOR_SERIAL_LIB_FILES) -rpath $(libdir) $(LIBS) -avoid-version
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(NVECTOR_SERIAL_LIB) $(libdir)
	$(INSTALL_HEADER) $(NVECTOR_SERIAL_INC_DIR)/nvector_serial.h $(includedir)
	@if test "X${FCMIX_ENABLED}" = "Xyes"; then                                                                                                               \
	   echo "${LIBTOOL} --mode=link ${CC} ${CFLAGS} -o ${FNVECTOR_SERIAL_LIB} ${FNVECTOR_SERIAL_LIB_FILES} -rpath ${libdir} ${LIBS} -static -avoid-version" ; \
	   ${LIBTOOL} --mode=link ${CC} ${CFLAGS} -o ${FNVECTOR_SERIAL_LIB} ${FNVECTOR_SERIAL_LIB_FILES} -rpath ${libdir} ${LIBS} -static -avoid-version ;        \
	   echo "${LIBTOOL} --mode=install ${INSTALL_LIB} ${FNVECTOR_SERIAL_LIB} ${libdir}" ;                                                                     \
	   ${LIBTOOL} --mode=install ${INSTALL_LIB} ${FNVECTOR_SERIAL_LIB} ${libdir} ;                                                                            \
	 fi

clean:
	$(LIBTOOL) --mode=clean rm -f $(NVECTOR_SERIAL_LIB)
	rm -f $(NVECTOR_SERIAL_LIB_FILES)
	rm -f nvector_serial.o
	@if test "X${FCMIX_ENABLED}" = "Xyes"; then                      \
	   echo "${LIBTOOL} --mode=clean rm -f ${FNVECTOR_SERIAL_LIB}" ; \
	   ${LIBTOOL} --mode=clean rm -f ${FNVECTOR_SERIAL_LIB} ;        \
	   echo "rm -f ${FNVECTOR_SERIAL_LIB_FILES}" ;                   \
	   rm -f ${FNVECTOR_SERIAL_LIB_FILES} ;                          \
	   echo "rm -f fnvector_serial.o" ;                              \
	   rm -f fnvector_serial.o ;                                     \
	 fi
	rm -f sundials_config.h

uninstall: clean
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(NVECTOR_SERIAL_LIB)
	rm -f $(includedir)/nvector_serial.h
	@if test "X${FCMIX_ENABLED}" = "Xyes"; then                                    \
	   echo "${LIBTOOL} --mode=uninstall rm -f ${libdir}/${FNVECTOR_SERIAL_LIB}" ; \
	   ${LIBTOOL} --mode=uninstall rm -f ${libdir}/${FNVECTOR_SERIAL_LIB} ;        \
	 fi

nvector_serial.lo: $(srcdir)/nvector_serial.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) -I. -I$(SHARED_INC_DIR) -I$(NVECTOR_SERIAL_INC_DIR) $(CFLAGS) -c $(srcdir)/nvector_serial.c
fnvector_serial.lo: $(srcdir)/fnvector_serial.c
	@if test "X${FCMIX_ENABLED}" = "Xyes"; then                                                                                                          \
	   echo "${LIBTOOL} --mode=compile ${CC} ${CPPFLAGS} -I. -I${SHARED_INC_DIR} -I${NVECTOR_SERIAL_INC_DIR} ${CFLAGS} -c ${srcdir}/fnvector_serial.c" ; \
	   ${LIBTOOL} --mode=compile ${CC} ${CPPFLAGS} -I. -I${SHARED_INC_DIR} -I${NVECTOR_SERIAL_INC_DIR} ${CFLAGS} -c ${srcdir}/fnvector_serial.c ;        \
	 fi

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd $(top_builddir) ;                 \
	 $(SHELL) ./config.status --recheck ; \
	 cd $(abs_builddir)
