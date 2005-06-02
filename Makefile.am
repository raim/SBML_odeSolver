# $Id: Makefile.am,v 1.3 2005/06/02 14:43:42 raimc Exp $
if MAKE_BINDINGS
   SUBDIRS = src bindings
else
   SUBDIRS = src
endif
CFLAGS = $(AM_CFLAGS) -Wno-unknown-pragmas
EXTRA_DIST = Perlen/AddLegend.pl \
             sbml/*.xsd \
             Test/MAPK.xml Test/simple.sh \
             doc/os.pdf \
	     examples/integrate.c examples/batch_integrate.c \
             examples/Makefile.am examples/Makefile.integrate \
             examples/Makefile.batch examples/README \
	     examples/Makefile.in
