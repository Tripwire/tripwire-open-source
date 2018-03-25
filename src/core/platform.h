//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2018 Tripwire,
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef __PLATFORM_H
#    define __PLATFORM_H

//NOTE: Autoconf is strongly preferred as the Right Way to detect platform-specific features/behaviors.
// These macros should really only be used when autoconf can't get the job done.

//=============================================================================
// Enumerations
//
// For each of these "enumerations" we create unique integers identifying each
// variation.  We group similar items together, such as OS_REDHAT and OS_SLACKWARE

#    define OS_UNKNOWN 0

#    define OS_WIN32 0x0101
#    define OS_CYGWIN 0x0102
#    define OS_DOS_DJGPP 0x0103

#    define OS_LINUX 0x0201
#    define OS_ANDROID 0x0202

#    define OS_FREEBSD 0x0301
#    define OS_NETBSD 0x0302
#    define OS_OPENBSD 0x0303
#    define OS_DARWIN 0x0304
#    define OS_DRAGONFLYBSD 0x0305
#    define OS_MIDNIGHTBSD 0x0306
#    define OS_MIRBSD 0x0307
#    define OS_BITRIG 0x0308
#    define OS_LIBERTYBSD 0x0309

#    define OS_SOLARIS 0x0400
#    define OS_AIX 0x0401
#    define OS_HPUX 0x0402
#    define OS_IRIX 0x0403
#    define OS_OSF1 0x0404

#    define OS_MINIX 0x0501
#    define OS_HURD 0x0502
#    define OS_HAIKU 0x0503
#    define OS_SYLLABLE 0x0504
#    define OS_SKYOS 0x0505
#    define OS_SORTIX 0x0506
#    define OS_MINT 0x0507
#    define OS_AROS 0x0508
#    define OS_RTEMS 0x0509
#    define OS_RISCOS 0x050A
#    define OS_REDOX 0x050B
#    define OS_QNX 0x050C

#    define COMP_UNKNOWN 0
#    define COMP_GCC 0x0001
#    define COMP_CLANG 0x0002

#    define COMP_MSVC 0x0101
#    define COMP_KAI_GCC 0x0201
#    define COMP_KAI_SUNPRO 0x0202
#    define COMP_KAI_GLIBC 0x0203
#    define COMP_KAI_VISUALAGE 0x0204
#    define COMP_KAI_HPANSIC 0x0205
#    define COMP_KAI_IRIX 0x0206
#    define COMP_KAI_OSF1ALPHA 0x0207
#    define COMP_SUNPRO 0x0301

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
//
// PLEASE NOTE:  Do not set any preprocessor variable other than the above three in this
// section.  Use the following sections for anything that does not fall into
// the above catagories.

#    if defined(_WIN32)
#        define OS OS_WIN32
#        define IS_WIN32 1

#    elif defined(__CYGWIN__)
#        define OS OS_CYGWIN
#        define IS_CYGWIN 1

#    elif defined(__DJGPP__)
#        define OS OS_DOS_DJGPP
#        define IS_DOS_DJGPP 1


#    elif defined(__ANDROID__)
#        define OS OS_ANDROID
#        define IS_ANDROID 1

#    elif defined(__linux__)
#        define OS OS_LINUX
#        define IS_LINUX 1


// A herd of BSDs.  Have to detect MidnightBSD before FreeBSD, and MirOS & Bitrig before OpenBSD
// because they also define symbols for their ancestor BSDs.
#    elif defined(__DragonFly__)
#        define OS OS_DRAGONFLYBSD
#        define IS_DRAGONFLYBSD 1

#    elif defined(__MidnightBSD__)
#        define OS OS_MIDNIGHTBSD
#        define IS_MIDNIGHTBSD 1

#    elif defined(__FreeBSD__)
#        define OS OS_FREEBSD
#        define IS_FREEBSD 1

#    elif defined(__NetBSD__)
#        define OS OS_NETBSD
#        define IS_NETBSD 1

#    elif defined(__MirBSD__)
#        define OS OS_MIRBSD
#        define IS_MIRBSD 1

