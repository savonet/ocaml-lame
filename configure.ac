# $Id$

# check for one particular file of the sources
AC_INIT([ocaml-lame],[0.4.0],[savonet-users@lists.sourceforge.net])

VERSION=$PACKAGE_VERSION
AC_MSG_RESULT(configuring $PACKAGE_STRING)

AC_BASE_CHECKS()

AC_MSG_CHECKING([whether to build shared modules])
if test -n "${OCAML_HAS_FIRST_CLASS_MODULES}"; then
  AC_MSG_RESULT([yes])
else
  OCAML_DYNLINK=""
  AC_MSG_RESULT([first-class modules not supported by the compiler, disabling shared modules])
fi

# Check for liblame
LAME_LIBS="-lmp3lame -lm"
LAME_CFLAGS=""
LAME_CPPFLAGS=""
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_CFLAGS="$CFLAGS"
ac_save_LIBS="$LIBS"
CPPFLAGS="$CPPFLAGS $LAME_CPPFLAGS"
CFLAGS="$CFLAGS $LAME_CFLAGS"
LIBS="$LAME_LIBS $LIBS"
AC_CHECK_HEADERS([lame/lame.h], [
                  AC_CHECK_FUNC([lame_init], [ac_have_lame="yes"])
])
CPPFLAGS="$ac_save_CPPFLAGS"
CFLAGS="$ac_save_CFLAGS"
LIBS="$ac_save_LIBS"
if test "$ac_have_lame" != "yes"; then
  AC_MSG_ERROR([Cannot find liblame.])
fi
CPPFLAGS="$CPPFLAGS $LAME_CPPFLAGS"
CFLAGS="$CFLAGS $LAME_CFLAGS"
LIBS="$LIBS $LAME_LIBS"


# substitutions to perform
AC_SUBST(VERSION)

# Finally create the Makefile and samples
AC_CONFIG_FILES([Makefile],[chmod a-w Makefile])
AC_CONFIG_FILES([src/Makefile],[chmod a-w src/Makefile])
AC_CONFIG_FILES([src/META])
AC_OUTPUT
