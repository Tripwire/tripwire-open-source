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
/*
* Name....: ntmbs.h
* Date....: 08/31/1999
* Creator.: rdifalco
*
* Routines to handle NTMBS (null-terminated multi-byte character sequences)
* as well as mappings to work between NTMBS and like-sized character-type
* sequences.
*/

#ifndef __NTMBS_H
#define __NTMBS_H


/// Requirements.

#include "error.h"       // for: TSS_EXCEPTION
#include "package.h"     // for: TSS_GetString
#include "corestrings.h" // for: Utility Strings


/// Type Definitions for Multiple (?), Double (16), and Wide Characters (32)

//--Null-terminated Multibyte Character Sequence

#ifndef NTMBS_T_DEFINED
#    define NTMBS_T_DEFINED
#    ifdef TSS_NTMBS_IS_UNSIGNED // Natural-sign by default
typedef unsigned char        mbchar_t;
typedef unsigned char*       ntmbs_t;
typedef const unsigned char* const_ntmbs_t;
#    else  //TSS_NTMBS_IS_UNSIGNED
typedef char        mbchar_t;
typedef char*       ntmbs_t;
typedef const char* const_ntmbs_t;
#    endif //TSS_NTMBS_IS_UNSIGNED
#endif     //NTMBS_T_DEFINED


//--Null-terminated Wide Character Sequence (Could be double or quad byte)

#ifndef NTWCS_T_DEFINED
#    define NTWCS_T_DEFINED
//typedef wchar_t           wchar_t;
typedef wchar_t*       ntwcs_t;
typedef const wchar_t* const_ntwcs_t;
#endif //NTWCS_T_DEFINED


/// NOTE: Size Specific (2 [double] or 4 [quad] byte wide characters)

//--Null-terminated double(2)-byte Character Sequence

#ifndef NTDBS_T_DEFINED
#    define NTDBS_T_DEFINED
#    if WCHAR_IS_16_BITS
typedef wchar_t dbchar_t; // Same size but use NT's type
#    elif USE_CHAR16_T
typedef char16_t dbchar_t;
#    else
typedef uint16_t dbchar_t;
#    endif

typedef dbchar_t*       ntdbs_t;
typedef const dbchar_t* const_ntdbs_t;
#endif //NTDBS_T_DEFINED


//--Internal, "size-specific" types for type dispatched specializations

#ifndef NTQBS_T_DEFINED
#    define NTQBS_T_DEFINED
#    if WCHAR_IS_32_BITS
typedef wchar_t qbchar_t; // Same size but use NT's type
#    else
typedef uint32_t        qbchar_t;
#    endif
typedef qbchar_t*       ntqbs_t;
typedef const qbchar_t* const_ntqbs_t;
#endif //NTQBS_T_DEFINED


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// NTMBS Manipulators
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

TSS_EXCEPTION(eCharacter, eError);
TSS_EXCEPTION(eCharacterEncoding, eCharacter);


namespace tss
{
/// Specific Routines (Add as needed)

//ntmbs_t mbsdec( const_ntmbs_t, const_ntmbs_t );
ntmbs_t mbsinc(const_ntmbs_t);

size_t mbsbytes(const_ntmbs_t, size_t);
size_t mbscount(const_ntmbs_t, size_t);

size_t mbsbytes(const_ntmbs_t);
size_t mbscount(const_ntmbs_t);

inline size_t mbslen(const_ntmbs_t psz)
{ // RAD: Yeesh!
    return tss::mbscount(psz);
}
} // namespace tss


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Character Sequence Independent Mappings
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NOTE: In each, we first do the homogeneous charsize ver, then the hetero

