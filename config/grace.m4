dnl $Id: grace.m4,v 1.4 2008/06/03 14:39:09 raimc Exp $

dnl
dnl look for GRACE Library headers in some standard set of directories
dnl
AC_DEFUN([AC_GRACE_PATH],
[ AC_MSG_CHECKING([for GRACE Library headers])
  for ac_dir in                    \
    /sw/include                    \
    /usr                           \
    /usr/local/grace/include       \
    /usr/grace/include	           \
    /usr/share/grace/include	   \
    /usr/local/share/grace/include \
    /opt/grace/include             \ 
    ;                              \
  do
    if test -r "$ac_dir/grace_np.h"; then
      ac_GRACE_includes="$ac_dir"
      GRACE_CFLAGS="-I$ac_GRACE_includes"
      AC_MSG_RESULT([yes])
      break
    fi
  done
  AC_MSG_CHECKING([for GRACE Library])
  for ac_dir in                \
    `echo "$ac_GRACE_includes" \
    | sed s/include/lib/ `     \
    /usr/local/lib             \
    ;                          \
  do
    for ac_extension in a so sl dylib; do
      if test -r $ac_dir/libgrace_np.$ac_extension; then
        GRACE_LDFLAGS="-L$ac_dir"
        if test $HOST_TYPE = darwin; then
          GRACE_RPATH=
        else
          GRACE_RPATH="-Wl,-rpath,$ac_dir"
        fi
        GRACE_LIBS="-lgrace_np"
        AC_MSG_RESULT([yes])
        break
      fi
    done
  done
])

dnl
dnl Check --with-grace[=PREFIX] is specified and grace is installed.
dnl
AC_DEFUN([CONFIG_LIB_GRACE],
[ AC_PREREQ(2.57)dnl
  AC_ARG_WITH([grace],
  AC_HELP_STRING([--with-grace=PREFIX],
                 [Use GRACE Library]),
              [with_grace=$withval],
              [with_grace=yes])
  dnl set GRACE related variables
  GRACE_CFLAGS=
  GRACE_LDFLAGS=
  GRACE_RPATH=
  GRACE_LIBS=
  if test $with_grace = yes; then
    AC_GRACE_PATH
  else
    GRACE_CFLAGS="-I$with_grace/include"
    GRACE_LDFLAGS="-L$with_grace/lib"
    if test $HOST_TYPE = darwin; then
      GRACE_RPATH=
    else
      GRACE_RPATH="-Wl,-rpath,$with_grace"
    fi    
    GRACE_LIBS="-lgrace_np"
  fi
  dnl check if GRACE Library is functional
  AC_MSG_CHECKING([correct functioning of GRACE])
  AC_LANG_PUSH(C)
  dnl cach values of some global variables
  grace_save_CFLAGS="$CFLAGS"
  grace_save_LDFLAGS="$LDFLAGS"
  grace_save_LIBS="$LIBS"
  dnl temporarily add GRACE specific stuff to global variables
  CFLAGS="$CFLAGS $GRACE_CFLAGS"
  LDFLAGS="$LDFLAGS $GRACE_LDFLAGS"
  LIBS="$GRACE_LIBS $LIBS" 
  dnl can we link a mini program with grace?
  AC_TRY_LINK([#include <grace_np.h>],
    [GraceOpen(2048); GracePrintf("world xmax %g", 10.0); GraceClose();],
    [grace_functional=yes],
    [grace_functional=no])

  if test $grace_functional = yes; then
    AC_MSG_RESULT([$grace_functional])
  else
    AC_MSG_RESULT([$grace_functional:
                   CFLAGS=$CFLAGS
                   LDFLAGS=$LDFLAGS
                   LIBS=$LIBS])
    AC_MSG_RESULT([Can not link to GRACE Library!])
    AC_MSG_RESULT([odeSolver will be installed without XMGrace functionality]) 
  fi
  dnl reset global variables to cached values
  CFLAGS=$grace_save_CFLAGS
  LDFLAGS=$grace_save_LDFLAGS
  LIBS=$grace_save_LIBS
  AC_LANG_POP
  if test $grace_functional = yes; then
    AC_DEFINE([USE_GRACE], 1, [Define to 1 to use the GRACE Library])
    AC_SUBST(USE_GRACE, 1)
    AC_SUBST(GRACE_CFLAGS)
    AC_SUBST(GRACE_LDFLAGS)
    AC_SUBST(GRACE_RPATH)
    AC_SUBST(GRACE_LIBS)
  else
    AC_DEFINE([USE_GRACE], 0, [Define to 1 to use the GRACE Library])
    AC_SUBST(USE_GRACE, 0)
    AC_SUBST(GRACE_CFLAGS, "")
    AC_SUBST(GRACE_LDFLAGS, "")
    AC_SUBST(GRACE_RPATH, "")
    AC_SUBST(GRACE_LIBS, "")    
  fi
])
