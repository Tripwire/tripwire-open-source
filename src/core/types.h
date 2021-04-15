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
// types.h -- place to abstract platform-specific type sizes
#ifndef __TYPES_H
#define __TYPES_H

#include "platform.h"

#if HAVE_STDINT_H
#   include <stdint.h>
#endif

#if HAVE_LIMITS_H
#   include <limits.h>
#endif

//-----------------------------------------------------------------------------
// standard TSS types
//-----------------------------------------------------------------------------

// Sensible defaults in case there's no stdint.h
#if !HAVE_STDINT_H
typedef unsigned char  uint8_t;
typedef signed char    int8_t;
typedef short          int16_t;
typedef unsigned short uint16_t;

//typedef float          float32_t;
//typedef double         float64_t;

#if SIZEOF_INT == 4
typedef int           int32_t;
typedef unsigned int  uint32_t;

#elif SIZEOF_LONG == 4
typedef long               int32_t;
typedef unsigned long      uint32_t;
#else
#    error "I don't seem to have a 32-bit integer type on this system."
#endif

#if SIZEOF_LONG == 8
typedef long          int64_t;
typedef unsigned long uint64_t;
#elif SIZEOF_LONG_LONG == 8
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#else
#    error "I don't seem to have a 64-bit integer type on this system."
#endif
#endif // !HAVE_STDINT_H

#ifdef CHAR_MIN
#   define TSS_CHAR_MIN   CHAR_MIN
#else
#   define TSS_CHAR_MIN   (-127 - 1)
#endif

#ifdef CHAR_MAX
#   define TSS_CHAR_MAX   CHAR_MAX
#else
#   define TSS_CHAR_MAX   127
#endif

#define TSS_TCHAR_MIN TSS_CHAR_MIN
#define TSS_TCHAR_MAX TSS_CHAR_MAX

#ifdef INT8_MIN
#   define TSS_INT8_MIN   INT8_MIN
#else
#   define TSS_INT8_MIN (-127 - 1)
#endif

#ifdef INT8_MAX
#   define TSS_INT8_MAX   INT8_MAX
#else
#   define TSS_INT8_MAX 127
#endif

#ifdef UINT8_MAX
#   define TSS_UINT8_MAX  UINT8_MAX
#else
#   define TSS_UINT8_MAX 0xFFU
#endif

#ifdef INT16_MIN
#   define TSS_INT16_MIN  INT16_MIN
#else
#   define TSS_INT16_MIN (-32767 - 1)
#endif

#ifdef INT16_MAX
#   define TSS_INT16_MAX  INT16_MAX
#else
#   define TSS_INT16_MAX 32767
#endif

#ifdef UINT16_MAX
#   define TSS_UINT16_MAX UINT16_MAX
#else
#   define TSS_UINT16_MAX 0xFFFFU
#endif

#ifdef INT32_MIN
#   define TSS_INT32_MIN  INT32_MIN
#else
#   define TSS_INT32_MIN (-2147483647 - 1)
#endif

#ifdef INT32_MAX
#   define TSS_INT32_MAX  INT32_MAX
#else
#   define TSS_INT32_MAX 2147483647
#endif

#ifdef UINT32_MAX
#   define TSS_UINT32_MAX UINT32_MAX
#else
#   define TSS_UINT32_MAX 0xFFFFFFFFU
#endif

#ifdef INT64_MIN
#   define TSS_INT64_MIN  INT64_MIN
#else
#   define TSS_INT64_MIN (-9223372036854775807LL - 1)
#endif

#ifdef INT64_MAX
#   define TSS_INT64_MAX  INT64_MAX
#else
#   define TSS_INT64_MAX 9223372036854775807LL
#endif

#ifdef UINT64_MAX
#   define TSS_UINT64_MAX UINT64_MAX
#else
#   define TSS_UINT64_MAX 0xFFFFFFFFFFFFFFFFULL
#endif


//-----------------------------------------------------------------------------
// Byte Swapping
//-----------------------------------------------------------------------------

inline int16_t SWAPBYTES16(int16_t i)
{

    return ((uint16_t)i >> 8) | ((uint16_t)i << 8);
}

inline int32_t SWAPBYTES32(int32_t i)
{
    return ((uint32_t)i >> 24) | (((uint32_t)i & 0x00ff0000) >> 8) | (((uint32_t)i & 0x0000ff00) << 8) | ((uint32_t)i << 24);
}

inline int64_t SWAPBYTES64(int64_t i)
{
    return ((uint64_t)i >> 56) | (((uint64_t)i & 0x00ff000000000000ULL) >> 40) |
           (((uint64_t)i & 0x0000ff0000000000ULL) >> 24) | (((uint64_t)i & 0x000000ff00000000ULL) >> 8) |
           (((uint64_t)i & 0x00000000ff000000ULL) << 8) | (((uint64_t)i & 0x0000000000ff0000ULL) << 24) |
           (((uint64_t)i & 0x000000000000ff00ULL) << 40) | ((uint64_t)i << 56);
}


#    ifdef WORDS_BIGENDIAN

#        define tw_htonl(x) (x)
#        define tw_ntohl(x) (x)
#        define tw_htons(x) (x)
#        define tw_ntohs(x) (x)
#        define tw_htonll(x) (x) // int64_t versions
#        define tw_ntohll(x) (x)

#    else //!WORDS_BIGENDIAN

#        define tw_htonl(x) SWAPBYTES32((x))
#        define tw_ntohl(x) SWAPBYTES32((x))
#        define tw_htons(x) SWAPBYTES16((x))
#        define tw_ntohs(x) SWAPBYTES16((x))
#        define tw_htonll(x) SWAPBYTES64((x)) // int64_t versions
#        define tw_ntohll(x) SWAPBYTES64((x))

#    endif //WORDS_BIGENDIAN

////////////////////////////////////////////

#    if USE_UNIQUE_PTR
#        define TW_UNIQUE_PTR std::unique_ptr
#    else
#        define TW_UNIQUE_PTR std::auto_ptr
#    endif


#endif // __TYPES_H
