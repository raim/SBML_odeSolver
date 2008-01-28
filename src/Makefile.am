# $Id: Makefile.am,v 1.28 2008/01/28 19:25:26 stefan_tbi Exp $
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
INCLUDES = @SUNDIALS_CFLAGS@ \
           @SBML_CFLAGS@ \
           @GRAPHVIZ_CFLAGS@ \
           @GRACE_CFLAGS@ \
           -I.
LDADD = @SBML_LDFLAGS@ @SBML_LIBS@ \
        @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
        @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
        @GRACE_LDFLAGS@ @GRACE_LIBS@
AM_LDFLAGS = @GRAPHVIZ_RPATH@ \
             @SBML_RPATH@
AM_CFLAGS = @FPIC@
AM_CXXFLAGS = @FPIC@
lib_LIBRARIES = libODES.a

libODES_a_SOURCES = ASTIndexNameNode.cpp \
                    arithmeticCompiler.c \
                    charBuffer.cpp \
                    compiler.c \
                    cvodeData.c \
                    cvodeSolver.c \
                    daeSolver.c \
                    drawGraph.c \
                    integratorInstance.c \
                    integratorSettings.c \
                    interpol.c \
                    modelSimplify.c \
                    nullSolver.c \
                    odeConstruct.c \
                    odeModel.c \
                    odeSolver.c \
                    processAST.c \
                    sbml.c \
                    sbmlResults.c \
                    sensSolver.c \
                    solverError.c \
                    util.c
pkginclude_HEADERS = sbmlsolver/ASTIndexNameNode.h \
                     sbmlsolver/arithmeticCompiler.h \
                     sbmlsolver/charBuffer.h \
                     sbmlsolver/compiler.h \
                     sbmlsolver/config.h \
                     sbmlsolver/cvodeData.h \
                     sbmlsolver/cvodeSolver.h \
                     sbmlsolver/daeSolver.h \
                     sbmlsolver/drawGraph.h \
                     sbmlsolver/exportdefs.h \
                     sbmlsolver/integratorInstance.h \
                     sbmlsolver/integratorSettings.h \
                     sbmlsolver/interpol.h \
                     sbmlsolver/modelSimplify.h \
                     sbmlsolver/nullSolver.h \
                     sbmlsolver/odeConstruct.h \
                     sbmlsolver/odeModel.h \
                     sbmlsolver/odeSolver.h \
                     sbmlsolver/processAST.h \
                     sbmlsolver/sbml.h \
                     sbmlsolver/sbmlResults.h \
                     sbmlsolver/sensSolver.h \
                     sbmlsolver/solverError.h \
                     sbmlsolver/util.h \
                     sbmlsolver/variableIndex.h
pkgconfig_DATA = libODES.pc

