# $Id: Makefile.am,v 1.4 2005/08/02 13:10:52 raimc Exp $
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
	     examples/integrate.c examples/batch_integrate.c \
             examples/Makefile.am examples/Makefile.integrate \
             examples/Makefile.batch examples/README \
	     examples/simpleIntegratorInstance.c \
	     examples/SharingIntegratorInstance.c \
	     examples/Makefile.in
