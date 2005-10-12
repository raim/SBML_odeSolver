# $Id: Makefile.am,v 1.8 2005/10/12 12:55:37 raimc Exp $
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
             doc/Makefile.am \
             doc/Makefile.in \
             doc/SBML_odeSolver.1.in \
             examples/ASTIndexTest.c \
             examples/batch_integrate.c \
             examples/ChangingIntegratorInstance.c \
             examples/definedTimeSeries.c \
	     examples/integrate.c \
             examples/Makefile.am \
             examples/Makefile.batch \
	     examples/Makefile.in \
             examples/Makefile.integrate \
             examples/README \
	     examples/simpleIntegratorInstanc.c
