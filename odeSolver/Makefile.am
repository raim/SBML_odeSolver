# $Id: Makefile.am,v 1.8 2007/09/20 01:16:12 raimc Exp $
## 
## This application is free software; you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation; either version 2.1 of the License, or
## any later version.
## 
## This application is distributed in the hope that it will be useful, but
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
##     Rainer Machne    
## 
## Contributor(s):
##
##

AM_CPPFLAGS = -I$(srcdir)/../src @SBML_CPPFLAGS@ @GRACE_CPPFLAGS@ @SUNDIALS_CPPFLAGS@
AM_CFLAGS = -Wno-unknown-pragmas -Wall -Wextra -ansi -std=iso9899:1990
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
bin_PROGRAMS = odeSolver
odeSolver_SOURCES = main.c \
		    commandLine.c \
		    options.c  \
		    printModel.c \
		    interactive.c \
                    getopt.c getopt1.c
odeSolver_LDADD   = ../src/libODES.la \
                    @SUNDIALS_LIBS@ \
                    @SBML_LIBS@ \
                    @GRACE_LIBS@ \
                    @GRAPHVIZ_LIBS@
odeSolver_LDFLAGS = @SUNDIALS_LDFLAGS@ \
                    @SBML_LDFLAGS@ \
                    @GRACE_LDFLAGS@ \
                    @GRAPHVIZ_LDFLAGS@
