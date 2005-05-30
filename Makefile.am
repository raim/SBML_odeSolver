# $Id: Makefile.am,v 1.1 2005/05/30 19:49:12 raimc Exp $
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
	     examples/main.c examples/Makefile examples/README
