#
# José Angel Sánchez Caso ( altomaltes@gijon.es )
#
#  Jul 2004
#

AC_INIT([libnsfb],[1.0.0])
AC_CONFIG_SRCDIR([libnsfb.c])
AM_INIT_AUTOMAKE( [subdir-objects]  )

AC_REVISION($Revision: 1.0.0.1 $)
AM_CONFIG_HEADER( nsfbcfg.h  )
AC_CONFIG_MACRO_DIR([m4])

dnl# Set some basic variables
DATE="`date '+%d %B %Y'`"
SVN_BUILD=m4_esyscmd_s([sh -c "svnversion | cut -f1 -d: "])
SVN_BUILD=m4_esyscmd_s([sh -c "./bld"])
AC_REVISION($Revision: $SVN_BUILD $)

AC_SUBST(DATE)
BUILD_DATE="`date`"
AC_DEFINE_UNQUOTED(BUILD_DATE, "$BUILD_DATE", [ build date ])
AUTHORS="Jose Angel Caso Sanchez (JACS) jascaso@gijon.es"
AC_SUBST(AUTHORS)
AC_DEFINE_UNQUOTED(AUTHORS, "$AUTHORS", [ library authors ] )

CPPFLAGS+="-I../../include"

dnl Several pure type stuff

AC_CHECK_SIZEOF(int      , 4 )
AC_CHECK_SIZEOF(void *   , 4 )
AC_CHECK_SIZEOF(long long, 8 )
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE

dnl Traditional typedefs and replacements 

AC_TYPE_MODE_T
AC_CHECK_TYPE(off_t, long)
AC_CHECK_TYPE(size_t, long)
AC_CHECK_TYPE(byte,  unsigned char )
AC_CHECK_TYPE(word,  unsigned short)
AC_CHECK_TYPE(dword, unsigned int )

LT_INIT

dnl Detect windows

AM_CONDITIONAL( WINDOWS,   test "${version_type}" = "windows" )

dnl needed for DRM driver
AC_CHECK_LIB( drm, drmError, [LIBDRM="-ldrm" AC_DEFINE(HAVE_DRM, 1, [ Define if you have drm. ])]
            , [AC_MSG_WARN([drm is required for GPU driver ])]
            , [])

dnl needed for GIF format 
AC_CHECK_LIB( gif, gifError, [LIBGIF="-lgif" AC_DEFINE(HAVE_GIF, 1, [ Define if you have gif support. ])]
            , [AC_MSG_WARN([gif is required for GIF image format support ])]
            , [])

dnl needs freetype, so check for it here
PKG_CHECK_MODULES( FT2, freetype2 >= 2.0.9, HAVE_FT2="yes", [ AC_CHECK_FT2(2.0.9, HAVE_FT2="yes", HAVE_FT2="no") ])
dnl make the HAVE_FT2 variable available to automake and Makefile.am




AC_PATH_PROG([SLACKBUILD], [makepkg], [notfound])

dnl AC_ARG_ENABLE([slack],
dnl     AS_HELP_STRING([--disable-slack], [No general el paquete de instalacion]))
dnl AS_IF([test "x$enable_slack" != "xno"], [ enable_slack=yes ])

AC_ARG_ENABLE([slack],
  [AS_HELP_STRING([--enable-slack=@<:@yes/no@:>@],
    [Configure for building SLACKWARE package @<:@default=yes@:>@ (requires makepkg)])],
  [AS_IF([test "x$enable_slack" = xyes],
    [AS_IF([test "x$SLACKBUILD" = xnotfound],
      [AC_MSG_ERROR([makepkg is required for --enable-slack])],
        [$test "x$SLACKBUILD" = xnotfound],
          [AC_MSG_ERROR([makepkg is required for --enable-slack])])])],
  [enable_slack=yes])

AM_CONDITIONAL(BUILDING_SLACK, test "x$enable_slack" = "xyes" )



dnl Publish results

AC_SUBST(AUTHORS,"Jose Angel Caso Sanchez (JACS) jascaso@gijon.es" )
AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)
AC_SUBST(DATE,[`date '+%d %B %Y'`])
AC_SUBST(DIST_ROOT)
AC_SUBST(SVN_BUILD)

AC_DEFINE_UNQUOTED(AUTHORS, "$AUTHORS", [ library authors ] )

dnl Output stuff

AC_PROG_CC


AC_CONFIG_FILES([Makefile arena/Makefile])
AC_OUTPUT

echo "

Configuration:

----------------------------------------------------

  Source code location:       $srcdir
  Host System Type:           $host
  Preprocessor:               $CC $CPPFLAGS
  Compiler:                   $CC $CFLAGS
  Linker:                     $CC $LDFLAGS $LIBS
  Install path:               $prefix

  freetype: $FT2_CFLAGS
            $FT2_LIBS

  gif: $HAVE_GIF

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Now type 'make' to build $PACKAGE $VERSION $SVN_BUILD
"

