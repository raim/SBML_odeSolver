# $Id: Makefile.am,v 1.1 2005/05/30 19:49:13 raimc Exp $
INCLUDES = -I. -I.. @SBML_CFLAGS@
AM_CFLAGS  = @CFLAGS@ --pedantic
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
bin_PROGRAMS = integrate batchIntegrate
integrate_SOURCES = integrate.c
integrate_LDADD   = -L../src -lODES \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    -lstdc++ -lxerces-c -lm
batchIntegrate_SOURCES = batch_integrate.c
batchIntegrate_LDADD   = -L../src -lODES \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                         -lstdc++ -lxerces-c -lm
