# $Id: Makefile.am,v 1.14 2005/11/02 17:32:13 raimc Exp $
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
if MAKE_BINDINGS
   SUBDIRS = src odeSolver bindings
else
   SUBDIRS = src odeSolver
endif
CFLAGS = $(AM_CFLAGS) -Wno-unknown-pragmas
EXTRA_DIST = odeSolver/*.c \
             odeSolver/*.h \
             odeSolver/Makefile.am \
             Perlen/AddLegend.pl \
             sbml/*.xsd \
             Test/MAPK.xml Test/simple.sh \
             doc/os_v1-5.pdf \
             doc/Makefile.am \
             doc/Makefile.in \
             doc/SBML_odeSolver.1.in \
	     examples/integrate.c \
             examples/batch_integrate.c \
	     examples/sensitivity.c \
             examples/definedTimeSeries.c \
             examples/printODEModel.c \
             examples/analyzeJabocian.c \
             examples/ASTIndexTest.c \
             examples/ChangingIntegratorInstance.c \
	     examples/simpleIntegratorInstance.c \
	     examples/basic-model1-forward-l2.xml \
             examples/Makefile.am \
             examples/Makefile.batch \
	     examples/Makefile.in \
             examples/Makefile.integrate \
             examples/README

