AC_PREREQ([2.69])
AC_INIT([shell], [1], [darrench3140@gmail.com])
AC_CONFIG_SRCDIR([src/shell.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build])

AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MMAP
AC_CHECK_FUNCS([dup2 getcwd strtol strtoul])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
