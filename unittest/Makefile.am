TESTS = unittest

noinst_PROGRAMS = unittest

unittest_CPPFLAGS = -DEXAMPLES=$(srcdir)/../examples \
                    -I$(srcdir)/../src \
                    @SBML_CPPFLAGS@
unittest_CFLAGS = -Wno-unknown-pragmas -Wall -ansi -std=iso9899:1990
unittest_LDFLAGS = @GRAPHVIZ_RPATH@ @GRACE_RPATH@ @SBML_RPATH@
unittest_LDADD = -lcheck \
                 ../src/libODES.la \
                 @SUNDIALS_LDFLAGS@ @SUNDIALS_LIBS@ \
                 @SBML_LDFLAGS@ @SBML_LIBS@ \
                 @GRACE_LDFLAGS@ @GRACE_LIBS@ \
                 @GRAPHVIZ_LDFLAGS@ @GRAPHVIZ_LIBS@
unittest_SOURCES = main.c \
                   test_ASTIndexNameNode.c \
                   test_charBuffer.c \
                   test_sbml.c \
                   test_solverError.c \
                   test_util.c
