dnl $Id: sbml.m4,v 1.5 2005/12/12 16:51:35 raimc Exp $

dnl
dnl look for SBML Library headers in some standard set of directories
dnl
AC_DEFUN([AC_SBML_PATH],
[ AC_MSG_CHECKING([for SBML Library headers])
  for ac_dir in             \
    /usr/local/include      \
    /usr/include            \
    /usr/local/share        \
    /opt                    \ 
    ;                       \
  do
    if test -r "$ac_dir/sbml/SBMLTypes.h"; then
      ac_SBML_includes="$ac_dir"
      dnl include /sbml folder for libSBML 2.3.4 bugs
      SBML_CFLAGS="-I$ac_SBML_includes -I$ac_SBML_includes/sbml"
      AC_MSG_RESULT([yes])
      break
    fi
  done

  AC_MSG_CHECKING([for SBML Library])
  ac_dir=`echo "$ac_SBML_includes" | sed s/include/lib/`
  for ac_extension in a so sl dylib; do
    if test -r $ac_dir/libsbml.$ac_extension; then
      SBML_LDFLAGS="-L$ac_dir"
      if test $HOST_TYPE = darwin; then
        SBML_RPATH=
      else
        SBML_RPATH="-Wl,-rpath,$ac_dir"
      fi
      SBML_LIBS="-lsbml"
      AC_MSG_RESULT([yes])
      break
    fi
  done

  dnl get version of installed libsbml
  changequote(<<, >>)
  sbml_sharedlib=`echo $ac_dir/libsbml.?.?.?.so`
  if test -z "$sbml_sharedlib"; then sbml_sharedlib='libsbml.2.3.3'; fi
  sbml_version=`echo $sbml_sharedlib | sed 's/.*libsbml\.\([0-9]*\.[0-9]*\.[0-9]*\).so/\1/p; d'`
  sbml_major_version=`expr $sbml_version : '\([0-9]*\)\.[0-9]*\.[0-9]*'`
  sbml_minor_version=`expr $sbml_version : '[0-9]*\.\([0-9]*\)\.[0-9]*'`
  sbml_micro_version=`expr $sbml_version : '[0-9]*\.[0-9]*\.\([0-9]*\)' '|' 0`
  changequote([, ])
  if test $sbml_major_version -eq 2 &&
     test $sbml_minor_version -eq 3 &&
     test $sbml_micro_version -le 3;
  then
    AC_MSG_NOTICE([found SBML Library Version <= $sbml_version])
  else
    AC_MSG_NOTICE([found SBML Library Version $sbml_version])
  fi 
])

