//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
//
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
//
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
//
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
///////////////////////////////////////////////////////////////////////////////
// platform.h
//

#ifndef __PLATFORM_H
#define __PLATFORM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_STDINT_H
#include <stdint.h>
#endif

#if HAVE_FEATURES_H
#include <features.h>
#endif

//NOTE: Autoconf is strongly preferred as the Right Way to detect platform-specific features/behaviors.
// These macros should really only be used when autoconf can't get the job done.

///////////////////////////////

#define COMP_UNKNOWN 0
#define COMP_GCC 0x0001
#define COMP_CLANG 0x0002

#define COMP_MSVC 0x0101

// Definitions for the old KAI C++ compiler.
// KCC was EOL'd ages ago, but I'm leaving these definitions here for now
// on the off chance someone's still using it.
#define COMP_KAI 0x0200
#define COMP_KAI_GCC 0x0201
#define COMP_KAI_SUNPRO 0x0202
#define COMP_KAI_GLIBC 0x0203
#define COMP_KAI_VISUALAGE 0x0204
#define COMP_KAI_HPANSIC 0x0205
#define COMP_KAI_IRIX 0x0206
#define COMP_KAI_OSF1ALPHA 0x0207

#define COMP_SUNPRO 0x0301 
#define COMP_XL_C   0x0302
#define COMP_ACC    0x0303


//=============================================================================
// Platform detection
//
// Using boolean logic on predefined compilers variables, detect and set
// PLATFORM preprosessor defines to the unique ID specified above.
//
// The following definitions are set in this section:
//
//      OS                  The OS
//      COMP                The compiler
//      plus IS_xxx macros for the OS & compiler


//////////////////////////////
// Compiler detection

#if defined(HAVE_GCC)
#    define IS_GCC 1

#elif defined(HAVE_CLANG)
#    define IS_CLANG 1

#elif defined(HAVE_MSVC)
#    define IS_MSVC 1

#elif defined(HAVE_KAI_KCC)
#    if !defined(COMP)
#        define COMP COMP_KAI
#    endif
#    define IS_KAI 1

#elif defined(HAVE_ORACLE_SUNCC)
#    define IS_SUNPRO 1

#elif defined(HAVE_IBM_XL_C) || defined(HAVE_IBM_GXLC)
#    define IS_XL_C 1

#elif defined(HAVE_HP_ACC)
#    define IS_HP_ACC 1
#endif


//////////////////////////
// OS detection

#if defined(_WIN32)
#    define IS_WIN32 1

#elif defined(__DJGPP__)
#    define IS_DOS_DJGPP 1

#elif defined(__FreeBSD__)
#    define IS_FREEBSD 1

#elif defined(SKYOS)
#    define IS_SKYOS 1

#elif defined(_SORTIX_SOURCE)
#    define IS_SORTIX 1

#elif defined(__AROS__) || defined(AMIGA)
#    define IS_AROS 1

#elif defined(__riscos__)
#    define IS_RISCOS 1

#elif defined(__redox__)
#    define IS_REDOX 1

#endif


//=============================================================================
// Platform Macros (a.k.a. "IS_" macros)
//
// These macros are the "worker bees" of platform.h.  Programmers should use
// these macros rather than comparing PLATFORM to the unique IDs by hand.
//
// NOTE: Wherever possible, let autotools figure this out & define it in config.h
// rather than hardcoding it here.  If autotools misdefines something, use
// core/fixups.h, which is guaranteed to be (re)included right after config.h
//
// NB: Programmers are STRONGLY ENCOURAGED not to use the OS detection macros
// or compiler detection marcros directly.  Instead they should create
// macros specific to the task at hand.  For example Win32 and Solaris support
// extended permissions for their files.  Rather than check IS_WIN32 || IS_SOLARIS,
// create a new macro called "HAS_EXTENDED_FILE_PERMISSIONS" and use that.
//
// One case where it is reasonable to use the IS_WIN32 or IS_UNIX is when
// you need to protect a #include that is specific to a platform.
//
// Example usage:
//
// #ifdef WORDS_BIGENDIAN
// int network_order = machine_order;
// #else
// int network_order = swap(machine_order);
// #endif



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define CPLUSPLUS_1998_OR_GREATER (__cplusplus >= 199711L)
#define CPLUSPLUS_PRE_1998 !CPLUSPLUS_1998_OR_GREATER

#define CPLUSPLUS_2011_OR_GREATER (__cplusplus >= 201103L)
#define CPLUSPLUS_PRE_2011 !CPLUSPLUS_2011_OR_GREATER

