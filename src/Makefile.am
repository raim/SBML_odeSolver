# $Id: Makefile.am,v 1.14 2005/10/26 12:36:50 raimc Exp $
#if MAKE_INVERSE
#   SUBDIRS = . inverse
#endif
pkgconfigdir = $(libdir)/pkgconfig
pkgincludedir = @includedir@/sbmlsolver
INCLUDES = @CVODE_CFLAGS@ \
           @GRAPHVIZ_CFLAGS@ \
           @SBML_CFLAGS@ \
           @GRACE_CFLAGS@ \
           -I.
LDADD = @SBML_LDFLAGS@ @SBML_LIBS@ \
        @CVODE_LDFLAGS@ @CVODE_LIBS@ \
        @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
        @GRACE_LDFLAGS@ @GRACE_LIBS@ \
        -lxerces-c -lm
AM_LDFLAGS = @GRAPHVIZ_RPATH@ \
             @SBML_RPATH@
lib_LIBRARIES = libODES.a

libODES_a_SOURCES = ASTIndexNameNode.cpp \
		    commandLine.c \
		    cvodedata.c \
		    drawGraph.c \
		    cvodeSolver.c \
		    integratorInstance.c \
		    integratorSettings.c \
		    interactive.c \
		    modelSimplify.c \
		    odeConstruct.c \
		    odeModel.c \
		    odeSolver.c \
		    options.c \
		    printModel.c \
		    processAST.c \
		    sbml.c \
		    sbmlResults.c \
		    solverError.c \
		    util.c
pkginclude_HEADERS = sbmlsolver/ASTIndexNameNode.h \
		     sbmlsolver/commandLine.h \
		     sbmlsolver/config.h \
		     sbmlsolver/cvodedata.h \
		     sbmlsolver/cvodedatatype.h \
		     sbmlsolver/drawGraph.h \
		     sbmlsolver/exportdefs.h \
		     sbmlsolver/cvodeSolver.h \
		     sbmlsolver/integratorInstance.h \
		     sbmlsolver/integratorSettings.h \
		     sbmlsolver/interactive.h \
		     sbmlsolver/modelSimplify.h \
		     sbmlsolver/odeConstruct.h \
		     sbmlsolver/odemodeldatatype.h \
		     sbmlsolver/odeModel.h \
		     sbmlsolver/odeSolver.h \
		     sbmlsolver/options.h \
		     sbmlsolver/printModel.h \
		     sbmlsolver/processAST.h \
		     sbmlsolver/sbml.h \
		     sbmlsolver/sbmlResults.h \
		     sbmlsolver/solverError.h \
		     sbmlsolver/util.h \
		     sbmlsolver/variableIndex.h
pkgconfig_DATA = libODES.pc
bin_PROGRAMS = odeSolver
odeSolver_SOURCES = main.c
odeSolver_LDADD   = -L. -lODES \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    -lxerces-c -lm