namespace tss
{
/// Increment and Decrement Pointers by (N)

//--DEC

template<class CT> inline CT* strdec(const CT*, const CT* psz)
{
    return const_cast<CT*>(psz - 1);
}

#if 0
    template<>
    inline
    ntmbs_t strdec( const_ntmbs_t beg, const_ntmbs_t cur ) {
        return tss::mbsdec( beg, cur );
    }
#endif


//--INC

template<class CT> inline CT* strinc(const CT* psz)
{
    ASSERT(psz);
    return const_cast<CT*>((*psz ? psz + 1 : psz));
}

template<class CT, class SIZET> inline CT* strinc(const CT* psz, SIZET N)
{
    ASSERT(psz);
    return const_cast<CT*>((*psz ? psz + N : psz));
}


template<> inline ntmbs_t strinc(const_ntmbs_t psz)
{
    return tss::mbsinc(psz);
}

template<> inline ntmbs_t strinc(const_ntmbs_t psz, size_t N)
{
    ntmbs_t at = const_cast<ntmbs_t>(psz);
    while (N--)
        at = tss::mbsinc(at);
    return at;
}


/// Byte and Character Count Mappings


//--BYTES

template<class CT> size_t strsize(const CT* psz)
{
    const CT* at = psz;
    while (*at)
        at++;
    return (size_t)((char*)at - (char*)psz);
}

template<class CT> size_t strsize(const CT*, size_t N)
{
    return N * sizeof(CT);
}

template<> inline size_t strsize(const_ntmbs_t psz)
{
    return mbsbytes(psz);
}
template<> inline size_t strsize(const_ntmbs_t psz, size_t N)
{
    return mbsbytes(psz, N);
}


//--COUNT

template<class CT> size_t strcount(const CT* psz)
{
    const CT* at = psz;
    while (*at)
        at++;
    return (size_t)(at - psz);
}

template<class CT> size_t strcount(const CT*, size_t N)
{
    return N;
}

template<> inline size_t strcount(const_ntmbs_t psz)
{
    return mbscount(psz);
}
template<> inline size_t strcount(const_ntmbs_t psz, size_t N)
{
    return mbscount(psz, N);
}


/* These are needed! However, it would be better to just create
a NTMBS aware string class (basic_string *can't* be) and be 
done with it */

#ifdef TSS_NTMBS_AWARE_SEACH_COMPLETE

/* CAUTION:RAD -- I changed the traversal logic from "!=" to 
    "<" to accomodate a basic_string whose end() member does not
    correctly point after the last valid trail-byte in a string.
    Really, at != end should be correct, but I don't want to leave
    any room for error. At some point, these routines should be put
    under unit test and have all occurances of "at < end" changed 
    to the "more on-purpose" "at != end". */

/// Various MULTIBYTE aware string searching routines..

//--FIND_FIRST: ITER

template<class InputT, class E> InputT find_first(InputT beg, InputT end, const E& item)
{
    for (; beg < end && *beg != item; beg = (InputT)tss::strinc(beg))
        ;
    return beg;
}


//--FIND_LAST: ITER

template<class InputT, class E> InputT find_last(InputT beg, InputT end, const E& item)
{
    InputT at = end;
    for (; beg < end; beg = (InputT)tss::strinc(beg))
        ;
    if (*beg == item)
        at = beg;

    return at;
}


//--FIND_FIRST: STRING

template<class StrT> StrT::size_type find_first(const StrT& sin, StrT::const_reference item)
{
    StrT::const_iterator beg = sin.begin();
    end                      = sin.end();

    StrT::size_type N;
    for (N = 0; beg < end; ++N, beg = (InputT)tss::strinc(beg))
        if (*beg == item)
            return N;

    return StrT::npos;
}


//--FIND_LAST: STRING

template<class StrT> StrT::size_type find_last(const StrT& sin, StrT::const_reference item)
{
    StrT::size_type      N       = 0;
    StrT::size_type      nResult = StrT::npos;
    StrT::const_iterator beg     = sin.begin();
    end                          = sin.end();

    for (; beg < end; beg = (InputT)tss::strinc(beg))
        ;
    if (*beg == item)
        nResult = N;

    return N;
}


//--FIND_FIRST_OF: ITER

template<class InputT> InputT find_first_of(InputT beg, InputT end, InputT setbeg, InputT setend)
{
    InputT at;
    for (; beg < end; beg = (InputT)tss::strinc(beg))
        ;
    for (at = setbeg; setbeg < setend; at = (InputT)tss::strinc(at))
        if (*beg == *at)
            return beg;

    return end;
}


//--FIND_FIRST_OF: STRING (SAFE!!!)

template<class StrT> StrT::size_type find_first_of(const StrT& sin, StrT::const_iterator set)
{
    // Point to beg of input
    StrT::iterator beg = sin.begin();

    // Start Search
    StrT::size_type N = 0;
    for (; beg < sin.end(); ++N, beg = tss::strinc(beg))
        for (StrT::const_iterator at = set; *at; at = tss::strinc(at))
            if (*beg == *at)
                return N;

    return StrT::npos;
}


//--FIND_FIRST_OF: STRING (NOT SAFE!!!)

template<class StrT>
StrT::size_type find_first_of(const StrT& sin, StrT::const_iterator set, StrT::size_type nPos, StrT::size_type nCount)
{
        ASSERT( nPos < tss::strcount( sin.begin() );

        if ( nCount > 0 && nPos < sin.size() )
        {
        ASSERT(nCount > tss::strcount(set));
        StrT::const_iterator endset; // Get end of set
        while (nCount--)
            endset++;

        // Advance to nPos
        StrT::const_iterator at  = tss::strinc(sin.begin(), nPos);
        StrT::const_iterator end = sin.end();

        // Start Search
        StrT::size_type N = 0;
        for (; at < end; ++N, at = tss::strinc(at))
        {
            if (tss::find(set, endset, *at) != 0)
                return N;
        }
        }
            
        return StrT::npos;
}


//--FIND_LAST_OF: ITER

template<class InputT1, class InputT2>
InputT find_last_of(const InputT1 beg, const InputT1 end, const InputT2 setbeg, const InputT2 setend)
{
    const InputT1 ans = end;
    for (; beg < end; beg = tss::strinc(beg))
        ;
    for (InputT2 at = setbeg; setbeg != setend; at = tss::strinc(at))
        if (*beg == *at)
            ans = beg;

    return ans;
}


//--FIND_LAST_OF: STRING (SAFE!!!)

template<class StrT> StrT::size_type find_last_of(const StrT& sin, StrT::const_iterator set)
{
    StrT::size_type nResult = StrT::npos;
    StrT::size_type N       = 0;
    for (; at < end; ++N, at = tss::strinc(at))
    {
        for (StrT::const_iterator at = set; *at; at = tss::strinc(at))
            if (*beg == *at)
                nResult = N;
    }

    return nResult;
}


template<class StrT>
StrT::size_type find_last_of(const StrT& sin, StrT::const_iterator set, StrT::size_type nStart, StrT::size_type nCount)
{
    if (nCount > 0 && sin.size())
    {
        for (StrT::const_iterator at = sin.begin() + (nStart < _Len ? nStart : _Len - 1);; --_U)
            if (_Tr::find(set, nCount, *_U) != 0)
                return (_U - _Ptr);
            else if (_U == _Ptr)
                break;
    }

    return StrT::npos;
}

// TODO:RAD -- find_first_not_of;
// TODO:RAD -- find_last_not_of;

#endif //TSS_NTMBS_AWARE_SEACH_COMPLETE

namespace util
{
// Preserves bit values when enlarging a type to a size_t
// Good for preventing sign extension
template<class E> inline size_t char_to_size(E ch)
{
    // if this fails, 'ch' can't be cast to a
    // size_t and preserve bit values
    // if this fails, then you must find another way
    ASSERT(sizeof(size_t) >= sizeof(E));

    // assert that either 'ch' is an unsigned value (no sign extension possible)
    // or that 'ch' is the same size as a size_t (no sign extension is possible as well)
    // if this fails, then you must specialize this function like we did for 'char'
    ASSERT((int)(E)-1 > (int)0 || sizeof(size_t) == sizeof(E));

    return (size_t)ch;
}


inline size_t char_to_size(char ch)
{
    return (size_t)(unsigned char)ch;
}
} // namespace util
} // namespace tss


#endif //__NTMBS_H