#define CPLUSPLUS_2017_OR_GREATER (__cplusplus >= 201703L)
#define CPLUSPLUS_PRE_2017 !CPLUSPLUS_2017_OR_GREATER

#define CPLUSPLUS_2020_OR_GREATER (__cplusplus >= 202002L)
#define CPLUSPLUS_PRE_2020 !CPLUSPLUS_2020_OR_GREATER
  
// KAI 3.4 uses a much improved stl
#define IS_KAI_3_4 (IS_KAI && (COMP == COMP_KAI_IRIX || COMP == COMP_KAI_OSF1ALPHA || COMP == COMP_KAI_GLIBC))

// Used in twlocale
#define USE_STD_CPP_LOCALE_WORKAROUND \
        (IS_SUNPRO || (IS_KAI && !IS_KAI_3_4)) // TODO:BAM -- name this something more general.

#define USE_CLIB_LOCALE (!HAVE_LOCALE || HAVE_GCC || IS_KAI)

#define USES_CLIB_DATE_FUNCTION      \
        (USE_CLIB_LOCALE || IS_SUNPRO || \
         IS_MSVC) // if we use clib, can't use C++ time_put, and SUNPRO and MSVC add characters

#if defined(__GLIBC__)
  #define USES_GLIBC 1
#endif

#define SUPPORTS_MEMBER_TEMPLATES (!IS_SUNPRO)
#define SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST (!IS_SUNPRO)

// Threading API
#define SUPPORTS_WIN32_THREADS IS_WIN32
#define SUPPORTS_POSIX_THREADS (HAVE_PTHREAD_H)

// Miscellaneous
#define SUPPORTS_C_FILE_IO (HAVE_FOPEN && HAVE_FREAD && HAVE_FCLOSE)
#define SUPPORTS_POSIX_FILE_IO (HAVE_OPEN && HAVE_READ && HAVE_CLOSE)
#define SUPPORTS_SYMLINKS (HAVE_READLINK || HAVE_READLINKAT)

#define ARCHAIC_STL (!HAVE_LOCALE && !HAVE_SSTREAM)

#if (SIZEOF_WCHAR_T == 2)  
#    define WCHAR_IS_16_BITS  1
#    define WCHAR_REP_IS_UCS2 1
#elif (SIZEOF_WCHAR_T == 4)
#    define WCHAR_IS_32_BITS 1
#endif

#define USE_U16STRING ((!WCHAR_IS_16_BITS) && CPLUSPLUS_2011_OR_GREATER)
#define USE_CHAR16_T USE_U16STRING
#define NEED_DBSTRING_IMPL (!WCHAR_IS_16_BITS && !USE_U16STRING)

#define USE_UNIQUE_PTR      CPLUSPLUS_2011_OR_GREATER
#define USE_LAMBDAS         CPLUSPLUS_2011_OR_GREATER
#define USE_UNICODE_ESCAPES CPLUSPLUS_2011_OR_GREATER
#define USE_UNEXPECTED      CPLUSPLUS_PRE_2017

#define SUPPORTS_POSIX_FORK_EXEC ((HAVE_FORK || HAVE_VFORK) && HAVE_EXECVE)

#define USE_DEV_URANDOM (HAVE_DEV_URANDOM && ENABLE_DEV_URANDOM)


// Per-platform special cases. At least some of these could be replaced
// with better autotools detection

// msystem+mpopen fail on Syllable, so use the libc equivalents until we figure out why.
// TODO: Figure out why.
#if !defined(__SYLLABLE__)
#    define USES_MPOPEN  (SUPPORTS_POSIX_FORK_EXEC)
#    define USES_MSYSTEM (SUPPORTS_POSIX_FORK_EXEC)
#endif

#if (!defined(__DJGPP__) && !defined(__MINGW32__))
#    define SUPPORTS_POSIX_SIGNALS 1
#endif

#if (!defined(__DJGPP__) && !defined(SORTIX_SOURCE) && !defined(__redox__))
#    define SUPPORTS_NETWORKING (HAVE_SOCKET)
#endif  

#if (!defined(SKYOS) && !defined(__riscos__))
#    define SUPPORTS_SYSLOG (HAVE_SYSLOG)
#endif

#if (defined(__CYGWIN__) || defined(__SYLLABLE__) || defined(__ANDROID__) || defined(SORTIX_SOURCE))
#   define NEEDS_SWAB_IMPL 1
#else
#   define NEEDS_SWAB_IMPL (!HAVE_SWAB)
#endif

#if defined(__MINGW32__)
#   define SWAB_TAKES_CHAR_PTRS 1
#   define MKDIR_TAKES_SINGLE_ARG 1
#endif

