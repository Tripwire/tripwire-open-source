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
// wchar16.h
//
// Function ans classes dealing with the WCHAR16 type
//

#ifndef __WCHAR16_H
#define __WCHAR16_H

#include "platform.h"

// TODO: Perhaps WCHAR16 should come out of types.h???
#ifndef __TYPES_H
#include "types.h"
#endif

#if WCHAR_IS_16_BITS
typedef wchar_t WCHAR16;
typedef std::wstring wc16_string;

#elif USE_U16STRING
typedef char16_t WCHAR16;
typedef std::u16string wc16_string;

#else
typedef uint16_t WCHAR16; // unix has 4 byte wchar_t, but we want to standardize on 16 bit wide chars

//=============================================================================
// class wc16_string
//
// This clas implements enough of the std::basic_string interface so we
// can use it like an STL string.
//
// In an ideal world we this would be a std::basic_string instantiation, but
// UNIX doesn't seem to dealing 16 bit wide chars, it uses 32 bit chars.
//
// a WCHAR16 is a 16-bit UCS2 (Unicode v1.1) character
//
//=============================================================================

class wc16_string_impl;

// TODO:BAM -- check into reducing wc16_string to:
// typedef std::basic_string<WCHAR16> wc16_string;

class wc16_string
{
public:
    typedef WCHAR16           value_type;
    typedef value_type*       iterator;
    typedef const value_type* const_iterator;
    typedef size_t            size_type;


    wc16_string();
    wc16_string(const wc16_string& rhs);
    wc16_string(const_iterator pStr);

    ~wc16_string();

    wc16_string& operator=(const wc16_string& rhs);
    int  compare(const wc16_string& rhs) const;

    size_type length() const;
    size_type size() const;

    const_iterator begin() const;
    iterator       begin();

    const_iterator end() const;
    iterator       end();

    const_iterator c_str() const;
    const_iterator data() const;

    bool empty() const
    {
        return length() == 0;
    }

    const value_type& operator[](int i) const; // throw std::out_of_range
    value_type&       operator[](int i);       // throw std::out_of_range

    void resize(size_type);
    void swapbytes(); // useful to convert to network byte order

private:
    wc16_string_impl* mpData;
};
#endif

namespace tss
{
    void swapbytes(wc16_string& str);
}

#ifndef WORDS_BIGENDIAN
#define TSS_SwapBytes(x) tss::swapbytes(x)
#else
#define TSS_SwapBytes(x)
#endif

#endif // __WCHAR16_H
