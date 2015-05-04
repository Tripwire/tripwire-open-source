//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
# include <config.h>
#endif

#ifndef __PLATFORM_H
#define __PLATFORM_H

//=============================================================================
// Enumerations
// 
// For each of these "enumerations" we create unique integers identifying each
// variation.  We group similar items together, such as OS_REDHAT and OS_SLACKWARE

#define OS_UNKNOWN		0
#define OS_WIN32		0x0101
#define OS_AIX			0x0401
#define OS_HPUX			0x0501
#define OS_IRIX			0x0601
#define OS_OSF1			0x0701

#define COMP_UNKNOWN		0
#define COMP_MSVC		    0x0101
#define COMP_KAI_GCC		0x0201
#define COMP_KAI_SUNPRO		0x0202
#define COMP_KAI_GLIBC		0x0203
#define COMP_KAI_VISUALAGE	0x0204
#define COMP_KAI_HPANSIC	0x0205
#define COMP_KAI_IRIX		0x0206
#define COMP_KAI_OSF1ALPHA	0x0207
#define COMP_SUNPRO		    0x0301

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

#if defined(_WIN32) 
    #define OS                  OS_WIN32

    #if defined(_MSC_VER)
        #define COMP            COMP_MSVC
    #else
        #error _MSC_VER not defined.  MSVC is currently the only supported compiler
    #endif

#elif defined(_IRIX)
    #define OS                  OS_IRIX
    #define COMP                COMP_KAI_IRIX

#elif defined(_ALPHA)
    #define OS			        OS_OSF1
    #define COMP		        COMP_KAI_OSF1ALPHA

#elif defined (_HPUX)
	#define OS					OS_HPUX
	#define COMP				COMP_KAI_HPANSIC

#else
//	OK for OS not to resolve, it's being phased out.
//    #error Unknown OS
#endif

#if !defined(OS)
//	OK for OS not to resolve, it's being phased out.
//    #error OS definition did not resolve.  Check "platform.h".
#endif
	/* XXX: COMP may now not resolve, because autoconf may
	 *	detect GCC.  This is done in the hopes that all
	 *	COMP detections, and indeed both OS & COMP detechtions
	 *	will eventualy be done automatically.
	 *
	 *	This means, the former "#if !defined(COMP)" will
	 *	temporarily have to also check the HAVE_[compiler]
	 *	#defines until all compilers are checked by autoconf,
	 *	at which point this can be removed completely.
	 *
	 *	PH - 20010311
	 */
#if !defined(COMP) && !defined(HAVE_GCC)
    #error COMP definition did not resolve.  Check "platform.h".
#endif

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

// OS detection 
// Note: Avoid using these if possible (see above)
#define IS_WIN32    (OS == OS_WIN32)
#define IS_AIX		(OS == OS_AIX)
#define IS_HPUX		(OS == OS_HPUX)
#define IS_IRIX		(OS == OS_IRIX)
#define IS_OSF1		(OS == OS_OSF1)

// complier detection
#define IS_KAI      (COMP == COMP_KAI_GCC || COMP == COMP_KAI_SUNPRO || COMP == COMP_KAI_GLIBC || COMP == COMP_KAI_VISUALAGE || COMP == COMP_KAI_HPANSIC || COMP == COMP_KAI_IRIX || COMP == COMP_KAI_OSF1ALPHA)
#define IS_MSVC     (COMP == COMP_MSVC)
#define IS_SUNPRO   (COMP == COMP_SUNPRO)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// Unicode
#define SUPPORTS_UNICODE        IS_WIN32    // The OS supports Unicode

// KAI 3.4 uses a much improved stl
#define IS_KAI_3_4              (IS_KAI && (COMP == COMP_KAI_IRIX || COMP == COMP_KAI_OSF1ALPHA || COMP == COMP_KAI_GLIBC))

// Used in twlocale
#define USE_STD_CPP_LOCALE_WORKAROUND (IS_SUNPRO || (IS_KAI && !IS_KAI_3_4))  // TODO:BAM -- name this something more general.
#define USE_CLIB_LOCALE         IS_KAI || HAVE_GCC
#define USES_CLIB_DATE_FUNCTION ( USE_CLIB_LOCALE || IS_SUNPRO || IS_MSVC ) // if we use clib, can't use C++ time_put, and SUNPRO and MSVC add characters
//#define USE_CLIB_LOCALE         (IS_ALPHA || IS_IRIX || (IS_KAI && !IS_KAI_3_4))

// Threading API
// TODO:mdb -- this is not complete or rigorous on the unix side!!! 
#define SUPPORTS_WIN32_THREADS	IS_WIN32
#define SUPPORTS_POSIX_THREADS	(!SUPPORTS_WIN32_THREADS)

// Miscellaneous
#define FSEEK_TAKES_INT32       IS_UNIX     // True if fseek takes 32-bit offsets
#define USE_OUTPUT_DEBUG_STRING IS_WIN32    // Use the Win32 OutputDebugString() for debug messages.
#define SUPPORTS_MAPI           IS_WIN32
#define WCHAR_IS_16_BITS        IS_WIN32
#define WCHAR_IS_32_BITS        IS_UNIX
#define WCHAR_REP_IS_UCS2       IS_WIN32
#define USES_MPOPEN             IS_UNIX
#define USES_WINSOCK            IS_WIN32
#define SUPPORTS_WCHART         IS_WIN32    // TODO: Remove after getting new ver of KAI
#define USES_GLIBC              ((COMP == COMP_KAI_GLIBC) || HAVE_GCC)
#define SUPPORTS_EVENTLOG       IS_WIN32
#define SUPPORTS_MEMBER_TEMPLATES               ( ! IS_SUNPRO )
#define SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST    ( ! IS_SUNPRO )

//=============================================================================
// Miscellaneous
//
// Put all items that are not an "IS_" macro here.  

#if IS_BYTE_ALIGNED            
    #define BYTE_ALIGN      8
#else
    #error  Unknown Byte alignment
#endif

// A scalar that matches the sizeof a pointer
typedef unsigned long ptr_size_type;    // true for all of our current platforms
                                        // TODO: I would like to use a XXXX_t like name

// Check integer representation
#if !(USES_2S_COMPLEMENT)
    #error "Tripwire will only work on a 2's complement CPU.  Check \"platform.h\"."
#endif




#endif // __PLATFORM_H

