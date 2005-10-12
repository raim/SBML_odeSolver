# $Id: Makefile.am,v 1.8 2005/10/12 12:52:45 raimc Exp $
INCLUDES = -I. -I../src -I@prefix@/include @SBML_CFLAGS@ @CVODE_CFLAGS@
AM_CFLAGS  = @CFLAGS@ --pedantic
AM_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
bin_PROGRAMS = defSeries integrate batchIntegrate simpleIntInst \
               changeIntInst ASTIndexTest printODEs
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
