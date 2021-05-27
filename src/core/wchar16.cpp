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
// Function and classes dealing with the WCHAR16 type
//
#include "stdcore.h"
#include "config.h"
#include "wchar16.h"
#include "debug.h"
#include <stdexcept> // for stl::out_of_range
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

int wchar16len(const WCHAR16* s)
{
    int i;
    for (i = 0; *s != 0; ++s, ++i)
        ;

    return i;
}

#if NEED_DBSTRING_IMPL
//=============================================================================
// class wc16_string
//
// This clas implements enough of the std::basic_string interface so we
// can use it like an STL string.
//
// In an ideal world we this would be a std::basic_string instantiation, but
// UNIX doesn't seem to dealing 16 bit wide chars, it uses 32 bit chars.
//=============================================================================

class wc16_string_impl
{
public:
    int      length;
    WCHAR16* pString;
    int      refCount;

    wc16_string_impl();
    wc16_string_impl(const wc16_string_impl& rhs);

    void AddRef();
    void Release();
    void Resize(int newlen);
    void CopyString(const wc16_string_impl& rhs);
    // note: does not alter this->referenceCount, and ASSERTs that refcount == 1

    ~wc16_string_impl(); // call Release() to delete
private:
    void operator=(const wc16_string_impl& rhs)
    {
        return;
    } // don't call
};

static WCHAR16 NULL_WCHAR16 = 0;

wc16_string::wc16_string() : mpData(0)
{
}

wc16_string::wc16_string(const wc16_string& rhs)
{
    mpData = rhs.mpData;
    if (mpData)
        mpData->AddRef();
}

wc16_string::wc16_string(const_iterator pStr)
{
    mpData = new wc16_string_impl;

    int len = wchar16len(pStr);
    mpData->Resize(len);
    memcpy(mpData->pString, pStr, len * sizeof(WCHAR16));
}

wc16_string::~wc16_string()
{
    if (mpData)
        mpData->Release();
}

wc16_string& wc16_string::operator=(const wc16_string& rhs)
{
    if (this != &rhs)
    {
        if (mpData)
            mpData->Release();

        mpData = rhs.mpData;

        if (mpData)
            mpData->AddRef();
    }
    return *this;
}


int wc16_string::compare(const wc16_string& rhs) const
{
    if (length() > rhs.length())
        return 1;
    else if (length() < rhs.length())
        return -1;
    else
        return memcmp(c_str(), rhs.c_str(), length() * sizeof(WCHAR16));
}

wc16_string::size_type wc16_string::length() const
{
    return mpData ? mpData->length : 0;
}

wc16_string::size_type wc16_string::size() const
{
    return mpData ? mpData->length : 0;
}

wc16_string::const_iterator wc16_string::c_str() const
{
    if (mpData == 0)
        return &NULL_WCHAR16;

    mpData->pString[mpData->length] = 0;

    return mpData->pString;
}

wc16_string::const_iterator wc16_string::begin() const
{
    return (const_cast<wc16_string*>(this))->begin();
}

wc16_string::iterator wc16_string::begin()
{
    return mpData ? mpData->pString : &NULL_WCHAR16;
}

wc16_string::const_iterator wc16_string::end() const
{
    return const_cast<iterator>(mpData ? mpData->pString + mpData->length : &NULL_WCHAR16);
}

wc16_string::iterator wc16_string::end()
{
    return mpData ? mpData->pString + mpData->length : &NULL_WCHAR16;
}

wc16_string::const_iterator wc16_string::data() const
{
    return begin();
}

const wc16_string::value_type& wc16_string::operator[](int i) const
{
    ASSERT(i >= 0);

    if (mpData == 0 || i >= mpData->length || i < 0)
    {
        ASSERT(false);
        throw std::out_of_range("wc16_string[]");
    }

    return mpData->pString[i];
}

wc16_string::value_type& wc16_string::operator[](int i)
{
    ASSERT(i >= 0);

    if (mpData == 0 || i >= mpData->length || i < 0)
    {
        ASSERT(false);
        throw std::out_of_range("wc16_string[]");
    }

    if (mpData->refCount > 1)
    {
        wc16_string_impl* newImpl = new wc16_string_impl(*mpData);
        mpData->Release();
        mpData = newImpl;
    }

    return mpData->pString[i];
}

void wc16_string::resize(size_type nCount)
{
    if (mpData == 0)
    {
        mpData = new wc16_string_impl;
    }
    else if (mpData->refCount > 1)
    {
        wc16_string_impl* newImpl = new wc16_string_impl(*mpData);
        mpData->Release();
        mpData = newImpl;
    }

    mpData->Resize(nCount);
}

// useful to convert to network byte order
void wc16_string::swapbytes()
{
    if (mpData)
    {
        if (mpData->refCount > 1)
        {
            wc16_string_impl* newImpl = new wc16_string_impl(*mpData);
            mpData->Release();
            mpData = newImpl;
        }

        int i;
        for (i = 0; i < mpData->length; ++i)
        {
            mpData->pString[i] = SWAPBYTES16(mpData->pString[i]);
        }
    }
}

wc16_string_impl::wc16_string_impl()
{
    length   = 0;
    pString  = new WCHAR16[1];
    refCount = 1;
}

wc16_string_impl::wc16_string_impl(const wc16_string_impl& rhs)
{
    int newlen = rhs.length;
    pString    = new WCHAR16[newlen + 1];
    length     = newlen;
    refCount   = 1;

    memcpy(pString, rhs.pString, newlen * sizeof(WCHAR16));
}

wc16_string_impl::~wc16_string_impl()
{
    delete [] pString;
}

void wc16_string_impl::AddRef()
{
    ++refCount;
}

void wc16_string_impl::Release()
{
    ASSERT(refCount > 0);

    if (--refCount == 0)
    {
        delete this;
    }
}

void wc16_string_impl::Resize(int newlen)
{
    ASSERT(newlen >= 0);
    if (newlen == length)
        return;

    WCHAR16* newString = new WCHAR16[newlen + 1];

    if (newlen < length)
    {
        memcpy(newString, pString, newlen * sizeof(WCHAR16));
        delete [] pString;
        pString = newString;
        length  = newlen;
    }
    else
    {
        memcpy(newString, pString, length * sizeof(WCHAR16));
        memset(newString + length, 0, (newlen - length) * sizeof(WCHAR16));
        delete [] pString;
        pString = newString;
        length  = newlen;
    }
}

void wc16_string_impl::CopyString(const wc16_string_impl& rhs)
{
    ASSERT(refCount == 1);

    int newlen = rhs.length;

    if (rhs.length != this->length)
    {
        delete [] pString;
        pString      = new WCHAR16[newlen + 1];
        this->length = newlen;
    }

    memcpy(this->pString, rhs.pString, newlen * sizeof(WCHAR16));
}
#endif

namespace tss
{
void swapbytes(wc16_string& str)
{
#if NEED_DBSTRING_IMPL
    str.swapbytes();
#else
    size_t len = str.length();
    for (size_t x=0; x < len; x++)
    {
        WCHAR16 current = str[x];
        str[x] = ((current >> 8) | ((current & 0xFF) << 8));
    }
#endif
}
}

