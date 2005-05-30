dnl $Id: graphviz.m4,v 1.1 2005/05/30 19:49:14 raimc Exp $

dnl
dnl look for GRAPHVIZ Library headers in some standard set of directories
dnl
AC_DEFUN([AC_GRAPHVIZ_PATH],
[ AC_MSG_CHECKING([for GRAPHVIZ Library headers])
  for ac_dir in                \
    /usr/local/include         \
    /usr/local/include/graphviz \
    /usr/include/graphviz       \
    /usr/local/share/graphviz   \
    /opt/graphviz               \ 
    ;                       \
  do
    if test -r "$ac_dir/gvrender.h"; then
      ac_GRAPHVIZ_includes="$ac_dir"
      GRAPHVIZ_CFLAGS="-I$ac_GRAPHVIZ_includes"
      AC_MSG_RESULT([yes])
      break
    fi
  done

  AC_MSG_CHECKING([for GRAPHVIZ Library])
  for ac_dir in               \
    `echo "$ac_GRAPHVIZ_includes" \
    | sed s/include/lib/`     \
    /usr/local/lib            \
    ;                         \
  do
    for ac_extension in a so sl dylib; do
      if test -r $ac_dir/libdotneato.$ac_extension; then
        GRAPHVIZ_LDFLAGS="-L$ac_dir"
        if test $HOST_TYPE = darwin; then
          GRAPHVIZ_RPATH=
        else
          GRAPHVIZ_RPATH="-Wl,-rpath,$ac_dir"
        fi
        GRAPHVIZ_LIBS="-ldotneato"
        AC_MSG_RESULT([yes])
        break
      fi
    done
  done
])

dnl
dnl Check --with-graphviz[=PREFIX] is specified and graphviz is installed.
dnl
AC_DEFUN([CONFIG_LIB_GRAPHVIZ],
[ AC_PREREQ(2.57)dnl
  AC_ARG_WITH([graphviz],
  AC_HELP_STRING([--with-graphviz=PREFIX],
                 [Use GRAPHVIZ Library]),
              [with_graphviz=$withval],
              [with_graphviz=yes])
  dnl set GRAPHVIZ related variables
  GRAPHVIZ_CFLAGS=
  GRAPHVIZ_LDFLAGS=
  GRAPHVIZ_RPATH=
  GRAPHVIZ_LIBS=
  if test $with_graphviz = yes; then
    AC_GRAPHVIZ_PATH
  else
    GRAPHVIZ_CFLAGS="-I$with_graphviz"
    GRAPHVIZ_LDFLAGS="-L$with_graphviz"
    if test $HOST_TYPE = darwin; then
     GRAPHVIZ_RPATH=
    else
     GRAPHVIZ_RPATH="-Wl,-rpath,$with_graphviz"
    fi
    GRAPHVIZ_LIBS="-ldotneato"
  fi
  dnl check if GRAPHVIZ Library is functional
  AC_MSG_CHECKING([correct functioning of GRAPHVIZ])
  AC_LANG_PUSH(C)
  dnl cach values of some global variables
  graphviz_save_CFLAGS="$CFLAGS"
  graphviz_save_LDFLAGS="$LDFLAGS"
  graphviz_save_LIBS="$LIBS"
  dnl add GRAPHVIZ specific stuff to global variables
  CFLAGS="$CFLAGS -Wno-unknown-pragmas $GRAPHVIZ_CFLAGS"
  LDFLAGS="$LDFLAGS $GRAPHVIZ_LDFLAGS"
  LIBS="$LIBS $GRAPHVIZ_LIBS"
  dnl can we link a mini program with graphviz?
  AC_TRY_LINK([#include <dotneato.h> #include <gvrender.h>],
    [GVC_t *gvc; gvc = gvNEWcontext(NULL, NULL); gvFREEcontext(gvc);],
    [graphviz_functional=yes],
    [graphviz_functional=no])

  if test $graphviz_functional = yes; then
    AC_MSG_RESULT([$graphviz_functional])
  else
    AC_MSG_RESULT([$graphviz_functional:
                   CFLAGS=$CFLAGS
                   LDFLAGS=$LDFLAGS
                   LIBS=$LIBS])
    AC_MSG_RESULT([Can not link to GRAPHVIZ Library])
    AC_MSG_RESULT([odeSolver will be installed without Graphviz functionality])
  fi
  dnl reset global variables to cached values
  CFLAGS=$graphviz_save_CFLAGS
  LDFLAGS=$graphviz_save_LDFLAGS
  LIBS=$graphviz_save_LIBS
  AC_LANG_POP
  if test $graphviz_functional = yes; then
    AC_DEFINE([USE_GRAPHVIZ], 1, [Define to 1 to use the GRAPHVIZ Library])
    AC_SUBST(USE_GRAPHVIZ, 1)
    AC_SUBST(GRAPHVIZ_CFLAGS)
    AC_SUBST(GRAPHVIZ_LDFLAGS)
    AC_SUBST(GRAPHVIZ_RPATH)
    AC_SUBST(GRAPHVIZ_LIBS)
  else
    AC_DEFINE([USE_GRAPHVIZ], 0, [Define to 1 to use the GRAPHVIZ Library])
    AC_SUBST(USE_GRAPHVIZ, 0)
    AC_SUBST(GRAPHVIZ_CFLAGS, "")
    AC_SUBST(GRAPHVIZ_LDFLAGS, "")
    AC_SUBST(GRAPHVIZ_RPATH, "")
    AC_SUBST(GRAPHVIZ_LIBS, "")    
  fi

])
