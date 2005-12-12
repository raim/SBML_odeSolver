# $Id: Makefile.am,v 1.15 2005/12/12 13:59:25 raimc Exp $
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
INCLUDES = -I. -I../src -I@prefix@/include @SBML_CFLAGS@ @CVODE_CFLAGS@
AM_CFLAGS  = @CFLAGS@ --pedantic
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
bin_PROGRAMS = defSeries integrate sensitivity batchIntegrate \
               simpleIntInst changeIntInst ASTIndexTest \
               ParameterScanner Sense \
               printODEs analyzeJacobian analyzeSens \
	       root
printODEs_SOURCES = printODEModel.c
printODEs_LDADD   = -L../src -L/@prefix@/lib -lODES \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    -lstdc++ -lxerces-c -lm
defSeries_SOURCES = definedTimeSeries.c
defSeries_LDADD   = -L../src -L/@prefix@/lib -lODES \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    -lstdc++ -lxerces-c -lm
integrate_SOURCES = integrate.c
integrate_LDADD   = -L../src -L/@prefix@/lib -lODES \
                    @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                    @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                    @SBML_LDFLAGS@ @SBML_LIBS@ \
                    @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                    -lstdc++ -lxerces-c -lm
sensitivity_SOURCES = sensitivity.c
sensitivity_LDADD   = -L../src -L/@prefix@/lib -lODES \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                      -lstdc++ -lxerces-c -lm
batchIntegrate_SOURCES = batch_integrate.c
batchIntegrate_LDADD   = -L../src -L/@prefix@/lib -lODES \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                         -lstdc++ -lxerces-c -lm
simpleIntInst_SOURCES  = simpleIntegratorInstance.c
simpleIntInst_LDADD    = -L../src -L/@prefix@/lib -lODES \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                         -lstdc++ -lxerces-c -lm
changeIntInst_SOURCES  = ChangingIntegratorInstance.c
changeIntInst_LDADD    = -L../src -L/@prefix@/lib -lODES \
                         @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                         @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                         @SBML_LDFLAGS@ @SBML_LIBS@ \
                         @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                         -lstdc++ -lxerces-c -lm
ASTIndexTest_SOURCES = ASTIndexTest.c
ASTIndexTest_LDADD   = -L../src -L/@prefix@/lib -lODES \
                       @SBML_LDFLAGS@ @SBML_LIBS@ \
                       -lstdc++ -lxerces-c -lm
analyzeJacobian_SOURCES = analyzeJacobian.c
analyzeJacobian_LDADD   = -L../src -L/@prefix@/lib -lODES \
                          @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                          @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                          @SBML_LDFLAGS@ @SBML_LIBS@ \
                          @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                          -lstdc++ -lxerces-c -lm
analyzeSens_SOURCES = analyzeSensitivity.c
analyzeSens_LDADD   = -L../src -L/@prefix@/lib -lODES \
                      @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                      @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                      @SBML_LDFLAGS@ @SBML_LIBS@ \
                      @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                      -lstdc++ -lxerces-c -lm
ParameterScanner_SOURCES = ParameterScanner.c
ParameterScanner_LDADD   = -L../src -L/@prefix@/lib -lODES \
                        @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                        @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                        @SBML_LDFLAGS@ @SBML_LIBS@ \
                        @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                        -lstdc++ -lxerces-c -lm
Sense_SOURCES = Sense.c
Sense_LDADD   = -L../src -L/@prefix@/lib -lODES \
                @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                @CVODE_LDFLAGS@ @CVODE_LIBS@ \
                @SBML_LDFLAGS@ @SBML_LIBS@ \
                @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
                -lstdc++ -lxerces-c -lm
root_SOURCES = findRoot.c
root_LDADD   = -L../src -L/@prefix@/lib -lODES \
               @GRACE_LDFLAGS@ @GRACE_LIBS@ \
               @CVODE_LDFLAGS@ @CVODE_LIBS@ \
               @SBML_LDFLAGS@ @SBML_LIBS@ \
               @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@ \
               -lstdc++ -lxerces-c -lm
