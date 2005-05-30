# $Id: Makefile.am,v 1.1 2005/05/30 19:49:12 raimc Exp $
if MAKE_INVERSE
   SUBDIRS = . inverse
endif
INCLUDES = @CVODE_CFLAGS@ \
           @GRAPHVIZ_CFLAGS@ \
           @SBML_CFLAGS@ \
           @GRACE_CFLAGS@ \
           -I.
CFLAGS  = @CFLAGS@ -Wno-unknown-pragmas
LDADD = @SBML_LDFLAGS@ @SBML_LIBS@ \
        @CVODE_LDFLAGS@ @CVODE_LIBS@ \
        @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
        @GRACE_LDFLAGS@ @GRACE_LIBS@ \
        -lxerces-c -lm
AM_LDFLAGS = @GRAPHVIZ_RPATH@ \
             @SBML_RPATH@
lib_LIBRARIES = libODES.a
libODES_a_SOURCES = odeSolver.c util.c \
                    options.c  sbml.c  \
                    modelSimplify.c  \
                    odeConstruct.c processAST.c \
                    odeIntegrate.c printModel.c \
                    drawGraph.c interactive.c \
                    cvodedata.c batchIntegrator.c \
		    sbmlResults.c
include_HEADERS = odeSolver.h util.h \
                  options.h sbml.h \
                  modelSimplify.h \
                  odeConstruct.h processAST.h \
                  odeIntegrate.h  printModel.h \
                  drawGraph.h  interactive.h \
                  cvodedata.h batchIntegrator.h \
		  sbmlResults.h
bin_PROGRAMS = odeSolver
odeSolver_SOURCES = main.c
odeSolver_LDADD   = -L. -lODES \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    -lxerces-c -lm
