# $Id: Makefile.am,v 1.27 2007/09/20 01:16:12 raimc Exp $
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
INCLUDES = -I. -I../src -I@prefix@/include @SBML_CFLAGS@ @SUNDIALS_CFLAGS@ 
AM_CFLAGS  = @CFLAGS@
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
bin_PROGRAMS = defSeries integrate sensitivity adj_sensitivity batchIntegrate \
               simpleIntInst changeIntInst ASTIndexTest \
               ParameterScanner Sense \
               printODEs analyzeJacobian analyzeSens \
	       root senstest adjsenstest integrateODEs compilerTest \
	       adjsenstest_ContDiscData bistability
printODEs_SOURCES = printODEModel.c
printODEs_LDADD   = -L../src -L@prefix@/lib -lODES \
                    @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ 
defSeries_SOURCES = definedTimeSeries.c
defSeries_LDADD   = -L../src -L@prefix@/lib -lODES \
                    @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
integrate_SOURCES = integrate.c
integrate_LDADD   = -L../src -L@prefix@/lib -lODES \
                    @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
sensitivity_SOURCES = sensitivity.c
sensitivity_LDADD   = -L../src -L@prefix@/lib -lODES \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
senstest_SOURCES = senstest.c
senstest_LDADD   = -L../src -L@prefix@/lib -lODES \
                      @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
adjsenstest_SOURCES = adjsenstest.c
adjsenstest_LDADD   = -L../src -L@prefix@/lib -lODES \
                      @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
adjsenstest_ContDiscData_SOURCES = adjsenstest_ContDiscData.c
adjsenstest_ContDiscData_LDADD   = -L../src -L@prefix@/lib -lODES \
                      @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
adj_sensitivity_SOURCES = adj_sensitivity.c
adj_sensitivity_LDADD   = -L../src -L@prefix@/lib -lODES \
                      @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
batchIntegrate_SOURCES = batch_integrate.c
batchIntegrate_LDADD   = -L../src -L@prefix@/lib -lODES \
                         @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
simpleIntInst_SOURCES  = simpleIntegratorInstance.c
simpleIntInst_LDADD    = -L../src -L@prefix@/lib -lODES \
                         @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
changeIntInst_SOURCES  = ChangingIntegratorInstance.c
changeIntInst_LDADD    = -L../src -L@prefix@/lib -lODES \
                         @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
ASTIndexTest_SOURCES = ASTIndexTest.c
ASTIndexTest_LDADD   = -L../src -L@prefix@/lib -lODES \
                       @SBML_LDFLAGS@ @SBML_LIBS@
analyzeJacobian_SOURCES = analyzeJacobian.c
analyzeJacobian_LDADD   = -L../src -L@prefix@/lib -lODES \
                          @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                          @SBML_LDFLAGS@ @SBML_LIBS@ \
                          @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                          @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
analyzeSens_SOURCES = analyzeSensitivity.c
analyzeSens_LDADD   = -L../src -L@prefix@/lib -lODES \
                      @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
ParameterScanner_SOURCES = ParameterScanner.c
ParameterScanner_LDADD   = -L../src -L@prefix@/lib -lODES \
                        @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                        @SBML_LDFLAGS@ @SBML_LIBS@ \
                        @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                        @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
Sense_SOURCES = Sense.c
Sense_LDADD   = -L../src -L@prefix@/lib -lODES \
                @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                @SBML_LDFLAGS@ @SBML_LIBS@ \
                @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
root_SOURCES = findRoot.c
root_LDADD   = -L../src -L@prefix@/lib -lODES \
               @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
               @SBML_LDFLAGS@ @SBML_LIBS@ \
               @GRACE_LDFLAGS@ @GRACE_LIBS@ \
               @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
integrateODEs_SOURCES = integrateODEs.c
integrateODEs_LDADD   = -L../src -L@prefix@/lib -lODES \
                        @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                        @SBML_LDFLAGS@ @SBML_LIBS@ \
                        @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                        @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
compilerTest_SOURCES = testCompiler.c
compilerTest_LDADD   = -L../src -L@prefix@/lib -lODES \
                       @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                       @SBML_LDFLAGS@ @SBML_LIBS@ \
                       @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                       @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
bistability_SOURCES = bistability.c
bistability_LDADD   = -L../src -L@prefix@/lib -lODES \
                       @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                       @SBML_LDFLAGS@ @SBML_LIBS@ \
                       @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                       @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
