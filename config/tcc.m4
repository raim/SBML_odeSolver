dnl $Id: tcc.m4,v 1.6 2007/08/20 18:02:12 chfl Exp $


dnl
dnl look for TCC library headers
dnl
AC_DEFUN([AC_TCC_PATH],
[ AC_MSG_CHECKING([for TCC Library headers])
  for ac_dir in             \
    /usr/local/include      \
    /usr/include            \
    /usr/local/share        \
    /opt                    \ 
    ;                       \
  do
    if test -r "$ac_dir/libtcc.h"; then
      ac_TCC_includes="$ac_dir"
      with_libtcc="$ac_dir"
      TCC_CFLAGS="-I$ac_TCC_includes"
      AC_MSG_RESULT([yes])
      break
    fi
  done

  AC_MSG_CHECKING([for TCC Library])
  ac_dir=`echo "$ac_TCC_includes" | sed s/include/lib/`
  for ac_extension in a so sl dylib; do
    if test -r $ac_dir/libtcc.$ac_extension; then
      TCC_LDFLAGS="-L$ac_dir"
      if test $HOST_TYPE = darwin; then
        TCC_RPATH=
      else
        TCC_RPATH="-Wl,-rpath,$ac_dir"
      fi

dnl !!! -m32 is required for tcc but in conflict with all others
      TCC_LIBS="-ldl -ltcc"
dnl      TCC_LIBS=""
      AC_MSG_RESULT([yes])
      break
    fi
  done
])

dnl
dnl Check --with-libtcc[=PREFIX] is specified and libtcc is installed.
dnl
AC_DEFUN([CONFIG_LIB_TCC],
[ AC_PREREQ(2.57)dnl
  AC_ARG_WITH([libtcc],
  AC_HELP_STRING([--with-libtcc=PREFIX],
                 [Use TCC Library]),
              [with_libtcc=$withval],
              [with_libtcc=no])

  dnl set TCC related variables
  TCC_CFLAGS=
  TCC_LDFLAGS=
  TCC_RPATH=
  TCC_LIBS=
  if test $with_libtcc = yes; then
    AC_TCC_PATH
  else
    TCC_CFLAGS="-I$with_libtcc/include"
    TCC_LDFLAGS="-L$with_libtcc/lib"
    if test $HOST_TYPE = darwin; then
      TCC_RPATH=
    else
      TCC_RPATH="-Wl,-rpath,$with_libtcc/lib"
    fi   
    TCC_LIBS="-ldl -ltcc"  
  fi

dnl !!! -m32 is required for tcc on x86_64 but in conflict with all others

  dnl check if TCC Library is functional
  AC_MSG_CHECKING([for correct functioning of TCC])
  AC_LANG_PUSH(C)
  dnl cach values of some global variables
  tcc_save_CFLAGS="$CFLAGS"
  tcc_save_LDFLAGS="$LDFLAGS"
  tcc_save_LIBS="$LIBS"
  dnl add TCC specific stuff to global variables
  CFLAGS="$CFLAGS $TCC_CFLAGS"
  LDFLAGS="$LDFLAGS $TCC_RPATH $TCC_LDFLAGS"
  LIBS=" $TCC_LIBS $LIBS"
  dnl set headers and test program
  tcc_headers="#include <libtcc.h>"
  tcc_testprg="TCCState *s;s = tcc_new();"
  dnl can we link a mini program with libtcc?
  AC_TRY_LINK([$tcc_headers],
     [$tcc_testprg],
     [tcc_functional=yes],
     [tcc_functional=no])

  if test $tcc_functional = yes; then
    AC_MSG_RESULT([$tcc_functional])
  else
    AC_MSG_RESULT([$tcc_functional:
                   CFLAGS=$CFLAGS
                   LDFLAGS=$LDFLAGS
                   LIBS=$LIBS])
    AC_MSG_RESULT([Can not link to TCC Library: online compilation disabled!])		  
  fi
  dnl reset global variables to cached values
  CFLAGS=$tcc_save_CFLAGS
  LDFLAGS=$tcc_save_LDFLAGS
  LIBS=$tcc_save_LIBS
  AC_LANG_POP
  if test $tcc_functional = yes; then
    dnl add the CFLAGS and LDFLAGS for tcc online compilation
    AC_DEFINE_UNQUOTED([TCC_CFLAGS], "${with_libtcc}/include",
              [TCC include directories])
    AC_DEFINE_UNQUOTED([TCC_LDFLAGS], "${with_libtcc}/lib",
              [TCC lib directories])
    AC_DEFINE_UNQUOTED([TCC_LIBS], "tcc",
              [TCC libs])
    AC_DEFINE([USE_TCC], 1, [Define to 1 to use the TCC Library])
    AC_SUBST(USE_TCC, 1)
    AC_SUBST(TCC_CFLAGS)
    AC_SUBST(TCC_LDFLAGS)
    AC_SUBST(TCC_RPATH)
    AC_SUBST(TCC_LIBS)
  else
    AC_DEFINE([USE_TCC], 0, [Define to 1 to use the TCC Library])
    AC_SUBST(USE_TCC, 0)
    AC_SUBST(TCC_CFLAGS, "")
    AC_SUBST(TCC_LDFLAGS, "")
    AC_SUBST(TCC_RPATH, "")
    AC_SUBST(TCC_LIBS, "")    
  fi

])
