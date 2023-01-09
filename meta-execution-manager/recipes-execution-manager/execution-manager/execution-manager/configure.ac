#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#
AC_INIT([execution_manager], [1.0], [lukas.stahlbock@iav.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])

AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_HEADERS([pthread.h])

dnl embedded yajl
AC_ARG_ENABLE(embedded-yajl,
AS_HELP_STRING([--enable-embedded-yajl], [Statically link a modified yajl version]),
[
case "${enableval}" in
  yes) embedded_yajl=true ;;
  no)  embedded_yajl=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-embedded-yajl) ;;
esac],[embedded_yajl=false])

AM_CONDITIONAL([HAVE_EMBEDDED_YAJL], [test x"$embedded_yajl" == xtrue])
AM_COND_IF([HAVE_EMBEDDED_YAJL], [], [
AC_SEARCH_LIBS(yajl_tree_get, [yajl], [AC_DEFINE([HAVE_YAJL], 1, [Define if libyajl is available])], [AC_MSG_ERROR([*** libyajl headers not found])])
PKG_CHECK_MODULES([YAJL], [yajl >= 2.0.0])
])

AC_SEARCH_LIBS([argp_parse], [argp], [], [AC_MSG_ERROR([*** argp functions not found - install libargp or argp_standalone])])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT