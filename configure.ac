AC_INIT([picoftpd], [0.1])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])

AC_CONFIG_SRCDIR([src/picoftpd.c])

AC_PROG_CC
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([unistd.h sys/types.h sys/socket.h sys/wait.h])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_OUTPUT
