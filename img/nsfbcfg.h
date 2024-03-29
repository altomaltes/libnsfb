/* nsfbcfg.h.  Generated from nsfbcfg.h.in by configure.  */
/* nsfbcfg.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* library authors */
#define AUTHORS "Jose Angel Caso Sanchez (JACS) jascaso@gijon.es"

/* build date */
#define BUILD_DATE "Thu Mar  2 15:21:35 CET 2023"

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define if you have drm. */
#define HAVE_DRM 1

/* Define if you have gif support. */
/* #undef HAVE_GIF */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libnsfb"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "libnsfb"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libnsfb 1.0.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libnsfb"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.0"

/* The size of `int ', as computed by sizeof. */
#define SIZEOF_INT______ 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `void * ', as computed by sizeof. */
#define SIZEOF_VOID_P___ 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.0.0"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to `unsigned char ' if <sys/types.h> does not define. */
#define byte unsigned char

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `unsigned int ' if <sys/types.h> does not define. */
#define dword unsigned int

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `unsigned short' if <sys/types.h> does not define. */
#define word unsigned short
