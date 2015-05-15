# $Id: Makefile.am,v 1.30 2009/02/15 17:51:28 chfl Exp $
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
AM_CPPFLAGS = -I$(srcdir)/../src @SBML_CPPFLAGS@ @SUNDIALS_CPPFLAGS@
AM_CFLAGS  = -Wno-unknown-pragmas -Wall -Wextra -ansi -std=iso9899:1990
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@ \
             @SUNDIALS_LDFLAGS@ \
             @SBML_LDFLAGS@ \
             @GRACE_LDFLAGS@ \
             @GRAPHVIZ_LDFLAGS@
LDADD = ../src/libODES.la \
    @SUNDIALS_LIBS@ \
    @SBML_LIBS@ \
    @GRACE_LIBS@ \
    @GRAPHVIZ_LIBS@

noinst_PROGRAMS = defSeries integrate sensitivity adj_sensitivity batchIntegrate \
               simpleIntInst changeIntInst changeParamInt ASTIndexTest ASTgetIndexTest\
               ParameterScanner Sense \
               sharingIntInst twinIntInst \
               printODEs analyzeJacobian analyzeSens \
	       root senstest FIMtest adjsenstest integrateODEs compilerTest \
	       adjsenstest_ContDiscData bistability
printODEs_SOURCES = printODEModel.c
defSeries_SOURCES = definedTimeSeries.c
integrate_SOURCES = integrate.c
sensitivity_SOURCES = sensitivity.c
senstest_SOURCES = senstest.c
FIMtest_SOURCES = FIMtest.c
adjsenstest_SOURCES = adjsenstest.c
adjsenstest_ContDiscData_SOURCES = adjsenstest_ContDiscData.c
adj_sensitivity_SOURCES = adj_sensitivity.c
batchIntegrate_SOURCES = batch_integrate.c
simpleIntInst_SOURCES  = simpleIntegratorInstance.c
changeIntInst_SOURCES  = ChangingIntegratorInstance.c
changeParamInt_SOURCES = ChangingParameterIntegrator.c
ASTIndexTest_SOURCES = ASTIndexTest.c
ASTgetIndexTest_SOURCES = ASTgetIndexTest.c
analyzeJacobian_SOURCES = analyzeJacobian.c
analyzeSens_SOURCES = analyzeSensitivity.c
ParameterScanner_SOURCES = ParameterScanner.c
Sense_SOURCES = Sense.c
sharingIntInst_SOURCES = SharingIntegratorInstance.c
twinIntInst_SOURCES = TwinIntegratorInstance.c
root_SOURCES = findRoot.c
integrateODEs_SOURCES = integrateODEs.c
compilerTest_SOURCES = testCompiler.c
bistability_SOURCES = bistability.c
