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

AC_CHECK_HEADER_MANDATORY(arpa/inet.h)
AC_CHECK_HEADER_MANDATORY(fcntl.h)
AC_CHECK_HEADER_MANDATORY(netinet/in.h)
AC_CHECK_HEADER_MANDATORY(poll.h)

AC_CHECK_HEADER_MANDATORY(strings.h)
AC_CHECK_HEADER_MANDATORY(sys/mman.h)
AC_CHECK_HEADER_MANDATORY(sys/socket.h)
AC_CHECK_HEADER_MANDATORY(sys/types.h)
AC_CHECK_HEADER_MANDATORY(sys/wait.h)
AC_CHECK_HEADER_MANDATORY(unistd.h)

AC_CHECK_HEADER_MANDATORY(semaphore.h)
AC_SEARCH_LIBS(sem_wait, [pthread], [], [
    AC_MSG_ERROR([missing POSIX semaphores])
])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_OUTPUT
