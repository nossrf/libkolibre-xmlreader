## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-xmlreader.
#
# Kolibre-xmlreader is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-xmlreader is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-xmlreader. If not, see <http://www.gnu.org/licenses/>.


# Process this file with autoconf to produce a configure script.

dnl -----------------------------------------------
dnl  Release instructions
dnl -----------------------------------------------
dnl
dnl  When you are about to release the software
dnl  either as a stable release or an unstable release
dnl  you should update the *_VERSION variables below
dnl  according to the following rules:
dnl
dnl  1. If any interfaces has been removed or changed,
dnl     increment MAJOR_VERSION and set others to 0
dnl  2. If any interface has been added,
dnl     increment MINOR_VERSION and set PATCH_VERSION to 0
dnl  3. If only the source code has changed,
dnl     increment PATCH_VERSION
dnl
dnl  If you are releasing a stable version,
dnl  set package version number in AC_INIT to
dnl  [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION]
dnl
dnl  If you are releasing an unstable version,
dnl  add a string (i.e -alpha or -beta or -rc1) to the
dnl  package version in AC_INIT after $PATCH_VERSION
dnl  e.g. [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION-rc1]

m4_define([MAJOR_VERSION], [0])
m4_define([MINOR_VERSION], [0])
m4_define([PATCH_VERSION], [1])
m4_define([EXTRA_VERSION], [])

AC_PREREQ([2.67])
AC_INIT([kolibre-xmlreader],[MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION[]EXTRA_VERSION],[BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([tar-pax color-tests parallel-tests])

VERSION_INFO=m4_eval(MAJOR_VERSION + MINOR_VERSION):PATCH_VERSION:MINOR_VERSION
AC_SUBST(VERSION_INFO)

# Checks for programs.
LT_INIT
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CXX
ACX_PTHREAD

dnl -----------------------------------------------
dnl Doxygen settings
dnl -----------------------------------------------

DX_DOXYGEN_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_PDF_FEATURE(ON)
DX_INIT_DOXYGEN([kolibre-xmlreader], doxygen.cfg, [doxygen-doc])

# Checks for libraries.

dnl -----------------------------------------------
dnl check for log4cxx
dnl -----------------------------------------------

PKG_CHECK_MODULES(LOG4CXX, liblog4cxx >= 0.10.0, HAVE_LOG4CXX=yes, HAVE_LOG4CXX=no)

if (test "x$HAVE_LOG4CXX" = "xno") then
    AC_MSG_ERROR([**liblog4cxx >= 0.10.0 is needed to build libkolibre-narrator **])
fi

AC_SUBST(LOG4CXX_CFLAGS)
AC_SUBST(LOG4CXX_LIBS)

dnl -----------------------------------------------
dnl check for Libcurl
dnl -----------------------------------------------
PKG_CHECK_MODULES(LIBCURL, libcurl >= 7.17.1, HAVE_LIBCURL=yes, HAVE_LIBCURL=no)

AC_SUBST(LIBCURL_CFLAGS)
AC_SUBST(LIBCURL_LIBS)

if (test "x$HAVE_LIBCURL" = "xno") then
  AC_MSG_ERROR([**libcurl >= 7.17.1 is needed to build libkolibre-xmlreader **])
fi

dnl -----------------------------------------------
dnl Check for libxml2
dnl -----------------------------------------------
PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= 2.6.23, HAVE_LIBXML2=yes, HAVE_LIBXML2=no)
if test "x$HAVE_LIBXML2" = "xno"
then
  AC_MSG_ERROR([**libxml2 >= 2.6.23 is needed to build libkolibre-xmlreader **])
fi

AC_SUBST(LIBXML2_LIBS)
AC_SUBST(LIBXML2_CFLAGS)

dnl -----------------------------------------------
dnl check for libtidy
dnl -----------------------------------------------

# libtidy does not come with a .pc file, thus we need custom checks
# to determine if and where tidy is installed on the system

AC_ARG_WITH([tidy], [AS_HELP_STRING([--without-tidy], [disable support for tidy])],
[], [with_tidy=yes])

AS_IF([test "x$with_tidy" = "xyes"],
[HAVE_LIBTIDY=no
AC_MSG_CHECKING([for libtidy])

if test -f ${prefix}/include/tidy.h; then
    HAVE_LIBTIDY=yes
    LIBTIDY_CFLAGS="-I${prefix}/include"
    LIBTIDY_LIBS="-L${prefix}/lib -ltidy"
else
    if test -f /usr/include/tidy/tidy.h; then
        HAVE_LIBTIDY=yes
        LIBTIDY_CFLAGS="-I/usr/include/tidy"
        LIBTIDY_LIBS="-L/usr/lib -ltidy"
    fi
fi

if test "x$HAVE_LIBTIDY" = "xno"
then
    AC_MSG_RESULT([no])
    AC_MSG_FAILURE([libtidy test failed (--without-tidy to disable)])
else
    AC_DEFINE([HAVE_LIBTIDY], [1],
        [Define when we have libtidy])
fi

AC_MSG_RESULT([yes])])

AC_SUBST(LIBTIDY_CFLAGS)
AC_SUBST(LIBTIDY_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([locale.h malloc.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset select strdup strerror strstr])

AC_CONFIG_FILES([Makefile
                 doxygen.cfg
                 src/Makefile
                 libkolibre-xmlreader.pc
                 tests/Makefile
])

AC_OUTPUT
