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
* Name....: stringutil.h
* Date....: 09/03/99
* Creator.:
*
* Basic String Utilities
*/

#ifndef __STRINGUTIL_H
#define __STRINGUTIL_H

#ifndef __WCHAR16_H
#include "core/wchar16.h"
#endif

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

//-----------------------------------------------------------------------------
// cStringUtil
// NOTE:RAD -- This should not be a class, changed to namespace.
// contains utility methods for manipulating strings
//-----------------------------------------------------------------------------
namespace cStringUtil
{
//--Type Dispatched Conversion (Generalization away from TCHAR)

/// Handle Like-Types (xstring to xstring)

inline const char* Convert(std::string& lhs, const std::string& rhs)
{
    lhs = rhs;
    return lhs.c_str();
}

/// Specialize Everything Else

std::string::const_iterator Convert(std::string&, const wc16_string&);

wc16_string::const_iterator Convert(wc16_string&, const std::string&);

#if SUPPORTS_WCHART && WCHAR_REP_IS_UCS2

inline std::wstring::const_iterator Convert(std::wstring& lhs, const std::wstring& rhs)
{
    lhs = rhs;
    return lhs.c_str();
}

std::wstring::const_iterator Convert(std::wstring&, const wc16_string&);

std::wstring::const_iterator Convert(std::wstring&, const std::string&);

std::string::const_iterator Convert(std::string&, const std::wstring&);

wc16_string::const_iterator Convert(wc16_string&, const std::wstring&);

#endif //SUPPORTS_WCHART


/// WARNING: These are deprecated, please use Convert()!

std::string TstrToStr(const TSTRING& tstr);
// converts a tstring to a narrow string; useful for passphrase input, which must always be
// the same format, regardless of what TCHAR is defined to.

TSTRING StrToTstr(const std::string& str);
// converts a narrow string to a TSTRING

TSTRING WstrToTstr(const wc16_string& src);
// convert wc16_string to a TSTRING
wc16_string TstrToWstr(const TSTRING& tstr);
// convert a TSTRING to a wc16_string


template<class IterT> void splitstring(IterT& iter, const TSTRING& str, const TSTRING& delims)
{

    TSTRING::size_type where;
    TSTRING            tmpStr;
    TSTRING            workingString = str;

    while (workingString.length() > 0)
    {

        where = workingString.find_first_of(delims, 0);

        // in the case of no delimeters, setp to
        // grab the entire string.
        //
        if (where == TSTRING::npos)
            where = workingString.length();

        tmpStr = workingString.substr(0, where);
        workingString.erase(0, where + 1);

        iter.push_back(tmpStr);
    }
}


} // namespace cStringUtil

/*
//-----------------------------------------------------------------------------
// push_back_string
//
// push_back decorator for basic_string.
// allows basic_string to be used in, among other things, a back_insert_iterator
//-----------------------------------------------------------------------------
namespace std
{
    template< class E >
    class push_back_string 
        : public basic_string<E>
    {
        public:            
            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            // internal typedefs
            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            typedef push_back_string<E>     _MyT;
            typedef basic_string<E>         _ParentT;

            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            // required typedef ( to be a container it must have this... )
            //- - - - - - - - - - - - - - - - - - - - - - - - - -
           typedef typename _ParentT::value_type    value_type;

            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            // required constructors and assignment operators...
            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            push_back_string( const _ParentT& rhs ) 
                : _ParentT( rhs ) 
            {
            }

            push_back_string() 
                : _ParentT() 
            {
            }

            _MyT& operator=( const _MyT& rhs ) 
            { 
                return _ParentT::operator=( rhs );
            }

            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            // this is it!  
            // makes the string look like a container to a back_insert_iterator....            
            //- - - - - - - - - - - - - - - - - - - - - - - - - -
            _MyT& push_back( const typename _ParentT::value_type& v ) 
            {
                *this += v; return( *this ); 
            }
    };
}
*/

#endif //__STRINGUTIL_H