dnl
dnl Check --with-libsbml[=PREFIX] is specified and libsbml is installed.
dnl
AC_DEFUN([CONFIG_LIB_SBML],
[ AC_PREREQ(2.57)dnl
  AC_ARG_WITH([libsbml],
  AC_HELP_STRING([--with-libsbml=PREFIX],
                 [Use SBML Library]),
              [with_libsbml=$withval],
              [with_libsbml=yes])
  dnl set SBML related variables
  SBML_CFLAGS=
  SBML_LDFLAGS=
  SBML_RPATH=
  SBML_LIBS=
  if test $with_libsbml = yes; then
    AC_SBML_PATH
  else
    dnl get version of installed libsbml
    AC_MSG_CHECKING(for SBML Library Version)
    changequote(<<, >>)
    sbml_sharedlib=$with_libsbml/libsbml.?.?.?.so
    if test -z "$sbml_sharedlib"; then sbml_sharedlib='libsbml.2.3.3'; fi
    sbml_version=`echo $sbml_sharedlib | sed 's/.*libsbml\.\([0-9]*\.[0-9]*\.[0-9]*\).so/\1/p; d'`
    sbml_major_version=`expr $sbml_version : '\([0-9]*\)\.[0-9]*\.[0-9]*'`
    sbml_minor_version=`expr $sbml_version : '[0-9]*\.\([0-9]*\)\.[0-9]*'`
    sbml_micro_version=`expr $sbml_version : '[0-9]*\.[0-9]*\.\([0-9]*\)' '|' 0`
    changequote([, ])
    if test $sbml_major_version -eq 2 &&
       test $sbml_minor_version -eq 3 &&
       test $sbml_micro_version -le 3;
    then
      AC_MSG_NOTICE([found SBML Library Version <= $sbml_version])
    else
      AC_MSG_NOTICE([found SBML Library Version $sbml_version])
    fi     
    dnl include /sbml folder for libSBML 2.3.4 bugs
    SBML_CFLAGS="-I$with_libsbml/include -I$with_libsbml/include/sbml"
    SBML_LDFLAGS="-L$with_libsbml/lib"
    
    dnl ac_SBML_includes=$with_libsbml
    if test $HOST_TYPE = darwin; then
      SBML_RPATH=
    else
      SBML_RPATH="-Wl,-rpath,$with_libsbml/lib"
    fi   
    
    SBML_LIBS="-lsbml"
  fi

  dnl check if SBML Library is functional
  AC_MSG_CHECKING([correct functioning of SBML])
  AC_LANG_PUSH(C)
  dnl cach values of some global variables
  sbml_save_CFLAGS="$CFLAGS"
  sbml_save_LDFLAGS="$LDFLAGS"
  sbml_save_LIBS="$LIBS"
  dnl add SBML specific stuff to global variables
  CFLAGS="$CFLAGS $SBML_CFLAGS"
  LDFLAGS="$LDFLAGS $SBML_RPATH $SBML_LDFLAGS"
  LIBS="$LIBS -lxerces-c $SBML_LIBS"
  dnl can we link a mini program with libsbml?
  AC_TRY_LINK([#include <sbml/SBMLTypes.h>],
    [SBMLReader_t *sr; sr = SBMLReader_create(); SBMLReader_free(sr);],
    [sbml_functional=yes],
    [sbml_functional=no])

  if test $sbml_functional = yes; then
    AC_MSG_RESULT([$sbml_functional])
  else
    AC_MSG_RESULT([$sbml_functional:
                   CFLAGS=$CFLAGS
                   LDFLAGS=$LDFLAGS
                   LIBS=$LIBS])
    AC_MSG_ERROR([Can not link to SBML Library:
		  Please, make sure to use libSBML version >= 2.3.2])
  fi

  dnl check for version of libSBML, could be in separate function
  dnl AC_MSG_CHECKING([libSBML version])
  dnl LIBV=`grep PACKAGE_VERSION $ac_SBML_includes/sbml/config.h | \
  dnl       sed 's/\#define PACKAGE_VERSION \"//g' | sed 's/\"//g'`
  dnl AC_MSG_RESULT([version is $LIBV])
  dnl AX_COMPARE_VERSION([$LIBV],[lt],[2.2.0],
  dnl                   [AC_DEFINE([OLD_SBML], 1, \
  dnl                   [Define to 1 to use SBML Library version < 2.2.0])],
  dnl                   [AC_DEFINE([OLD_SBML], 0, \
  dnl                   [Define to 1 to use SBML Library version < 2.2.0])])
  dnl AC_SUBST(OLD_SBML)
  dnl AC_MSG_RESULT([$LIBV])

  dnl work around broken include-header-paths in libsbml-2.3.4
  if test $sbml_major_version -eq 2 &&
     test $sbml_minor_version -eq 3 &&
     test $sbml_micro_version -eq 4;
  then
     SBML_CFLAGS="$SBML_CFLAGS $SBML_CFLAGS/sbml"
  fi

  dnl reset global variables to cached values
  CFLAGS=$sbml_save_CFLAGS
  LDFLAGS=$sbml_save_LDFLAGS
  LIBS=$sbml_save_LIBS
  AC_LANG_POP
  AC_DEFINE([USE_SBML], 1, [Define to 1 to use the SBML Library])
  AC_SUBST(USE_SBML, 1)
  AC_DEFINE([OLD_LIBSBML], 0, [Define to 1 to use SBML Library version < 2.2.0])
  AC_SUBST(OLD_LIBSBML)
  AC_SUBST(SBML_CFLAGS)
  AC_SUBST(SBML_LDFLAGS)
  AC_SUBST(SBML_RPATH)
  AC_SUBST(SBML_LIBS)
])
