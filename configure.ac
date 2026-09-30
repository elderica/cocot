#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cocot],[1.2-20171118])
AC_CONFIG_SRCDIR([cocot.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
AM_PROG_AR
AC_PROG_RANLIB

# Checks for libraries.

# iconv() is in libc or in libiconv. GNU libiconv renames the functions
# with macros in <iconv.h>, so check by linking a program that uses it.
AC_CHECK_HEADERS([iconv.h],,[AC_MSG_ERROR([iconv.h is not available])])
AC_CACHE_CHECK([for library containing iconv], [cocot_cv_lib_iconv], [
  cocot_cv_lib_iconv=no
  cocot_save_LIBS=$LIBS
  for cocot_lib in '' -liconv; do
    LIBS="$cocot_lib $cocot_save_LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <iconv.h>]],
      [[iconv_t cd = iconv_open("", "");
        iconv(cd, 0, 0, 0, 0);
        iconv_close(cd);]])],
      [cocot_cv_lib_iconv=${cocot_lib:-"none required"}; break])
  done
  LIBS=$cocot_save_LIBS])
AS_CASE([$cocot_cv_lib_iconv],
  [no], [AC_MSG_ERROR([iconv is not available])],
  ["none required"], [],
  [LIBS="$cocot_cv_lib_iconv $LIBS"])

# openpty() and login_tty() are in libc or libutil depending on the platform.
AC_SEARCH_LIBS([openpty], [util])
AC_SEARCH_LIBS([login_tty], [util])
AC_CHECK_FUNCS([openpty login_tty cfmakeraw])

# Checks for header files.
# Headers declaring openpty()/login_tty() vary between platforms.
AC_CHECK_HEADERS([utmp.h libutil.h util.h pty.h])
# Without openpty(), STREAMS modules may have to be pushed (e.g. Solaris).
AS_IF([test "x$ac_cv_func_openpty" != xyes], [AC_CHECK_HEADERS([stropts.h])])

# DEFAULT_TERM_CODE
AC_ARG_WITH([default-term-code],
  [AS_HELP_STRING([--with-default-term-code=CODE],
    [specify default value for term code (default is UTF-8)])],
  [DEFAULT_TERM_CODE=$withval],
  [DEFAULT_TERM_CODE="UTF-8"])
AC_DEFINE_UNQUOTED([DEFAULT_TERM_CODE], ["$DEFAULT_TERM_CODE"], [default value for term code])

# DEFAULT_PROC_CODE
AC_ARG_WITH([default-proc-code],
  [AS_HELP_STRING([--with-default-proc-code=CODE],
    [specify default value for proc code (default is CP932)])],
  [DEFAULT_PROC_CODE=$withval],
  [DEFAULT_PROC_CODE="CP932"])
AC_DEFINE_UNQUOTED([DEFAULT_PROC_CODE], ["$DEFAULT_PROC_CODE"], [default value for proc code])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
