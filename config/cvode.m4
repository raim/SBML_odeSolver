dnl $Id: cvode.m4,v 1.1 2005/05/30 19:49:14 raimc Exp $

dnl
dnl look for CVODE Library headers in some standard set of directories
dnl
AC_DEFUN([AC_CVODE_PATH],
[ AC_MSG_CHECKING([for CVODE Library headers])
  for ac_dir in              \
    /usr/local/include       \
    /usr/local/include/cvode \
    /usr/local/include/CVODE \
    /usr/include/cvode       \
    /usr/include/CVODE       \
    /usr/local/share/cvode   \
    /usr/local/share/CVODE   \
    /opt/cvode               \
    /opt/CVODE               \
    ;                        \
  do
    if test -r "$ac_dir/cvode.h"; then
      ac_CVODE_includes="$ac_dir"
      CVODE_CFLAGS="-I$ac_dir"
      AC_MSG_RESULT([yes])
      break
    fi
  done

  AC_MSG_CHECKING([for CVODE Library])
  ac_dir=`echo "$ac_CVODE_includes" | sed s/include/lib/`
  for ac_extension in a so sl dylib; do
    if test -r $ac_dir/libcvode.$ac_extension; then
      CVODE_LDFLAGS="-L$ac_dir"
      CVODE_LIBS="-lcvode"
      AC_MSG_RESULT([yes])
      break
    fi
  done
])

dnl
dnl Check --with-libcvode[=PREFIX] is specified and libcvode is installed.
dnl
AC_DEFUN([CONFIG_LIB_CVODE],
[ AC_PREREQ(2.57)dnl
  AC_ARG_WITH([libcvode],
  AC_HELP_STRING([--with-libcvode=PREFIX],
                 [Use CVODE Library]),
              [with_libcvode=$withval],
              [with_libcvode=yes])
  dnl set CVODE related variables
  CVODE_CFLAGS=
  CVODE_LDFLAGS=
  CVODE_LIBS=
  if test "$with_libcvode" = yes; then
    AC_CVODE_PATH
  else
    CVODE_CFLAGS="-I$with_libcvode/include"
    CVODE_LDFLAGS="-L$with_libcvode/lib"
    CVODE_LIBS="-lcvode"
  fi
  dnl check if CVODE Library is functional
  AC_MSG_CHECKING([correct functioning of CVODE])
  AC_LANG_PUSH(C)
  dnl cach values of some global variables
  cvode_save_CFLAGS="$CFLAGS"
  cvode_save_LDFLAGS="$LDFLAGS"
  cvode_save_LIBS="$LIBS"
  dnl add CVODE specific stuff to global variables
  CFLAGS="$CFLAGS $CVODE_CFLAGS"
  LDFLAGS="$LDFLAGS $CVODE_LDFLAGS"
  LIBS="$LIBS -lm $CVODE_LIBS"
  dnl can we link a mini program with cvode?
  AC_TRY_LINK([#include <cvode.h>  \
               #include <cvdense.h>\
               #include <dense.h>],
    [N_Vector y; y = N_VNew(3, NULL); N_VFree(y);],
    [cvode_functional=yes],
    [cvode_functional=no])

  if test "$cvode_functional" = yes; then
    AC_MSG_RESULT([$cvode_functional])
  else
    AC_MSG_RESULT([$cvode_functional:
                   CFLAGS=$CFLAGS
                   LDFLAGS=$LDFLAGS
                   LIBS=$LIBS])
    AC_MSG_ERROR([Can not link to CVODE Library])
  fi
  dnl reset global variables to cached values
  CFLAGS=$cvode_save_CFLAGS
  LDFLAGS=$cvode_save_LDFLAGS
  LIBS=$cvode_save_LIBS
  AC_LANG_POP(C)

  AC_DEFINE([USE_CVODE], 1, [Define to 1 to use the CVODE Library])
  AC_SUBST(USE_CVODE, 1)
  AC_SUBST(CVODE_CFLAGS)
  AC_SUBST(CVODE_LDFLAGS)
  AC_SUBST(CVODE_LIBS)
])
