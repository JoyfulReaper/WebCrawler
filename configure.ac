AC_PREREQ([2.69])
AC_INIT([WebCrawler], [0.0.1], [xvegan88x@gmail.com])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([/src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)
AC_PROG_RANLIB
AM_PROG_AR

# Check for C++11
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

# Checks for libraries
AX_PROG_LUA
AX_LUA_HEADERS
AX_LUA_LIBS

AX_BOOST_BASE([1.54], [], [AC_MSG_ERROR[Boost is required, see boost.org]])
# AX_BOOST_FILESYSTEM
AX_BOOST_SYSTEM
AX_BOOST_THREAD
# AX_BOOST_SERIALIZATION

AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile
		 src/Makefile])
AC_OUTPUT
