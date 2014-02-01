#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.53])
AC_INIT([msicon-extractor], [0.01], [https://github.com/rgcjonas/msicon-extractor])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIR([build])
AM_CONFIG_HEADER(config.h)
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.38])
PKG_CHECK_MODULES([libpng], [libpng12 >= 1.2])

# Checks for header files.


# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# initialize tests
GLIB_TESTS

# i18n
#FIXME: add intltool support and create actual l10n
AC_SUBST(GETTEXT_PACKAGE, $PACKAGE_NAME)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[The name of the gettext domain])

AC_CONFIG_FILES([
Makefile
src/Makefile
test/Makefile
])
AC_OUTPUT
