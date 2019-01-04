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
// stringencoder.h
//

#ifndef __STRINGENCODER_H
#define __STRINGENCODER_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

////////////////////////////////////////////////////////////////////////
// class cCharEncoder
//
// encodes "chEncodeMe" to "chEncoded1,chEncoded2"
// encodes "chEncoded1" to "chEncoded1,chEncoded1"
// chEncoded2 will need no encoding
//
class cCharEncoder
{
public:
    cCharEncoder(TCHAR chEncodeMe, TCHAR chEncoded1, TCHAR chEncoded2);

    void  EncodeChar(TSTRING::const_iterator& i, TSTRING& strToAppendTo);
    TCHAR DecodeChar(TSTRING::const_iterator& i, TSTRING::const_iterator& end);
    TCHAR GetEncodeChar();

private:
    cCharEncoder();
    cCharEncoder(const cCharEncoder&);
    cCharEncoder& operator=(const cCharEncoder&);

    TCHAR mchEncodeMe;
    TCHAR mchEncoded1;
    TCHAR mchEncoded2;
};

////////////////////////////////////////////////////////////////////////
// class cStringEncoder
//
class cStringEncoder
{
public:
    cStringEncoder(TCHAR chEncodeMe, TCHAR chEncoded1, TCHAR chEncoded2);

    TSTRING& Encode(TSTRING& inAndOut);
    TSTRING& Encode(const TSTRING& in, TSTRING& out);
    // returns reference to out

    TSTRING& Unencode(TSTRING& inAndOut);
    TSTRING& Unencode(const TSTRING& in, TSTRING& out);
    // returns reference to out

    TCHAR GetEncodeChar();

private:
    cStringEncoder();
    cStringEncoder(const cStringEncoder&);
    cStringEncoder& operator=(const cStringEncoder&);

    cCharEncoder ce;
};

////////////////////////////////////////////////////////////////////////
// class cQuoteEncoder
//
// convenience class that converts <"> -> <\"> and <\> -> <\\>
//
class cQuoteEncoder : public cStringEncoder
{
public:
    cQuoteEncoder();

private:
    cQuoteEncoder(const cQuoteEncoder&);
    cQuoteEncoder& operator=(const cQuoteEncoder&);
};


//=========================================================================
// INLINE FUNCTIONS
//=========================================================================


inline cCharEncoder::cCharEncoder(TCHAR chEncodeMe, TCHAR chEncoded1, TCHAR chEncoded2)
    : mchEncodeMe(chEncodeMe), mchEncoded1(chEncoded1), mchEncoded2(chEncoded2)
{
}

inline TCHAR cCharEncoder::GetEncodeChar()
{
    return mchEncodeMe;
}

inline cStringEncoder::cStringEncoder(TCHAR chEncodeMe, TCHAR chEncoded1, TCHAR chEncoded2)
    : ce(chEncodeMe, chEncoded1, chEncoded2)
{
}

inline TCHAR cStringEncoder::GetEncodeChar()
{
    return ce.GetEncodeChar();
}

inline cQuoteEncoder::cQuoteEncoder() : cStringEncoder(_T('\"'), _T('\\'), _T('\"'))
{
}

#endif //__STRINGENCODER_H
