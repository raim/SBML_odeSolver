# $Id: Makefile.am,v 1.4 2005/06/15 12:58:36 raimc Exp $
#if MAKE_INVERSE
#   SUBDIRS = . inverse
#endif
pkgincludedir = @includedir@/sbmlsolver
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
pkginclude_HEADERS = sbmlsolver/odeSolver.h sbmlsolver/util.h \
                     sbmlsolver/options.h sbmlsolver/sbml.h \
                     sbmlsolver/modelSimplify.h \
                     sbmlsolver/odeConstruct.h sbmlsolver/processAST.h \
                     sbmlsolver/odeIntegrate.h  sbmlsolver/printModel.h \
                     sbmlsolver/drawGraph.h  sbmlsolver/interactive.h \
                     sbmlsolver/cvodedata.h sbmlsolver/batchIntegrator.h \
		     sbmlsolver/sbmlResults.h sbmlsolver/exportdefs.h
bin_PROGRAMS = odeSolver
odeSolver_SOURCES = main.c
odeSolver_LDADD   = -L. -lODES \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    -lxerces-c -lm
