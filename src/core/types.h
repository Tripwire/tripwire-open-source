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
// types.h -- place to abstract platform-specific type sizes
#ifndef __TYPES_H
#define __TYPES_H

#include "platform.h"

#if HAVE_STDINT_H
#include <stdint.h>
#endif


//-----------------------------------------------------------------------------
// standard TSS types
//-----------------------------------------------------------------------------

typedef unsigned char byte; // platform-independent

typedef signed char    int8;
typedef short          int16;
typedef float          float32;
typedef double         float64;
typedef unsigned char  uint8;
typedef unsigned short uint16;

#if HAVE_STDINT_H
typedef int32_t  int32;
typedef uint32_t uint32;

#elif SIZEOF_INT == 4
typedef int           int32;
typedef unsigned int  uint32;

#elif SIZEOF_LONG == 4
typedef long               int32;
typedef unsigned long      uint32;
#else
#    error "I don't seem to have a 32-bit integer type on this system."
#endif

#if HAVE_STDINT_H
typedef int64_t  int64;
typedef uint64_t uint64;
#elif SIZEOF_LONG == 8
typedef long          int64;
typedef unsigned long uint64;
#elif SIZEOF_LONG_LONG == 8
typedef long long          int64;
typedef unsigned long long uint64;
#else
#    error "I don't seem to have a 64-bit integer type on this system."
#endif

// other Win32 definitions
//typedef uint16                UINT;
//typedef uint32                DWORD;

//-----------------------------------------------------------------------------
// Limits -- should be platform independent, right? ( assumes 2's complement numbers )
//-----------------------------------------------------------------------------


#if (!(USES_2S_COMPLEMENT))
#    error limits.h depends on 2s complement integers.  Check core/types.h
#endif

#include <limits.h> // defines limits for built-in types

#define TSS_INT8_MIN (-127 - 1)
#define TSS_INT8_MAX 127
#define TSS_UINT8_MAX 0xFFU
#define TSS_INT16_MIN (-32767 - 1)
#define TSS_INT16_MAX 32767
#define TSS_UINT16_MAX 0xFFFFU
#define TSS_INT32_MIN (-2147483647 - 1)
#define TSS_INT32_MAX 2147483647
#define TSS_UINT32_MAX 0xFFFFFFFFU
#define TSS_INT64_MIN (-9223372036854775807LL - 1)
#define TSS_INT64_MAX 9223372036854775807LL
#define TSS_UINT64_MAX 0xFFFFFFFFFFFFFFFFULL

/*
//-----------------------------------------------------------------------------
// numeric_limits : TODO:BAM eventually I should get around to finishing these....
//-----------------------------------------------------------------------------

#pragma warning( push )             
pragma warning( disable: 4663 )    // Use new template specialization syntax: template<>
#define NOMINMAX  // turn off min() and max() macros
#include <limits>
#pragma warning( pop )             

//
// define numeric_limits for our types
//
// NOTE: assumes std::numeric_limits< int > does not depend on signedness of type
// TODO:BAM -- add members like digits and digits10
// TODO:BAM -- define floating point numeric_limits
#define DECLARE_INTEGRAL_NUM_LIMITS( intT, minVal, maxVal, nDigits,  ) \
    template<> class numeric_limits< intT > : public std::numeric_limits< int > \
    {\
        static intT (min)() throw() { return ( minVal ); } \
        static intT (max)() throw() { return ( maxVal ); } \
        enum { digits = 0 };
        enum { digits = 0 };
    };

// NOTE: assumes ( std::numeric_limits< int >::is_specialized == true )
namespace std
{
#if IS_UNIX
    DECLARE_INTEGRAL_NUM_LIMITS( int8,  SCHAR_MIN, SCHAR_MAX );
    DECLARE_INTEGRAL_NUM_LIMITS( int16, SHRT_MIN, SHRT_MAX );
    DECLARE_INTEGRAL_NUM_LIMITS( int32, LONG_MIN, LONG_MAX );

#endif
}
*/

//-----------------------------------------------------------------------------
// Byte Swapping
//-----------------------------------------------------------------------------

inline int16 SWAPBYTES16(int16 i)
{

    return ((uint16)i >> 8) | ((uint16)i << 8);
}

inline int32 SWAPBYTES32(int32 i)
{
    return ((uint32)i >> 24) | (((uint32)i & 0x00ff0000) >> 8) | (((uint32)i & 0x0000ff00) << 8) | ((uint32)i << 24);
}

inline int64 SWAPBYTES64(int64 i)
{
    return ((uint64)i >> 56) | (((uint64)i & 0x00ff000000000000ULL) >> 40) |
           (((uint64)i & 0x0000ff0000000000ULL) >> 24) | (((uint64)i & 0x000000ff00000000ULL) >> 8) |
           (((uint64)i & 0x00000000ff000000ULL) << 8) | (((uint64)i & 0x0000000000ff0000ULL) << 24) |
           (((uint64)i & 0x000000000000ff00ULL) << 40) | ((uint64)i << 56);
}


#    ifdef WORDS_BIGENDIAN

#        define tw_htonl(x) (x)
#        define tw_ntohl(x) (x)
#        define tw_htons(x) (x)
#        define tw_ntohs(x) (x)
#        define tw_htonll(x) (x) // int64 versions
#        define tw_ntohll(x) (x)

#    else //!WORDS_BIGENDIAN

#        define tw_htonl(x) SWAPBYTES32((x))
#        define tw_ntohl(x) SWAPBYTES32((x))
#        define tw_htons(x) SWAPBYTES16((x))
#        define tw_ntohs(x) SWAPBYTES16((x))
#        define tw_htonll(x) SWAPBYTES64((x)) // int64 versions
#        define tw_ntohll(x) SWAPBYTES64((x))

#    endif //WORDS_BIGENDIAN

////////////////////////////////////////////

#    if __cplusplus >= 201103L
#        define TW_UNIQUE_PTR std::unique_ptr
#    else
#        define TW_UNIQUE_PTR std::auto_ptr
#    endif


#endif // __TYPES_H
