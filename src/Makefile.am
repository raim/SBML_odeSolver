# $Id: Makefile.am,v 1.29 2009/02/13 17:45:32 chfl Exp $
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
AM_CPPFLAGS = @SUNDIALS_CPPFLAGS@ \
           @SBML_CPPFLAGS@ \
           @GRAPHVIZ_CPPFLAGS@ \
           @GRACE_CPPFLAGS@
AM_LDFLAGS = @GRAPHVIZ_RPATH@ \
             @SBML_RPATH@
AM_CFLAGS = -Wno-unknown-pragmas -Wall -Wextra -ansi -std=iso9899:1990
lib_LTLIBRARIES = libODES.la


libODES_la_LIBADD = @SBML_LIBS@ \
                    @SUNDIALS_LIBS@ \
                    @GRAPHVIZ_LIBS@ \
                    @GRACE_LIBS@
libODES_la_LDFLAGS = -no-undefined \
                     @SBML_LDFLAGS@ \
                     @SUNDIALS_LDFLAGS@ \
                     @GRAPHVIZ_LDFLAGS@ \
                     @GRACE_LDFLAGS@
libODES_la_SOURCES = ASTIndexNameNode.c \
                    arithmeticCompiler.c \
                    charBuffer.c \
                    compiler.c \
                    cvodeData.c \
                    cvodeSolver.c \
                    daeSolver.c \
                    drawGraph.c \
                    evaluateAST.c \
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
                    util.c \
                    private/data.c \
                    private/error.c
pkginclude_HEADERS = sbmlsolver/ASTIndexNameNode.h \
                     sbmlsolver/arithmeticCompiler.h \
                     sbmlsolver/charBuffer.h \
                     sbmlsolver/compiler.h \
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