#    elif defined(__Bitrig__)
#        define OS OS_BITRIG
#        define IS_BITRIG 1

#    elif defined(TW_LibertyBSD)
#        define OS OS_LIBERTYBSD
#        define IS_LIBERTYBSD 1

#    elif defined(__OpenBSD__)
#        define OS OS_OPENBSD
#        define IS_OPENBSD 1

#    elif defined(__APPLE__)
#        define OS OS_DARWIN
#        define IS_DARWIN 1


#    elif defined(__sun)
#        define OS OS_SOLARIS
#        define IS_SOLARIS 1

#    elif defined(_AIX)
#        define OS OS_AIX
#        define IS_AIX 1

#    elif defined(__hpux)
#        define OS OS_HPUX
#        define IS_HPUX 1

#    elif defined(__sgi)
#        define OS OS_IRIX
#        define IS_IRIX 1

#    elif defined(TRU64) || defined(__OSF1__)
#        define OS OS_OSF1
#        define IS_OSF1 1


#    elif defined(__minix__)
#        define OS OS_MINIX
#        define IS_MINIX 1

#    elif defined(__gnu_hurd__)
#        define OS OS_HURD
#        define IS_HURD 1

#    elif defined(__HAIKU__)
#        define OS OS_HAIKU
#        define IS_HAIKU 1

#    elif defined(__SYLLABLE__)
#        define OS OS_SYLLABLE
#        define IS_SYLLABLE 1

#    elif defined(SKYOS)
#        define OS OS_SKYOS
#        define IS_SKYOS 1

#    elif defined(_SORTIX_SOURCE)
#        define OS OS_SORTIX
#        define IS_SORTIX 1

#    elif defined(__MINT__)
#        define OS OS_MINT
#        define IS_MINT 1

#    elif defined(__AROS__)
#        define OS OS_AROS
#        define IS_AROS 1

#    elif defined(__rtems__)
#        define OS OS_RTEMS
#        define IS_RTEMS 1

#    elif defined(__riscos__)
#        define OS OS_RISCOS
#        define IS_RISCOS 1

#    elif defined(__redox__)
#        define OS OS_REDOX
#        define IS_REDOX 1

#    elif defined(__QNX__)
#        define OS OS_QNX
#        define IS_QNX 1

#    endif


    /* XXX: COMP may now not resolve, because autoconf may
     *  detect GCC.  This is done in the hopes that all
     *  COMP detections, and indeed both OS & COMP detechtions
     *  will eventualy be done automatically.
     *
     *  This means, the former "#if !defined(COMP)" will
     *  temporarily have to also check the HAVE_[compiler]
     *  #defines until all compilers are checked by autoconf,
     *  at which point this can be removed completely.
     *
     *  PH - 20010311
     */
#    if !defined(COMP) && !defined(HAVE_GCC)
#        error COMP definition did not resolve.  Check "platform.h".
#    endif

//=============================================================================
// Platform Macros (a.k.a. "IS_" macros)
//
// These macros are the "worker bees" of platform.h.  Programmers should use
// these macros rather than comparing PLATFORM to the unique IDs by hand.
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


// complier detection
#    define IS_KAI                                                                                                  \
        (COMP == COMP_KAI_GCC || COMP == COMP_KAI_SUNPRO || COMP == COMP_KAI_GLIBC || COMP == COMP_KAI_VISUALAGE || \
         COMP == COMP_KAI_HPANSIC || COMP == COMP_KAI_IRIX || COMP == COMP_KAI_OSF1ALPHA)
#    define IS_MSVC (COMP == COMP_MSVC)
#    define IS_SUNPRO (COMP == COMP_SUNPRO)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Unicode
#    define SUPPORTS_UNICODE IS_WIN32 // The OS supports Unicode

// KAI 3.4 uses a much improved stl
#    define IS_KAI_3_4 (IS_KAI && (COMP == COMP_KAI_IRIX || COMP == COMP_KAI_OSF1ALPHA || COMP == COMP_KAI_GLIBC))

// Used in twlocale
#    define USE_STD_CPP_LOCALE_WORKAROUND \
        (IS_SUNPRO || (IS_KAI && !IS_KAI_3_4)) // TODO:BAM -- name this something more general.
