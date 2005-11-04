# $Id: Makefile.am,v 1.19 2005/11/04 19:31:44 raimc Exp $
## 
## This library is free software; you can redistribute it and/or modify it
## under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation; either version 2.1 of the License, or
## any later version.
## 
## This library is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
## MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. The software and
## documentation provided hereunder is on an "as is" basis, and the
## authors have no obligations to provide maintenance, support,
## updates, enhancements or modifications.  In no event shall the
## authors be liable to any party for direct, indirect, special,
## incidental or consequential damages, including lost profits, arising
## out of the use of this software and its documentation, even if the
## authors have been advised of the possibility of such damage.  See
## the GNU Lesser General Public License for more details.
## 
## You should have received a copy of the GNU Lesser General Public License
## along with this library; if not, write to the Free Software Foundation,
## Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
## 
## The original code contained here was initially developed by:
## 
##     Christoph Flamm
## 
## Contributor(s):
##     Rainer Machne
##
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
		    cvodedata.c \
		    drawGraph.c \
		    cvodeSolver.c \
		    sensSolver.c \
		    nullSolver.c \
		    integratorInstance.c \
		    integratorSettings.c \
		    modelSimplify.c \
		    odeConstruct.c \
		    odeModel.c \
		    odeSolver.c \
		    processAST.c \
		    sbml.c \
		    sbmlResults.c \
		    solverError.c \
		    util.c
pkginclude_HEADERS = sbmlsolver/ASTIndexNameNode.h \
		     sbmlsolver/config.h \
		     sbmlsolver/cvodedata.h \
		     sbmlsolver/cvodedatatype.h \
		     sbmlsolver/drawGraph.h \
		     sbmlsolver/exportdefs.h \
		     sbmlsolver/cvodeSolver.h \
		     sbmlsolver/sensSolver.h \
		     sbmlsolver/nullSolver.h \
		     sbmlsolver/integratorInstance.h \
		     sbmlsolver/integratorSettings.h \
		     sbmlsolver/modelSimplify.h \
		     sbmlsolver/odeConstruct.h \
		     sbmlsolver/odemodeldatatype.h \
		     sbmlsolver/odeModel.h \
		     sbmlsolver/odeSolver.h \
		     sbmlsolver/processAST.h \
		     sbmlsolver/sbml.h \
		     sbmlsolver/sbmlResults.h \
		     sbmlsolver/solverError.h \
		     sbmlsolver/util.h \
		     sbmlsolver/variableIndex.h
pkgconfig_DATA = libODES.pc

