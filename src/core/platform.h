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

//NOTE: Autoconf is strongly preferred as the Right Way to detect platform-specific features/behaviors.
// These macros should really only be used when autoconf can't get the job done.

//=============================================================================
// Enumerations
//
// For each of these "enumerations" we create unique integers identifying each
// variation.  We group similar items together, such as OS_REDHAT and OS_SLACKWARE

#    define OS_UNKNOWN 0

#    define OS_WIN32       0x0101
#    define OS_CYGWIN      0x0102
#    define OS_DOS_DJGPP   0x0103
#    define OS_WIN32_MINGW 0x0104
#    define OS_OS2         0x0105
#    define OS_OS2_EMX     0x0106

#    define OS_LINUX   0x0201
#    define OS_ANDROID 0x0202

#    define OS_FREEBSD      0x0301
#    define OS_NETBSD       0x0302
#    define OS_OPENBSD      0x0303
#    define OS_DARWIN       0x0304
#    define OS_DRAGONFLYBSD 0x0305
#    define OS_MIDNIGHTBSD  0x0306
#    define OS_MIRBSD       0x0307
#    define OS_BITRIG       0x0308
#    define OS_LIBERTYBSD   0x0309
#    define OS_BSDI         0x030A

#    define OS_SOLARIS 0x0400
#    define OS_AIX     0x0401
#    define OS_HPUX    0x0402
#    define OS_IRIX    0x0403
#    define OS_OSF1    0x0404
#    define OS_OS400   0x0405
#    define OS_MVS     0x0406

#    define OS_MINIX   0x0501
#    define OS_HURD    0x0502
#    define OS_HAIKU   0x0503
#    define OS_SYLLABLE 0x0504
#    define OS_SKYOS   0x0505
#    define OS_SORTIX  0x0506
#    define OS_MINT    0x0507
#    define OS_AROS    0x0508
#    define OS_RTEMS   0x0509
#    define OS_RISCOS  0x050A
#    define OS_REDOX   0x050B
#    define OS_QNX     0x050C
#    define OS_VXWORKS 0x050D
#    define OS_LYNXOS  0x050E
#    define OS_OS9     0x050F // For Microware's OS-9, not the Apple one.
#    define OS_PLAN9   0x0510


///////////////////////////////

#    define COMP_UNKNOWN 0
#    define COMP_GCC 0x0001
#    define COMP_CLANG 0x0002

#    define COMP_MSVC 0x0101

// Definitions for the old KAI C++ compiler.
// KCC was EOL'd ages ago, but I'm leaving these definitions here for now
// on the off chance someone's still using it.
#    define COMP_KAI 0x0200
#    define COMP_KAI_GCC 0x0201
#    define COMP_KAI_SUNPRO 0x0202
#    define COMP_KAI_GLIBC 0x0203
#    define COMP_KAI_VISUALAGE 0x0204
#    define COMP_KAI_HPANSIC 0x0205
#    define COMP_KAI_IRIX 0x0206
#    define COMP_KAI_OSF1ALPHA 0x0207

#    define COMP_SUNPRO 0x0301 
#    define COMP_XL_C   0x0302
#    define COMP_ACC    0x0303


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

#    if defined(HAVE_GCC)
#        define COMP COMP_GCC
#        define IS_GCC 1

#    elif defined(HAVE_CLANG)
#        define COMP COMP_CLANG
#        define IS_CLANG 1

#    elif defined(HAVE_MSVC)
#        define COMP COMP_MSVC
#        define IS_MSVC 1

#    elif defined(HAVE_KAI_KCC)
#        if !defined(COMP)
#            define COMP COMP_KAI
#        endif
#        define IS_KAI 1

#    elif defined(HAVE_ORACLE_SUNCC)
#        define COMP COMP_SUNPRO
#        define IS_SUNPRO 1

#    elif defined(HAVE_IBM_XL_C) || defined(HAVE_IBM_GXLC)
#        define COMP COMP_XL_C
#        define IS_XL_C 1

#    elif defined(HAVE_HP_ACC)
#        define COMP COMP_ACC
#        define IS_HP_ACC 1
#    endif


//////////////////////////
// OS detection

#    if defined(_WIN32)
#        if defined(__MINGW32__)
#            define OS OS_WIN32_MINGW
#            define IS_MINGW 1
#        else  	     
#            define OS OS_WIN32
#        endif
#        define IS_WIN32 1

#    elif defined(__CYGWIN__)
#        define OS OS_CYGWIN
#        define IS_CYGWIN 1

#    elif defined(__DJGPP__)
#        define OS OS_DOS_DJGPP
#        define IS_DOS_DJGPP 1

#    elif defined(__OS2__)
#        if defined(__EMX__)
#            define OS OS_OS2_EMX
#            define IS_EMX 1
#        else
#            define OS OS_OS2
#        endif
#        define IS_OS2 1

// Detect Android first, since Linux macros are also defined there
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

