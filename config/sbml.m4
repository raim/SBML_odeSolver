dnl $Id: sbml.m4,v 1.12 2007/12/14 09:55:12 raimc Exp $


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
      with_libsbml="$ac_dir"
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

  dnl specify prefix for libxerces-c
  AC_ARG_WITH([xerces],
  AC_HELP_STRING([--with-xerces=PREFIX],
                 [Use Xerces XML Library]),
  [with_xerces=$withval],
  [with_xerces=yes])

  dnl specify prefix for libexpat
  AC_ARG_WITH([expat],
  AC_HELP_STRING([--with-expat=PREFIX],
                 [Use Expat XML Library]),
            [with_expat=$withval],
            [with_expat=no])



  dnl set SBML related variables
  SBML_CFLAGS=
  SBML_LDFLAGS=
  SBML_RPATH=
  SBML_LIBS=
  if test $with_libsbml = yes; then
    AC_SBML_PATH
  else
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

 
  dnl set with_xerces=no if option --with-expat was given		 
  if test $with_expat != no; then
     with_xerces=no
  fi
  
  dnl dispach xerces versus expat
  if test $with_xerces != no; then
     if test $with_xerces == yes; then
       SBML_LIBS="$SBML_LIBS -lxerces-c"
     else
       SBML_LDFLAGS="$SBML_LDFLAGS -L$with_xerces/lib"
       SBML_LIBS="$SBML_LIBS -lxerces-c"
     fi     
  else
     if test $with_expat == yes; then
      SBML_LIBS="$SBML_LIBS -lexpat"
     else
      SBML_LIBS="$SBML_LIBS -lexpat"
      SBML_LDFLAGS="$SBML_LDFLAGS -L$with_expat/lib"
     fi
  fi

  dnl check if SBML Library is functional
  AC_MSG_CHECKING([for correct functioning of SBML])
  AC_LANG_PUSH(C)
  dnl cach values of some global variables
  sbml_save_CFLAGS="$CFLAGS"
  sbml_save_LDFLAGS="$LDFLAGS"
  sbml_save_LIBS="$LIBS"
  dnl add SBML specific stuff to global variables
  CFLAGS="$CFLAGS $SBML_CFLAGS"
  LDFLAGS="$LDFLAGS $SBML_RPATH $SBML_LDFLAGS"
  LIBS="$LIBS $SBML_LIBS"
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
		  Please, make sure to use libSBML version >= 3.0.2 or CVS])
  fi

  dnl work around broken include-header-paths in libsbml-2.3.4
  dnl SBML_CFLAGS="$SBML_CFLAGS $SBML_CFLAGS/sbml"

  dnl reset global variables to cached values
  CFLAGS=$sbml_save_CFLAGS
  LDFLAGS=$sbml_save_LDFLAGS
  LIBS=$sbml_save_LIBS
  AC_LANG_POP

  dnl add the CFLAGS and LDFLAGS for tcc online compilation
  AC_DEFINE_UNQUOTED([SBML_CFLAGS], "${with_libsbml}/include",
            [SBML include directories])
  AC_DEFINE_UNQUOTED([SBML_CFLAGS2], "${with_libsbml}/include/sbml",
            [SBML include directories])
  AC_DEFINE_UNQUOTED([SBML_LDFLAGS], "${with_libsbml}/lib",
            [SBML lib directories])
  AC_DEFINE_UNQUOTED([SBML_LIBS], "sbml",
            [SBML libs])


  AC_DEFINE([USE_SBML], 1, [Define to 1 to use the SBML Library])
  AC_SUBST(USE_SBML, 1)
  AC_DEFINE([OLD_LIBSBML], 0, [Define to 1 for SBML Library version < 2.2.0])
  AC_SUBST(OLD_LIBSBML)
  AC_SUBST(SBML_CFLAGS)
  AC_SUBST(SBML_LDFLAGS)
  AC_SUBST(SBML_RPATH)
  AC_SUBST(SBML_LIBS)


])
