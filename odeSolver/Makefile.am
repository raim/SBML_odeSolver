# $Id: Makefile.am,v 1.1 2005/10/26 14:27:42 raimc Exp $
INCLUDES = -I. -I../src -I@prefix@/include @SBML_CFLAGS@ @GRACE_CFLAGS@ @CVODE_CFLAGS@
AM_CFLAGS  = @CFLAGS@ --pedantic
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
bin_PROGRAMS = odeSolver
odeSolver_SOURCES = main.c \
		    commandLine.c \
		    options.c  \
		    printModel.c \
		    interactive.c
odeSolver_LDADD   = -L../src -L/@prefix@/lib -lODES \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    -lstdc++ -lxerces-c -lm