// Detect LibertyBSD first since OpenBSD macros are also defined.
// Autotools can tell them apart by target triplet, so we define
// 'TW_LibertybSD' there and pass it in.
#    elif defined(TW_LibertyBSD)
#        define OS OS_LIBERTYBSD
#        define IS_LIBERTYBSD 1

#    elif defined(__OpenBSD__)
#        define OS OS_OPENBSD
#        define IS_OPENBSD 1

#    elif defined(__APPLE__)
#        define OS OS_DARWIN
#        define IS_DARWIN 1


// Next up, SysV Unixes, commercial & otherwise, and platforms that
// resemble SysV to varying degrees
#    elif defined(__sun)
#        define OS OS_SOLARIS
#        define IS_SOLARIS 1

#    elif defined(_AIX)
#        define OS OS_AIX
#        define IS_AIX 1
#        if defined(__PASE__)
#            define IS_PASE 1
#        endif

#    elif defined(__hpux)
#        define OS OS_HPUX
#        define IS_HPUX 1

#    elif defined(__sgi)
#        define OS OS_IRIX
#        define IS_IRIX 1

#    elif defined(TRU64) || defined(__OSF1__)
#        define OS OS_OSF1
#        define IS_OSF1 1

#    elif defined(__OS400__)
#        define OS OS_OS400
#        define IS_OS400 1

#    elif defined(__MVS__)
#        define OS OS_MVS
#        define IS_MVS 1


// Anything else that has a vaguely POSIX-esque file API 
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

#    elif defined(__QNX__) || defined(__QNXNTO__)
#        define OS OS_QNX
#        define IS_QNX 1

#    elif defined(__VXWORKS__)
#        define OS OS_VXWORKS
#        define IS_VXWORKS 1

#    elif defined(__Lynx__)
#        define OS OS_LYNXOS
#        define IS_LYNXOS 1

#    elif defined(__OS9000)
#        define OS OS_OS9
#        define IS_OS9 1

#    elif defined(EPLAN9)
#        define OS OS_PLAN9
#        define IS_PLAN9 1

#    endif


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

#    define CPLUSPLUS_2011_OR_GREATER (__cplusplus >= 201103L)
#    define CPLUSPLUS_PRE_2011 !CPLUSPLUS_2011_OR_GREATER

#    define CPLUSPLUS_2017_OR_GREATER (__cplusplus >= 201703L)
#    define CPLUSPLUS_PRE_2017 !CPLUSPLUS_2011_OR_GREATER

// KAI 3.4 uses a much improved stl
#    define IS_KAI_3_4 (IS_KAI && (COMP == COMP_KAI_IRIX || COMP == COMP_KAI_OSF1ALPHA || COMP == COMP_KAI_GLIBC))

// Used in twlocale
#    define USE_STD_CPP_LOCALE_WORKAROUND \
        (IS_SUNPRO || (IS_KAI && !IS_KAI_3_4)) // TODO:BAM -- name this something more general.

#    define USE_CLIB_LOCALE (!HAVE_LOCALE || HAVE_GCC || IS_KAI)

#    define USES_CLIB_DATE_FUNCTION      \
        (USE_CLIB_LOCALE || IS_SUNPRO || \
         IS_MSVC) // if we use clib, can't use C++ time_put, and SUNPRO and MSVC add characters


// Threading API
#    define SUPPORTS_WIN32_THREADS IS_WIN32
#    define SUPPORTS_POSIX_THREADS (HAVE_PTHREAD_H)

// Miscellaneous
#    define SUPPORTS_C_FILE_IO (HAVE_FOPEN && HAVE_FREAD && HAVE_FCLOSE)
#    define SUPPORTS_POSIX_FILE_IO (HAVE_OPEN && HAVE_READ && HAVE_CLOSE)
#    define SUPPORTS_SYMLINKS (HAVE_READLINK || HAVE_READLINKAT)

#    if (SIZEOF_WCHAR_T == 2)  
#        define WCHAR_IS_16_BITS  1
#        define WCHAR_REP_IS_UCS2 1
#    elif (SIZEOF_WCHAR_T == 4)
#        define WCHAR_IS_32_BITS 1
#    endif

#    define USE_U16STRING ((!WCHAR_IS_16_BITS) && CPLUSPLUS_2011_OR_GREATER)
#    define USE_CHAR16_T USE_U16STRING
#    define NEED_DBSTRING_IMPL (!WCHAR_IS_16_BITS && !USE_U16STRING)

#    define USE_UNIQUE_PTR      CPLUSPLUS_2011_OR_GREATER
#    define USE_LAMBDAS         CPLUSPLUS_2011_OR_GREATER
#    define USE_UNICODE_ESCAPES CPLUSPLUS_2011_OR_GREATER
#    define USE_UNEXPECTED      CPLUSPLUS_PRE_2017

#    define SUPPORTS_POSIX_FORK_EXEC (HAVE_FORK && HAVE_EXECVE)
// msystem+mpopen fail on Syllable, so use the libc equivalents until we figure out why.
// TODO: Figure out why.
#    define USES_MPOPEN (SUPPORTS_POSIX_FORK_EXEC && !IS_SYLLABLE)
#    define USES_MSYSTEM (SUPPORTS_POSIX_FORK_EXEC && !IS_SYLLABLE)
  