#    define USE_CLIB_LOCALE IS_KAI || HAVE_GCC
#    define USES_CLIB_DATE_FUNCTION      \
        (USE_CLIB_LOCALE || IS_SUNPRO || \
         IS_MSVC) // if we use clib, can't use C++ time_put, and SUNPRO and MSVC add characters
//#define USE_CLIB_LOCALE         (IS_ALPHA || IS_IRIX || (IS_KAI && !IS_KAI_3_4))

// Threading API
// TODO:mdb -- this is not complete or rigorous on the unix side!!!
#    define SUPPORTS_WIN32_THREADS IS_WIN32
#    define SUPPORTS_POSIX_THREADS (!SUPPORTS_WIN32_THREADS)

// Miscellaneous
#    define WCHAR_IS_16_BITS IS_WIN32
#    define WCHAR_IS_32_BITS IS_UNIX
#    define WCHAR_REP_IS_UCS2 IS_WIN32
// msystem+mpopen fail on Syllable, so use the libc equivalents until we figure out why.
// TODO: Figure out why.
#    define USES_MPOPEN (IS_UNIX && !IS_SYLLABLE)
#    define USES_MSYSTEM (IS_UNIX && !IS_SYLLABLE)
#    define SUPPORTS_WCHART IS_WIN32 // TODO: Remove after getting new ver of KAI
#    define USES_GLIBC ((COMP == COMP_KAI_GLIBC) || HAVE_GCC)
#    define SUPPORTS_MEMBER_TEMPLATES (!IS_SUNPRO)
#    define SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST (!IS_SUNPRO)

#    define SUPPORTS_POSIX_SIGNALS (!IS_DOS_DJGPP)
#    define SUPPORTS_NETWORKING (!IS_SORTIX && !IS_DOS_DJGPP && !IS_REDOX)
#    define SUPPORTS_SYSLOG (HAVE_SYSLOG_H && !IS_SKYOS && !IS_RISCOS)
#    define NEEDS_SWAB_IMPL (IS_CYGWIN || IS_SYLLABLE || IS_ANDROID || IS_SORTIX)
#    define USES_MBLEN (!IS_ANDROID && !IS_AROS)
#    define USES_DEVICE_PATH (IS_AROS || IS_DOS_DJGPP || IS_RISCOS || IS_REDOX)
#    define ICONV_CONST_SOURCE (IS_MINIX)
#    define SUPPORTS_DIRECT_IO (IS_LINUX)
// Linux is the only platform where direct i/o hashing has been tested & works properly so far.

#    define SUPPORTS_TERMIOS (!IS_RTEMS && !IS_REDOX)
// RTEMS errors are probably just a buildsys issue & this will change or go away.
// Redox will probably implement this in the future.

#    define CAN_UNLINK_WHILE_OPEN (!IS_AROS && !IS_RISCOS && !IS_REDOX && !IS_DOS_DJGPP)

#    define SUPPORTS_DOUBLE_SLASH_PATH (IS_CYGWIN)
// POSIX standard says paths beginning with 2 slashes are "implementation defined"
// (see http://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap04.html#tag_04_11 )
// The only platform OST works on (afaik) that actually defines a double-slash behavior is Cygwin
// which uses this syntax for UNC paths.  So we'll allow leading double slashes there, but
// continue removing them on all other platforms

#    define USE_DEV_URANDOM (HAVE_DEV_URANDOM && ENABLE_DEV_URANDOM)

//=============================================================================
// Miscellaneous
//
// Put all items that are not an "IS_" macro here.

#    if IS_BYTE_ALIGNED
#        define BYTE_ALIGN 8
#    else
#        error Unknown Byte alignment
#    endif

// A scalar that matches the sizeof a pointer
typedef unsigned long ptr_size_type; // true for all of our current platforms
                                     // TODO: I would like to use a XXXX_t like name

// Check integer representation
#    if !(USES_2S_COMPLEMENT)
#        error "Tripwire will only work on a 2's complement CPU.  Check \"platform.h\"."
#    endif


#endif // __PLATFORM_H
