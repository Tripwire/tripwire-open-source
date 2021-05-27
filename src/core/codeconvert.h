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
//
// Name....: codeconvert.h
// Date....: 9/2/99
// Creator.: Brian McFeely (bmcfeely)
//
// CodePage Conversion between Character Sequences
//
// Notes on Terminology
// --------------------
//  ntbs_t   -- A null-terminated single byte sequence (pointer)
//  char     -- A single character
//
//  ntmbs_t  -- A null-terminated multibyte sequence (pointer)
//  mbchar_t -- A single lead or trail byte
//
//  ntdbs_t  -- A null-terminated two-byte character sequence (pointer)
//  dbchar_t -- A single double-byte character
//
//  ntwcs_t  -- null-terminated two or four-byte wide character sequence (pointer)
//  wchar_t  -- A single wide (double or quad) character
//
// This module happens to use only multibyte (ntmbs) and double-byte only
// (ntwcs). None of these routines should be used with a wide character
// of unknown size, such as wchar_t, or a UCS4 or quad-byte (ntqbs) pointer.
//

#ifndef __CODECONVERT_H
#define __CODECONVERT_H

/// Requirements

#include "platform.h" // for: Platform specific code
#include "ntmbs.h"    // for: NTBS, NTMBS, NTWCS, and NTDBS types


/// Exceptions

TSS_EXCEPTION(eConverter, eError);
TSS_EXCEPTION(eConverterReset, eConverter);
TSS_EXCEPTION(eConverterFatal, eConverter);
TSS_EXCEPTION(eConverterUnsupportedConversion, eConverter);
TSS_EXCEPTION(eConverterUnknownCodepage, eConverter);


/// Classes

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* iCodeConverter -- The CodePage Converter Interface
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* SYNOPSIS:  Converts between the local code page and UCS2 (if possible)
*            Ensures a one-to-one correspondance between mb and UCS2 by
*            making sure the mapping is roundtrip. If it is not roundtrip,
*            or a byte value is not a character, it is cast to a reserved
*            region of UCS2 ( 0xE000 - 0xE0FF ).
* CONSTRAINTS:
*   
* INVARIANTS:
*   
*/
class iCodeConverter
{
public:
    static iCodeConverter* GetInstance(); // Singleton
    static void            Finit();


    /// Subclass Responsibilities
    virtual int Convert(ntmbs_t,            // NTMBS buffer
                        size_t,             // Capacity in mbchar_t's (bytes)
                        const_ntdbs_t,      // null terminated two-byte wide character (UCS2 rep)
                        size_t nCount) = 0; // Amount to convert in dbchar_t's
    // returns number of buffer items converted, -1 on error


    virtual int Convert(ntdbs_t,       // NTDBS (Null-terminated two byte sequence) buf
                        size_t,        // Capacity in dbchar_t's
                        const_ntmbs_t, // Null-terminated multi-byte sequence
                        size_t) = 0;   // Capacity in mbchar_t's (bytes)
    // returns number of buffer items converted, -1 on error

protected:
    iCodeConverter()
    {
    }
    virtual ~iCodeConverter()
    {
    }

private:
    static iCodeConverter* CreateConverter();
    static iCodeConverter* CreateGoodEnoughConverter();

    static iCodeConverter* m_pInst;
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cIconvConverter -- Concrete Converter
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HAVE_ICONV_H
#include <iconv.h>

#    ifdef HAVE_LANGINFO_H
#        ifndef __USE_XOPEN
#            define __USE_XOPEN 1
#        endif
#        include <langinfo.h>
#    endif

class cIconvUtil
{
public:
    static const char* GetCodePageID(); // gets code page id for current locale, throws if error
    static bool        GetCodePageID(const char** ppCP);
    static const char* GetIconvDbIdentifier();
    static const char* GetMiddleIdentifier();
    static void        ResetConverter(iconv_t);
    static bool        TestConverter(const char* pTo, const char* pFrom);
    static iconv_t     OpenHandle(const char* pTo, const char* pFrom); // throws
    static void        CloseHandle(iconv_t ic);
};


class cIconvConverter : public iCodeConverter
{
public:
    static bool Test(); // is there a conversion for the current codepage?

    virtual int Convert(ntmbs_t, size_t, const_ntdbs_t, size_t);
    virtual int Convert(ntdbs_t, size_t, const_ntmbs_t, size_t);

    cIconvConverter();
    virtual ~cIconvConverter();

private:
    void Init();

    iconv_t icToDb;
    iconv_t icFromDb;
};

class cDoubleIconvConverter : public iCodeConverter
{
public:
    static bool Test(); // is there a conversion for the current codepage?

    virtual int Convert(ntmbs_t, size_t, const_ntdbs_t, size_t);
    virtual int Convert(ntdbs_t, size_t, const_ntmbs_t, size_t);

    cDoubleIconvConverter();
    virtual ~cDoubleIconvConverter();

private:
    void Init();

    iconv_t icMbToUTF8;
    iconv_t icUTF8ToDb;

    iconv_t icDbToUTF8;
    iconv_t icUTF8ToMb;
};

#endif // HAVE_ICONV_H


/// QUESTION:RAD -- Why not just have one platform-specific of the below

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cWcharIs32BitUcs2Converterer
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if WCHAR_IS_32_BITS

// TODO:BAM -- this is not really correct!  Convert will not honor nSource!!
// it looks for the first null char!

class cWcharIs32BitUcs2Converterer : public iCodeConverter
{
public:
    virtual int Convert(ntmbs_t, size_t, const_ntdbs_t, size_t);
    virtual int Convert(ntdbs_t, size_t, const_ntmbs_t, size_t);

    virtual ~cWcharIs32BitUcs2Converterer()
    {
    }
};

#endif // WCHAR_IS_32_BITS

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cWcharIs16BitUcs2Converterer
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if WCHAR_IS_16_BITS

// TODO:BAM -- this is not really correct!  Convert will not honor nSource!!
// it looks for the first null char!

class cWcharIs16BitUcs2Converterer : public iCodeConverter
{
public:
    virtual int Convert(ntmbs_t, size_t, const_ntdbs_t, size_t);
    virtual int Convert(ntdbs_t, size_t, const_ntmbs_t, size_t);

    virtual ~cWcharIs16BitUcs2Converterer()
    {
    }
};

#endif // WCHAR_IS_16_BITS

// does a straight cast of each mbchar_t byte to a dbchar_t and back.
// converts mbchars > 0x7F to our reserved space
class cGoodEnoughConverterer : public iCodeConverter
{
public:
    virtual int Convert(ntmbs_t, size_t, const_ntdbs_t, size_t);
    virtual int Convert(ntdbs_t, size_t, const_ntmbs_t, size_t);

    virtual ~cGoodEnoughConverterer()
    {
    }
};


class cConvertUtil
{
    enum
    {
        TSS_UCS2_RESERVED_START = 0xE800u, // E000-E8FF is private use range, but
        TSS_UCS2_RESERVED_END   = 0xE8FFu, // SJIS and EUC use E000-E757
        TSS_HIGH_ASCII_START    = 0x0080u,
        TSS_HIGH_ASCII_END      = 0x00FFu
    };

public:
    static dbchar_t ConvertNonChar(mbchar_t ch);
    static mbchar_t ConvertNonChar(dbchar_t ch);

    static bool ValueInReservedRange(mbchar_t ch);
    static bool ValueInReservedRange(dbchar_t ch);
};


#endif //__CODECONVERT_H