#if (defined(__QNX__) || defined(__QNXNTO__)) && !defined(BBNDK_VERSION_CURRENT)
#   define SWAB_TAKES_CHAR_PTRS 1
#endif

#if (!defined(__ANROID__) && !defined(__AROS__))
#   define USES_MBLEN (HAVE_MBLEN)
#endif

#if (defined(__DJGPP__) || (defined(_WIN32) && !defined(__CYGWIN__)) || (defined(__OS2__) && !defined(__EMX__)))
#   define USES_DOS_DEVICE_PATH 1
#endif

#if defined(__redox__)
#   define USES_URI_PATH 1
#endif

#if (USES_DOS_DEVICE_PATH || USES_URI_PATH || defined(__riscos__) || defined(__AROS__))
#   define USES_DEVICE_PATH 1
#endif

#if defined(__minix__)
#   define ICONV_CONST_SOURCE 1
#endif

#if defined(SKYOS)
#   define READLINK_NULL_TERMINATES 1
#endif

// HP-UX does have posix_fadvise(), but sys/fcntl.h neglects to wrap it in extern "C" on
// at least some flavors of the OS. (see https://community.hpe.com/t5/Languages-and-Scripting/Bacula-try-to-compile-on-hpux11-31/m-p/6843389 )
// The thread indicates this problem can be fixed by editing sys/fcntl.h, and then the special case below
// can be removed. This is left as an exercise for the reader.
#if !defined(__hpux)
#   define SUPPORTS_POSIX_FADVISE (HAVE_POSIX_FADVISE)
#endif

#if defined(__VXWORKS__)
#   define READ_TAKES_CHAR_PTR 1
#endif

#if (!defined(__RTEMS__) && !defined(__redox__))
#  define SUPPORTS_TERMIOS (HAVE_TERMIOS_H)
#endif

// Unlinking an open file (to make a temporary file nobody else can see) is a Posix-ism
// that other platforms generally don't support.
#if (!defined(__CYGWIN__) && !defined(__DJGPP__) && !defined(__riscos__) && !defined(__redox__) && !defined(_WIN32) && !defined(__OS2__) && !IS_AROS)
#   define CAN_UNLINK_WHILE_OPEN 1
#endif

// POSIX standard says paths beginning with 2 slashes are "implementation defined"
// (see http://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap04.html#tag_04_11 )
// The only platform OST is known to work on (afaik) that actually defines a double-slash behavior is Cygwin
// which uses this syntax for UNC paths.  OST also might Just Work under Unix System Services on
// z/OS / MVS / OS/390, which uses a leading double slash for dataset paths, so I'll add that
// to the macro even though I don't have the hardware to actually test this.
//
// Other platform known to use this include IBM z/OS and the ancient Apollo Domain/OS.
#if (defined(__CYGWIN__) || defined(__MVS__))
#   define SUPPORTS_DOUBLE_SLASH_PATH 1
#endif

// Platforms where we might encounter AS/400 native objects,
// which are only sometimes readable via ordinary file API
#if defined(__OS400__) || defined(_AIX)
#    define SUPPORTS_NATIVE_OBJECTS 1
#endif
  
// On most platforms, uname() returns 0 on success, like every other syscall out there.
// However the POSIX standard merely says uname() returns some nonnegative value on success, probably
// so certain vendors could be POSIX-compliant without changing anything.  Solaris seems to return 1
// on success, for example.  If any other examples crop up, add them here, or figure out how to
// do this with autoconf instead.
//
#if defined(__sun)
#   define UNAME_SUCCESS_POSIX 1
#else
#   define UNAME_SUCCESS_ZERO 1
#endif  

// Whether to try unaligned access checks in one unit test of dubious value
#if !defined(__sun) && !defined(__hpux)
#   define ENABLE_ALIGNMENT_TEST 1
#endif

//=============================================================================
// Miscellaneous
//
// Put all items that are not an "IS_" macro here.

// This should ordinarily always be 8, but we'll do it the autoconf way just in case. 
#define BYTE_ALIGN ALIGNOF_LONG_LONG

// A scalar that matches the sizeof a pointer
typedef uintptr_t ptr_size_type;

// Check signed integer representation
// This check has been around for ages, and I'm not sure it's still accurate.
// It's not like there's a lot of contemporary non-2s-complement hardware out there
// to try this on.  Anyway, the configure script always defines USES_2S_COMPLEMENT to 1
// without actually checking anything, so this check wasn't performing a valuable service here.
//
//#if !(USES_2S_COMPLEMENT)
//#    error "Tripwire will only work on a 2's complement CPU.  Check \"platform.h\"."
//#endif


#endif // __PLATFORM_H
