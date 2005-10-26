# $Id: Makefile.am,v 1.9 2005/10/26 14:27:42 raimc Exp $
if MAKE_BINDINGS
   SUBDIRS = src odeSolver bindings
else
   SUBDIRS = src odeSolver
endif
CFLAGS = $(AM_CFLAGS) -Wno-unknown-pragmas
EXTRA_DIST = odeSolver/*.c \
             odeSolver/*.h \
             odeSolver/Makefile.am \
             Perlen/AddLegend.pl \
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
	     examples/simpleIntegratorInstance.c
