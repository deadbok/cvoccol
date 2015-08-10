#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cvoccol], [0.0.1], [martin.groenholdt@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/cvoccol.c])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

#LT_INIT

dnl
dnl Get SphinxBase from command line if given
dnl
AC_ARG_WITH(sphinxbase,
	AS_HELP_STRING([--with-sphinxbase=DIRECTORY],
			[Look for SphinxBase installation in DIRECTORY.  If this is 'auto',
			 the system-wide installation will be used.]),
			 sphinxbase=$withval)

dnl
dnl Check for system SphinxBase if none was passed to us
dnl Also allow --with-sphinxbase=auto to use system one explicitly
dnl
if test x$sphinxbase = x || test x$sphinxbase = xauto; then
   sphinxbase=

   if test "x$HAVE_PKGCONFIG" = "xno"; then
      SPHINXBASE_CFLAGS = "-I/usr/include/sphinxbase -I/usr/local/include/sphinxbase"
      SPHINXBASE_LIBS = "-lsphinxbase"
      SPHINXBASE_PREFIX="/usr/local"
   else
      PKG_CHECK_MODULES(SPHINXBASE, [sphinxbase],,[
      AC_MSG_FAILURE(dnl
[SphinxBase was not found on your system.
Make sure that you have installed it and that the
PKG_CONFIG_PATH environment variable is set correctly, if
it was installed in a non-standard prefix.])])
      SPHINXBASE_PREFIX=`pkg-config --variable=prefix sphinxbase`
   fi
   
   LIBS="$LIBS $SPHINXBASE_LIBS"
   CPPFLAGS="$CPPFLAGS $SPHINXBASE_CFLAGS"
   #SPHINXBASE_SWIG="$SPHINXBASE_PREFIX/share/sphinxbase/swig"
   AC_CHECK_HEADER(sphinx_config.h,,[AC_MSG_FAILURE([SphinxBase was not found on your system.])])
else
   LIBS="$LIBS -lsphinxbase"
   LDFLAGS="$LDFLAGS -L$sphinxbase/lib -L$sphinxbase/src/libsphinxad -L$sphinxbase/src/libsphinxbase"
   CPPFLAGS="$CPPFLAGS -I$sphinxbase/include -I$sphinxbase/include/sphinxbase"
   #SPHINXBASE_SWIG="$sphinxbase/swig"
fi

dnl
dnl Get PocketSphinx from command line if given
dnl
AC_ARG_WITH(pocketsphinx,
	AS_HELP_STRING([--with-pocketsphinx=DIRECTORY],
			[Look for PocketSphinx installation in DIRECTORY.  If this is 'auto',
			 the system-wide installation will be used.]),
			 pocketsphinx=$withval)


dnl
dnl Check for system PocketSphinx if none was passed to us
dnl Also allow --with-pocketsphinx=auto to use system one explicitly
dnl
if test x$pocketsphinx = x || test x$pocketsphinx = xauto; then
   pocketsphinx=

   if test "x$HAVE_PKGCONFIG" = "xno"; then
      POCKETSPHINX_CFLAGS = "-I/usr/include/pocketsphinx -I/usr/local/include/pocketsphinx"
      POCKETSPHINX_LIBS = "-lpocketsphinx"
      POCKETSPHINX_PREFIX="/usr/local"
   else
      PKG_CHECK_MODULES(POCKETSPHINX, [pocketsphinx],,[
      AC_MSG_FAILURE(dnl
[PocketSphinx was not found on your system.
Make sure that you have installed it and that the
PKG_CONFIG_PATH environment variable is set correctly, if
it was installed in a non-standard prefix.])])
      POCKETSPHINX_PREFIX=`pkg-config --variable=prefix pocketsphinx`
   fi
   
   LIBS="$LIBS $POCKETSPHINX_LIBS"
   CPPFLAGS="$CPPFLAGS $POCKETSPHINX_CFLAGS"
   #POCKETSPHINX_SWIG="$POCKETSPHINX_PREFIX/share/pocketsphinx/swig"
   AC_CHECK_HEADER(sphinx_config.h,,[AC_MSG_FAILURE([PocketSphinx was not found on your system.])])
else
   LIBS="$LIBS -lpocketsphinx"
   LDFLAGS="$LDFLAGS -L$pocketsphinx/src/libpocketsphinx"
   CPPFLAGS="$CPPFLAGS -I$pocketsphinx/include -I$pocketsphinx/include/pocketsphinx"
   #POCKETSPHINX_SWIG="$pocketsphinx/swig"
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
