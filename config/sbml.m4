dnl $Id: sbml.m4,v 1.16 2010/09/28 13:41:08 raimc Exp $


dnl
dnl look for SBML Library headers in some standard set of directories
dnl
AC_DEFUN([AC_SBML_PATH],
[ AC_MSG_CHECKING([for SBML Library headers])
  for ac_dir in             \
    /usr                    \
    /usr/local              \
    /usr/local/include      \
    /usr/include            \
    /usr/local/share        \
    /opt                    \
    ;                       \
  do
    if test -r "$ac_dir/include/sbml/SBMLTypes.h"; then
      ac_SBML_includes="${ac_dir}/include"
      with_libsbml="$ac_dir"
      SBML_CPPFLAGS="-I$ac_SBML_includes"
      AC_MSG_RESULT([yes])
      break
    fi
  done

  AC_MSG_CHECKING([for SBML Library])
  ac_dir=`echo "$ac_SBML_includes" | sed s/include/lib/`
  for ac_extension in a so sl dylib; do
    if test -r $ac_dir/libsbml.$ac_extension; then
      SBML_LDFLAGS="-L$ac_dir"
      if test "$HOST_TYPE" = darwin; then
        SBML_RPATH=
      else
        if test "$HOST_TYPE" = aix; then
          SBML_RPATH="-Wl,-R,$ac_dir"
        else
          SBML_RPATH="-Wl,-rpath,$ac_dir"
        fi
      fi
      SBML_LIBS="-lsbml"
      AC_MSG_RESULT([yes])
      break
    elif test -r ${ac_dir}64/libsbml.$ac_extension; then
      SBML_LDFLAGS="-L${ac_dir}64"
      if test "$HOST_TYPE" = darwin; then
        SBML_RPATH=
      else
        if test "$HOST_TYPE" = aix; then
          SBML_RPATH="-Wl,-R,${ac_dir}64"
        else
          SBML_RPATH="-Wl,-rpath,${ac_dir}64"
        fi
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
  [with_xerces=no])

  dnl specify prefix for libexpat
  AC_ARG_WITH([expat],
  AC_HELP_STRING([--with-expat=PREFIX],
                 [Use Expat XML Library]),
            [with_expat=$withval],
            [with_expat=no])

  dnl specify prefix for libxml2,
  dnl libxml2 is default, but path can be added
  AC_ARG_WITH([libxml2],
  AC_HELP_STRING([--with-libxml2=PREFIX],
                 [Use XML2 XML Library]),
            [with_libxml2=$withval],
            [with_libxml2=yes])



  dnl set SBML related variables
  SBML_CPPFLAGS=
  SBML_LDFLAGS=
  SBML_RPATH=
  SBML_LIBS=
  if test "$with_libsbml" = yes; then
    AC_SBML_PATH
  else
    SBML_CPPFLAGS="-I$with_libsbml/include"
    SBML_LDFLAGS="-L$with_libsbml/lib"

    dnl ac_SBML_includes=$with_libsbml
    if test "$HOST_TYPE" = darwin; then
      SBML_RPATH=
    else
      if test "$HOST_TYPE" = aix; then
        SBML_RPATH="-Wl,-R,$with_libsbml/lib"
      else
        SBML_RPATH="-Wl,-rpath,$with_libsbml/lib"
      fi
    fi

    SBML_LIBS="-lsbml"
  fi


  dnl set default with_libxml2 to no
  dnl if option --with-expat was given
  if test "$with_expat" != no; then
     with_libxml2=no
  fi
  if test "$with_xerces" != no; then
     with_libxml2=no
  fi

  dnl add xerces flags
  if test "$with_xerces" != no; then
     if test "$with_xerces" = yes; then
       SBML_LIBS="$SBML_LIBS -lxerces-c"
     else
       SBML_LDFLAGS="$SBML_LDFLAGS -L$with_xerces/lib"
       SBML_LIBS="$SBML_LIBS -lxerces-c"
     fi
  fi

  dnl add expat flags
  if test "$with_expat" != no; then
     if test "$with_expat" = yes; then
      SBML_LIBS="$SBML_LIBS -lexpat"
     else
      SBML_LIBS="$SBML_LIBS -lexpat"
      SBML_LDFLAGS="$SBML_LDFLAGS -L$with_expat/lib"
     fi
  fi

  dnl add libxml2 flags
  if test "$with_libxml2" != no; then
     if test "$with_libxml2" = yes; then
      SBML_LIBS="$SBML_LIBS -lxml2"
     else
      SBML_LIBS="$SBML_LIBS -lxml2"
      SBML_LDFLAGS="$SBML_LDFLAGS -L$with_libxml2/lib"
     fi
  fi

  dnl check if SBML Library is functional
  AC_MSG_CHECKING([for correct functioning of SBML])
  AC_LANG_PUSH([C++])
  dnl cach values of some global variables
  sbml_save_CPPFLAGS="$CPPFLAGS"
  sbml_save_LDFLAGS="$LDFLAGS"
  sbml_save_LIBS="$LIBS"
  dnl add SBML specific stuff to global variables
  CPPFLAGS="$SBML_CPPFLAGS $CPPFLAGS"
  LDFLAGS="$SBML_RPATH $SBML_LDFLAGS $LDFLAGS"
  LIBS="$SBML_LIBS $LIBS"
  dnl can we link a mini program with libsbml?
  AC_TRY_LINK([#include <sbml/SBMLTypes.h>],
    [SBMLReader_t *sr; sr = SBMLReader_create(); SBMLReader_free(sr);],
    [sbml_functional=yes],
    [sbml_functional=no])

  if test "$sbml_functional" = yes; then
    AC_MSG_RESULT([$sbml_functional])
  else
    AC_MSG_RESULT([$sbml_functional:
                   CPPFLAGS=$CPPFLAGS
                   LDFLAGS=$LDFLAGS
                   LIBS=$LIBS])
    AC_MSG_ERROR([Can not link to SBML Library:
		  Please, make sure to use libSBML version >= 3.4.1])
  fi

  dnl reset global variables to cached values
  CPPFLAGS=$sbml_save_CPPFLAGS
  LDFLAGS=$sbml_save_LDFLAGS
  LIBS=$sbml_save_LIBS
  AC_LANG_POP([C++])

  dnl add the CPPFLAGS and LDFLAGS for tcc online compilation
  AC_DEFINE_UNQUOTED([SBML_CPPFLAGS], "${with_libsbml}/include/sbml",
            [SBML include directories])
  AC_DEFINE_UNQUOTED([SBML_CPPFLAGS2], "${with_libsbml}/include/sbml",
            [SBML include directories])
  AC_DEFINE_UNQUOTED([SBML_LDFLAGS], "${with_libsbml}/lib",
            [SBML lib directories])
  AC_DEFINE_UNQUOTED([SBML_LIBS], "sbml",
            [SBML libs])

  AC_SUBST(SBML_CPPFLAGS)
  AC_SUBST(SBML_LDFLAGS)
  AC_SUBST(SBML_RPATH)
  AC_SUBST(SBML_LIBS)


])
