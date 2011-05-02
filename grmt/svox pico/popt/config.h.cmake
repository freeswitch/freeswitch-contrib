/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#cmakedefine ENABLE_NLS

/* Define to 1 if you have the MacOS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
#cmakedefine HAVE_CFLOCALECOPYCURRENT

/* Define to 1 if you have the MacOS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
#cmakedefine HAVE_CFPREFERENCESCOPYAPPVALUE

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#cmakedefine HAVE_DCGETTEXT

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H

/* Define to 1 if you have the <float.h> header file. */
#cmakedefine HAVE_FLOAT_H

/* Define to 1 if you have the <fnmatch.h> header file. */
#cmakedefine HAVE_FNMATCH_H

/* Define to 1 if you have the `geteuid' function. */
#cmakedefine HAVE_GETEUID

/* Define if the GNU gettext() function is already present or preinstalled. */
#cmakedefine HAVE_GETTEXT

/* Define to 1 if you have the `getuid' function. */
#cmakedefine HAVE_GETUID

/* Define to 1 if you have the <glob.h> header file. */
#cmakedefine HAVE_GLOB_H

/* Define if you have the iconv() function and it works. */
#cmakedefine HAVE_ICONV

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H

/* Define to 1 if you have the <langinfo.h> header file. */
#cmakedefine HAVE_LANGINFO_H

/* Define to 1 if you have the <libintl.h> header file. */
#cmakedefine HAVE_LIBINTL_H

/* Define to 1 if you have the <mcheck.h> header file. */
#cmakedefine HAVE_MCHECK_H

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* Define to 1 if you have the `mtrace' function. */
#cmakedefine HAVE_MTRACE

/* Define to 1 if you have the `setregid' function. */
#cmakedefine HAVE_SETREGID

/* Define to 1 if you have the `srandom' function. */
#cmakedefine HAVE_SRANDOM

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H

/* Define to 1 if you have the `stpcpy' function. */
#cmakedefine HAVE_STPCPY

/* Define to 1 if you have the `strerror' function. */
#ifndef _MSC_VER
/* The check seems to fail on MSVC, but it actually works */
#cmakedefine HAVE_STRERROR
#else
#define HAVE_STRERROR 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* Define to 1 if you have the `vasprintf' function. */
#cmakedefine HAVE_VASPRINTF

/* Define to 1 if you have the `__secure_getenv' function. */
#cmakedefine HAVE___SECURE_GETENV

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#cmakedefine LT_OBJDIR

/* Name of package */
#define PACKAGE "${popt_package}"

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#define PACKAGE_NAME "${popt_package_name}"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "${popt_package_string}"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION

/* Full path to popt top_srcdir. */
#cmakedefine POPT_SOURCE_PATH

/* Full path to default POPT configuration directory */
#define POPT_SYSCONFDIR ${popt_sysconfdir}

/* Define to 1 if the C compiler supports function prototypes. */
#cmakedefine PROTOTYPES

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Version number of package */
#cmakedefine VERSION

/* Number of bits in a file offset, on hosts where this is settable. */
#cmakedefine _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#cmakedefine _LARGE_FILES

/* Define like PROTOTYPES; this can be used by system headers. */
#cmakedefine __PROTOTYPES


