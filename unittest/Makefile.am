TESTS = unittest

noinst_PROGRAMS = unittest

unittest_CPPFLAGS = -DEXAMPLES=$(srcdir)/../examples \
                    -I$(srcdir)/../src \
                    @SUNDIALS_CPPFLAGS@ \
                    @SBML_CPPFLAGS@
unittest_CFLAGS = -Wno-unknown-pragmas -Wall -Wextra -ansi -std=iso9899:1990 \
                  @CHECK_CFLAGS@
unittest_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@ \
                   @SUNDIALS_LDFLAGS@ \
                   @SBML_LDFLAGS@ \
                   @GRACE_LDFLAGS@ \
                   @GRAPHVIZ_LDFLAGS@
unittest_LDADD = @CHECK_LIBS@ \
                 ../src/libODES.la \
                 @SUNDIALS_LIBS@ \
                 @SBML_LIBS@ \
                 @GRACE_LIBS@ \
                 @GRAPHVIZ_LIBS@
unittest_SOURCES = main.c \
                   test_ASTIndexNameNode.c \
                   test_charBuffer.c \
                   test_cvodeData.c \
                   test_cvodeSolver.c \
                   test_daeSolver.c \
                   test_integratorInstance.c \
                   test_integratorSettings.c \
                   test_interpol.c \
                   test_modelSimplify.c \
                   test_nullSolver.c \
                   test_odeConstruct.c \
                   test_odeModel.c \
                   test_odeSolver.c \
                   test_processAST.c \
                   test_sbml.c \
                   test_sbmlResults.c \
                   test_sensSolver.c \
                   test_solverError.c \
                   test_util.c
