# $Id: Makefile.am,v 1.5 2005/09/01 14:03:25 chfl Exp $
if MAKE_BINDINGS
   SUBDIRS = src bindings
else
   SUBDIRS = src
endif
CFLAGS = $(AM_CFLAGS) -Wno-unknown-pragmas
EXTRA_DIST = Perlen/AddLegend.pl \
             sbml/*.xsd \
             Test/MAPK.xml Test/simple.sh \
             doc/os_v1-5.pdf \
             examples/ASTIndexTest.c \
             examples/batch_integrate.c \
             examples/ChangingParameterIntegrator.c \
	     examples/integrate.c \
             examples/Makefile.am \
	     examples/Makefile.in \
             examples/Makefile.batch \
             examples/Makefile.integrate \
             examples/ParameterScanner.c \
             examples/README \
	     examples/SharingIntegratorInstance.c \
	     examples/simpleIntegratorInstance.c
