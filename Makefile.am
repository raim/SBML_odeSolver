# $Id: Makefile.am,v 1.2 2005/05/31 13:55:43 raimc Exp $
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
	     examples/integrate.c examples/batchintegrate.c \
             examples/Makefile.am examples/Makefile.integrate \
             examples/Makefile.batch examples/README
