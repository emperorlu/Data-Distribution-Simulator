AC_INIT([Data Distribution Simulator - dadisi],
        [0.9],
        [alberto.miranda@bsc.es],
        [dadisi],
        [http://sourceforge.net/projects/dadisi/])

AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([.])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([build-aux/config.h])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE([1.9 tar-pax dist-bzip2 dist-zip foreign subdir-objects])
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
LT_INIT

AC_SUBST([SO_VERSION], [0:9])
AC_SUBST([API_VERSION], [0.9])

# ==========================================================================
# === permit specifiyng dependencies manually ==============================
# ==========================================================================

# xerces
AC_ARG_WITH([xerces-include-path],
    [AS_HELP_STRING([--with-xerces-include-path],
        [location of the Xerces-C library headers, defaults to /usr/include])],
    [XERCES_CPPFLAGS="-I$withval"],
    [XERCES_CPPFLAGS='-I/usr/include'])
AC_SUBST([XERCES_CPPFLAGS])

AC_ARG_WITH([xerces-lib-path],
    [AS_HELP_STRING([--with-xerces-lib-path],
        [location of the Xerces-C libraries])],
    [XERCES_LIBS="-L$withval -lxerces-c"],
    [XERCES_LIBS='-lxerces-c'])
AC_SUBST([XERCES_LIBS])


# sqlite3
AC_ARG_WITH([sqlite3-include-path],
    [AS_HELP_STRING([--with-sqlite3-include-path],
        [location of the SQLite3 library headers, defaults to /usr/include])],
    [SQLITE3_CPPFLAGS="-I$withval"],
    [SQLITE3_CPPFLAGS='-I/usr/include'])
AC_SUBST([SQLITE3_CPPFLAGS])

AC_ARG_WITH([sqlite3-lib-path],
    [AS_HELP_STRING([--with-sqlite3-lib-path],
        [location of the SQLite3 libraries])],
    [SQLITE3_LIBS="-L$withval -lsqlite3"],
    [SQLITE3_LIBS='-lsqlite3'])
AC_SUBST([SQLITE3_LIBS])


# gmp
AC_ARG_WITH([gmp3-include-path],
    [AS_HELP_STRING([--with-gmp3-include-path],
        [location of the GMP3 library headers, defaults to /usr/include])],
    [GMP3_CPPFLAGS="-I$withval"],
    [GMP3_CPPFLAGS='-I/usr/include'])
AC_SUBST([GMP3_CPPFLAGS])

AC_ARG_WITH([gmp3-lib-path],
    [AS_HELP_STRING([--with-gmp3-lib-path],
        [location of the GMP3 libraries])],
    [GMP3_LIBS="-L$withval -lgmp"],
    [GMP3_LIBS='-lgmp'])
AC_SUBST([GMP3_LIBS])


# gcrypt
AC_ARG_WITH([gcrypt-include-path],
    [AS_HELP_STRING([--with-gcrypt-include-path],
        [location of the gcrypt library headers, defaults to /usr/include])],
    [GCRYPT_CPPFLAGS="-I$withval"],
    [GCRYPT_CPPFLAGS='-I/usr/include'])
AC_SUBST([GCRYPT_CPPFLAGS])

AC_ARG_WITH([gcrypt-lib-path],
    [AS_HELP_STRING([--with-gcrypt-lib-path],
        [location of the GCRYPT libraries])],
    [GCRYPT_LIBS="-L$withval -lgcrypt"],
    [GCRYPT_LIBS='-lgcrypt'])
AC_SUBST([GCRYPT_LIBS])


# ==========================================================================
# === check for libraries ==================================================
# ==========================================================================

BOOST_REQUIRE([1.39])


saved_CPPFLAGS=$CPPFLAGS
CPPFLAGS=$XERCES_CPPFLAGS

AC_CHECK_HEADER(xercesc/dom/DOMElement.hpp, 
                [], 
                [AC_MSG_ERROR([
----------------------------------------------------------------------                
ERROR: Could not find xerces headers.
Please provide a valid path for the headers and library with 
--with-xerces-include-path and --with-xerces-lib-path.
----------------------------------------------------------------------])
                ])


CPPFLAGS=$SQLITE3_CPPFLAGS
AC_CHECK_HEADER(sqlite3.h, 
                [], 
                [AC_MSG_ERROR([
----------------------------------------------------------------------                
ERROR: Could not find sqlite3 headers.
Please provide a valid path for the headers and library with 
--with-sqlite3-include-path and --with-sqlite3-lib-path.
----------------------------------------------------------------------])
                ])


CPPFLAGS=$GMP3_CPPFLAGS
AC_CHECK_HEADER(gmpxx.h, 
                [], 
                [AC_MSG_ERROR([
----------------------------------------------------------------------                
ERROR: Could not find gmp3 headers.
Please provide a valid path for the headers and library with 
--with-gmp3-include-path and --with-gmp3-lib-path.
----------------------------------------------------------------------])
                ])

AC_MSG_CHECKING([whether get_str() accepts mp_exp_t&])
AC_LANG_PUSH(C++)
AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[
#include <gmpxx.h>
]],[[
mp_exp_t exp;
mpf_class c;
c.get_str(exp, 10, 0);
]]),
[get_str_ref='yes'],
[get_str_ref='no'])
AC_MSG_RESULT([$get_str_ref])
AC_LANG_POP()

if [[ "$get_str_ref" == "yes" ]]; then
    GMP3_CPPFLAGS+=" -Dmy_gmp_ref"
fi

CPPFLAGS=$GCRYPT_CPPFLAGS
AC_CHECK_HEADER(gcrypt.h, 
                [], 
                [AC_MSG_ERROR([
----------------------------------------------------------------------                
ERROR: Could not find gcrypt headers. 
Please provide a valid path for the headers and the library with 
--with-gcrypt-include-path and --with-gcrypt-lib-path.
----------------------------------------------------------------------])
                ])

CPPFLAGS=$saved_CPPFLAGS

AC_OUTPUT
