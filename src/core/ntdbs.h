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
* Name....: ntdbs.h
* Date....: 09/16/99
* Creator.: rdifalco
*
* Allows dbchar_t (aka: WCHAR16) to be used with basic_string.
*
* std::basic_string<dbchar_t> (aka: tss::dbstring) should be 
* used instead of wc16_string!
*
* NOTE: If platform's wchar_t is already a dbchar_t (i.e. 2 bytes)
*       then tss::dbstring and dbchar_t are synonyms for std::wstring
*       and wchar_t.
*/

#ifndef __NTDBS_H
#define __NTDBS_H

#include "stringutil.h"
#include "ntmbs.h"

#if IS_KAI
#    define IS_STDLIB_MODENA 1 // Has special char_traits req's!
#    ifdef MSIPL_WCHART
#        define MBSTATE_T_DEFINED
#    endif
#else
#    define IS_STDLIB_MODENA 0
#    define MBSTATE_T_DEFINED
#endif


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Char traits for WCHAR16 (aka dbchar_t) and NTMBCS (mctype_t)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if (WCHAR_IS_16_BITS)
namespace tss
{
typedef std::wstring dbstring;
}

#elif (USE_U16STRING)
namespace tss
{
typedef std::u16string dbstring;
}

#elif (WCHAR_IS_32_BITS)
#if HAVE_LOCALE
namespace std
{
template<> struct char_traits<dbchar_t>;
}
#endif
namespace tss
{
typedef std::basic_string<dbchar_t> dbstring;
}

#else
#    error "wchar_t is not correctly factored!"

#endif


#if NEED_DBSTRING_IMPL // We already have a dbstring implicitly in wstring!!!

#if HAVE_LOCALE
#   include <locale>

// specialize *std*::char_traits!!!

template<> struct std::char_traits<dbchar_t>
{
    typedef dbchar_t  char_type;
    typedef wint_t    int_type;
    typedef streampos pos_type;
    typedef streamoff off_type;

#    ifdef MBSTATE_T_DEFINED
    typedef mbstate_t state_type;
#    else
    typedef int state_type; // Shame on you KAI! This is ANSI-C!
#    endif

    static void assign(char_type& lhs, const char_type& rhs)
    {
        lhs = rhs;
    }

    static bool eq(const char_type& lhs, const char_type& rhs)
    {
        return lhs == rhs;
    }

    static bool lt(const char_type& lhs, const char_type& rhs)
    {
        return lhs < rhs;
    }

    static int compare(const char_type* lhs, const char_type* rhs, size_t N)
    {
        for (; N; ++lhs, ++rhs, --N)
            if (*lhs != *rhs)
                return (*lhs < *rhs) ? -1 : +1;

        return 0;
    }

    static size_t length(const char_type* psz)
    {
        if (psz == 0)
            return 0;
        else
        {
            const char_type* end;
            for (end = psz; *end; end++)
                ;
            return (size_t)(end - psz);
        }
    }

    static char_type* copy(char_type* lhs, const char_type* rhs, size_t N)
    {
        for (char_type* at = lhs; N; ++at, ++rhs, --N)
            *at = *rhs;

        return lhs;
    }

    static const char_type* find(const char_type* psz, size_t N, const char_type& ch)
    {
        for (; N; ++psz, --N)
            if (*psz == ch)
                return psz;

        return 0;
    }

    static char_type* move(char_type* lhs, const char_type* rhs, size_t N)
    {
        return (char_type*)memmove(lhs, rhs, N * sizeof(char_type));
    }

    static char_type* assign(char_type* lhs, size_t N, char_type ch)
    {
        for (char_type* at = lhs; N; ++at, --N)
            *at = ch;
        return lhs;
    }

    static int_type not_eof(const int_type& ch)
    {
        return int_type(!eq(ch, eof()) ? ch : ~ch);
    }

    static char_type to_char_type(const int_type& ch)
    {
        return eq_int_type(ch, to_int_type(ch)) ? ch : char_type(0);
    }

    static int_type to_int_type(const char_type& ch)
    {
        return (int_type)ch;
    }

    static bool eq_int_type(const int_type& lhs, const int_type& rhs)
    {
        return lhs == rhs;
    }

#if IS_STDLIB_MODENA
    // CAUTION:RAD -- Extra members required by Modena!!
#ifdef MBSTATE_T_DEFINED // This is ANSI-C *not* ANSI-C++!!
    static state_type get_state(pos_type pos)
    {
        return pos.state();
    }

    static pos_type get_pos(pos_type pos, state_type state)
    {
        return pos_type(pos.offset(), state);
    }
#endif //MBSTATE_T_DEFINED

    static char_type newline()
    {
        return L'\n';
    }

    static char_type eos()
    {
        return 0;
    }
#endif //IS_STDLIB_MODENA

    static int_type eof()
    {
        return (wint_t)(0xFFFF);
    }
};
#endif

#    endif //WCHAR_IS_16_BITS // We already have a dbstring implicitly in wstring!!!


#endif //__NTDBS_H
