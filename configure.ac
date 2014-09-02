AC_INIT([picoftpd], [0.1])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])

AC_CONFIG_SRCDIR([src/picoftpd.c])

AC_PROG_CC
AC_HEADER_STDC

AC_DEFUN([AC_CHECK_HEADER_MANDATORY], [
	AC_CHECK_HEADER([$1], [], [
		AC_MSG_ERROR([missing header: $1])
	])
])

AC_CHECK_HEADER_MANDATORY(stdio.h)
AC_CHECK_HEADER_MANDATORY(unistd.h)
AC_CHECK_HEADER_MANDATORY(sys/types.h)
AC_CHECK_HEADER_MANDATORY(sys/wait.h)
AC_CHECK_HEADER_MANDATORY(sys/socket.h)
AC_CHECK_HEADER_MANDATORY(netdb.h)
AC_CHECK_HEADER_MANDATORY(netinet/in.h)

AC_CHECK_HEADER_MANDATORY(getopt.h)
AC_SEARCH_LIBS(getopt_long, [gnugetopt], [], [ 
	AC_MSG_ERROR([missing getopt_long()])
])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_OUTPUT