//#    define SUPPORTS_WCHART IS_WIN32 // TODO: Remove this?
#    define USES_GLIBC ((COMP == COMP_KAI_GLIBC) || HAVE_GCC)
#    define SUPPORTS_MEMBER_TEMPLATES (!IS_SUNPRO)
#    define SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST (!IS_SUNPRO)

#    define SUPPORTS_POSIX_SIGNALS (!IS_DOS_DJGPP && !IS_MINGW)
#    define SUPPORTS_NETWORKING (HAVE_SOCKET && !IS_SORTIX && !IS_DOS_DJGPP && !IS_REDOX)
#    define SUPPORTS_SYSLOG (HAVE_SYSLOG && !IS_SKYOS && !IS_RISCOS)
#    define NEEDS_SWAB_IMPL (IS_CYGWIN || IS_SYLLABLE || IS_ANDROID || IS_SORTIX)
#    define USES_MBLEN (!IS_ANDROID && !IS_AROS)
#    define USES_DOS_DEVICE_PATH (IS_DOS_DJGPP || (IS_WIN32 && !IS_CYGWIN) || (IS_OS2 && !IS_EMX))
#    define USES_DEVICE_PATH (IS_AROS || IS_RISCOS || IS_REDOX || USES_DOS_DEVICE_PATH)
#    define ICONV_CONST_SOURCE (IS_MINIX)

// Linux is the only platform where direct i/o hashing has been tested & works properly so far.
#    define SUPPORTS_DIRECT_IO (IS_LINUX)

// HP-UX does have posix_fadvise(), but sys/fcntl.h neglects to wrap it in extern "C" on
// at least some flavors of the OS. (see https://community.hpe.com/t5/Languages-and-Scripting/Bacula-try-to-compile-on-hpux11-31/m-p/6843389 )
// The thread indicates this problem can be fixed by editing sys/fcntl.h, and then the !IS_HPUX below
// can be removed. This is left as an exercise for the reader.
#    define SUPPORTS_POSIX_FADVISE (HAVE_POSIX_FADVISE && !IS_HPUX)

#    define READ_TAKES_CHAR_PTR  (IS_VXWORKS)

// RTEMS errors are probably just a buildsys issue & this will change or go away.
// Redox will probably implement this in the future.
#    define SUPPORTS_TERMIOS (HAVE_TERMIOS_H && (!IS_RTEMS && !IS_REDOX))


// Unlinking an open file (to make a temporary file nobody else can see) is a Posix-ism
// that other platforms generally don't support.
#    define CAN_UNLINK_WHILE_OPEN (!IS_AROS && !IS_RISCOS && !IS_REDOX && !IS_DOS_DJGPP)


// POSIX standard says paths beginning with 2 slashes are "implementation defined"
// (see http://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap04.html#tag_04_11 )
// The only platform OST is known to work on (afaik) that actually defines a double-slash behavior is Cygwin
// which uses this syntax for UNC paths.  OST also might Just Work under Unix System Services on
// z/OS / MVS / OS/390, which uses a leading double slash for dataset paths, so I'll add that
// to the macro even though I don't have the hardware to actually test this.
//
// Other platform known to use this include IBM z/OS and the ancient Apollo Domain/OS.
#    define SUPPORTS_DOUBLE_SLASH_PATH (IS_CYGWIN || IS_MVS)


#    define USE_DEV_URANDOM (HAVE_DEV_URANDOM && ENABLE_DEV_URANDOM)

// Platforms where we might encounter AS/400 native objects,
// which are only sometimes readable via ordinary file API
#    define SUPPORTS_NATIVE_OBJECTS (IS_AIX || IS_OS400)

  
// On most platforms, uname() returns 0 on success, like every other syscall out there.
// However the POSIX standard merely says uname() returns some nonnegative value on success, probably
// so certain vendors could be POSIX-compliant without changing anything.  Solaris seems to return 1
// on success, for example.  If any other examples crop up, add them here.
#if IS_SOLARIS
#   define UNAME_SUCCESS_POSIX 1
#else
#   define UNAME_SUCCESS_ZERO 1
#endif  

// Work around single-arg mkdir on MinGW.
// consider using autoconf AX_FUNC_MKDIR if
// we need to handle any more cases here
/*#if IS_MINGW
#   define mkdir(a,b) mkdir(a)
#endif*/

//=============================================================================
// Miscellaneous
//
// Put all items that are not an "IS_" macro here.

// This should ordinarily always be 8, but we'll do it the autoconf way just in case. 
#define BYTE_ALIGN ALIGNOF_LONG_LONG

// A scalar that matches the sizeof a pointer
typedef uintptr_t ptr_size_type;

// Check integer representation
#    if !(USES_2S_COMPLEMENT)
#        error "Tripwire will only work on a 2's complement CPU.  Check \"platform.h\"."
#    endif


#endif // __PLATFORM_H
