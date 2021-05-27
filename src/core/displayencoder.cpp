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
// displayencoder.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdcore.h"
#include "displayencoder.h"
#include "charutil.h"
#include "debug.h"
#include "twlocale.h"
#include "stringutil.h"
#include "errorutil.h"
#include "ntmbs.h"
#include "codeconvert.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <iomanip>
#include <iterator>

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

// uncomment this to test schema
// #define TSS_DO_SCHEMA_VALIDATION

//////////////////////////////////////////////////////////////////////////////
// ENCODER UTILITIES
//////////////////////////////////////////////////////////////////////////////


inline bool IsSingleTCHAR(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (first + 1 == last);
}

//////////////////////////////////////////////////////////////////////////////
// CHAR ENCODER INTERFACE
//////////////////////////////////////////////////////////////////////////////

// all derived classes should encode a char to "EscapeChar() + Identifier() + Encode( char ) [ + Identifier() ]"

class iCharEncoder
{
public:
    virtual ~iCharEncoder(){};

    virtual bool NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const = 0;
    // Determines if character identified by [first,last) needs encoding.
    // Returns true if it does.

    virtual TSTRING EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const = 0;
    // Encodes character identified by [first,last) in such a way that it
    // can be decoded by Decode().  Returns encoded character sequence.

    virtual TSTRING EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const = 0;
    // Encodes character identified by [first,last) in a manner that is not roundtrip,
    // but looks good.  Returns encoded character sequence.

    virtual TSTRING Decode(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end) const = 0;
    // Decodes character sequence beginning with '*pcur' and ending before 'end'.
    // Returns decoded character or sequence of characters.  Advances *pcur beyond
    // the last character decoded.


    virtual TCHAR Identifier() const = 0;

    static TCHAR EscapeChar()
    {
        return char_escape;
    }

protected:
    static TCHAR char_escape;
};


class cNonNarrowableCharEncoder : public iCharEncoder
{
public:
    virtual ~cNonNarrowableCharEncoder()
    {
    }

    virtual bool NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING Decode(TSTRING::const_iterator* cur, const TSTRING::const_iterator end) const;

    virtual TCHAR Identifier() const;

private:
    static TCHAR char_identifier;
    static TCHAR char_replace;
};


class cNonPrintableCharEncoder : public iCharEncoder
{
public:
    cNonPrintableCharEncoder(bool f_allowWS) : m_allowWS(f_allowWS){};

    virtual ~cNonPrintableCharEncoder()
    {
    }

    virtual bool NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING Decode(TSTRING::const_iterator* cur, const TSTRING::const_iterator end) const;

    virtual TCHAR Identifier() const;

private:
    static TCHAR char_identifier;
    static TCHAR char_replace;

    bool m_allowWS;
};

class cQuoteCharEncoder : public iCharEncoder
{
public:
    virtual ~cQuoteCharEncoder()
    {
    }

    virtual bool NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING Decode(TSTRING::const_iterator* cur, const TSTRING::const_iterator end) const;

    virtual TCHAR Identifier() const;

private:
    static TCHAR char_test;
    static TCHAR char_identifier;
    static TCHAR char_replace;
};


class cBackslashCharEncoder : public iCharEncoder
{
public:
    virtual ~cBackslashCharEncoder()
    {
    }

    virtual bool NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const;

    virtual TSTRING Decode(TSTRING::const_iterator* cur, const TSTRING::const_iterator end) const;

    virtual TCHAR Identifier() const;

private:
    static TCHAR char_test;
    static TCHAR char_identifier;
    static TCHAR char_replace;
};


//////////////////////////////////////////////////////////////////////////////
// CHARACTER SPECIALIZATIONS
//////////////////////////////////////////////////////////////////////////////

TCHAR iCharEncoder::char_escape = _T('\\');

TCHAR cNonNarrowableCharEncoder::char_identifier = _T('x');
TCHAR cNonPrintableCharEncoder::char_identifier  = _T('x');
TCHAR cQuoteCharEncoder::char_identifier         = _T('\"');
TCHAR cBackslashCharEncoder::char_identifier     = _T('\\');

TCHAR cBackslashCharEncoder::char_test = cBackslashCharEncoder::char_identifier;
TCHAR cQuoteCharEncoder::char_test     = cQuoteCharEncoder::char_identifier;

TCHAR cBackslashCharEncoder::char_replace = cBackslashCharEncoder::char_identifier;
TCHAR cQuoteCharEncoder::char_replace     = cQuoteCharEncoder::char_identifier;

TCHAR cNonNarrowableCharEncoder::char_replace = _T('?');
TCHAR cNonPrintableCharEncoder::char_replace  = _T('?');

//////////////////////////////////////////////////////////////////////////////
// TESTS
//////////////////////////////////////////////////////////////////////////////

bool cNonNarrowableCharEncoder::NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    return false; // all chars are narrow
}


bool cNonPrintableCharEncoder::NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    // TODO:BAM -- handle this with mb chars
    // std::isprint<wchar_t> does a wctob() on the wchar!!?!?!
    // what's up with that?  Maybe ignore this all together and
    // just do a isprint like KAI does?

    // HYPOTHESIS: all mb characters are printable.  only sb ASCII
    // chars that would have C isprint() return false actually aren't printable
    // So escape chars, and tabs and such are only in sb chars that C isprint() would check.
    // HMMMM: true in all locales, though? (LC_CTYPE is checked for C isprint(), though...)

    // Sooooo... it should be something like
    //
    //  #ifdef _UNICODE
    //      char nch = wctob( ch );
    //      return( nch != EOF && ! isprint( nch ) );
    //  #else
    //      return( ! isprint( ch ) );
    //  #endif
    //

    // assuming all unprintable chars are one TCHAR long
    if (!IsSingleTCHAR(first, last))
        return false;

    if (m_allowWS && cCharEncoderUtil::IsWhiteSpace(*first))
        return false;

    return cCharEncoderUtil::IsPrintable(*first);
}

bool cQuoteCharEncoder::NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    return (IsSingleTCHAR(first, last) && (*first == char_test));
}

bool cBackslashCharEncoder::NeedsEncoding(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    return (IsSingleTCHAR(first, last) && (*first == char_test));
}

//////////////////////////////////////////////////////////////////////////////
// ROUNDTRIP ENCODINGS
//////////////////////////////////////////////////////////////////////////////


TSTRING cNonNarrowableCharEncoder::EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    TSTRING str;

    str += char_escape;
    str += char_identifier;
    str += cCharEncoderUtil::CharStringToHexValue(TSTRING(first, last));

    return str;
}


TSTRING cNonPrintableCharEncoder::EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    ASSERT(IsSingleTCHAR(first, last)); // non-prints are single char (see NOTE above)

    TSTRING str;

    str += char_escape;
    str += char_identifier;
    str += cCharEncoderUtil::CharStringToHexValue(TSTRING(first, last));

    return str;
}


TSTRING cQuoteCharEncoder::EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    // should just be a quote
    ASSERT(IsSingleTCHAR(first, last));
    ASSERT(*first == char_test);

    TSTRING str;

    str += char_escape;
    str += char_identifier;

    return str;
}


TSTRING cBackslashCharEncoder::EncodeRoundtrip(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    // should just be a backslash
    ASSERT(IsSingleTCHAR(first, last));
    ASSERT(*first == char_test);

    TSTRING str;

    str += char_escape;
    str += char_identifier;

    return str;
}

//////////////////////////////////////////////////////////////////////////////
// NON-ROUNDTRIP ENCODINGS
//////////////////////////////////////////////////////////////////////////////


TSTRING cNonNarrowableCharEncoder::EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    return EncodeRoundtrip(first, last);
}


TSTRING cNonPrintableCharEncoder::EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    return EncodeRoundtrip(first, last);
}


TSTRING cQuoteCharEncoder::EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    // should just be a quote
    ASSERT(IsSingleTCHAR(first, last));
    ASSERT(*first == char_test);

    return TSTRING(1, char_replace);
}


TSTRING cBackslashCharEncoder::EncodePretty(TSTRING::const_iterator first, TSTRING::const_iterator last) const
{
    // should just be a backslash
    ASSERT(IsSingleTCHAR(first, last));
    ASSERT(*first == char_test);

    return TSTRING(1, char_replace);
}

//////////////////////////////////////////////////////////////////////////////
// DECODINGS
//////////////////////////////////////////////////////////////////////////////


TSTRING cNonNarrowableCharEncoder::Decode(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end) const
{
    // check preconditions
    if ((*pcur) >= end || *(*pcur) != Identifier())
        ThrowAndAssert(eBadDecoderInput());

    return (cCharEncoderUtil::DecodeHexToChar(pcur, end));
}


TSTRING cNonPrintableCharEncoder::Decode(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end) const
{
    // check preconditions
    if ((*pcur) >= end || *(*pcur) != Identifier())
        ThrowAndAssert(eBadDecoderInput());

    return (cCharEncoderUtil::DecodeHexToChar(pcur, end));
}


TSTRING cQuoteCharEncoder::Decode(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end) const
{
    if ((*pcur) >= end || *(*pcur) != Identifier())
        ThrowAndAssert(eBadDecoderInput());

    (*pcur)++; // advance past part decoded

    return TSTRING(1, Identifier());
}


TSTRING cBackslashCharEncoder::Decode(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end) const
{
    if ((*pcur) >= end || *(*pcur) != Identifier())
        ThrowAndAssert(eBadDecoderInput());

    (*pcur)++; // advance past part decoded

    return TSTRING(1, Identifier());
}

//////////////////////////////////////////////////////////////////////////////
// IDENTIFIERS
//////////////////////////////////////////////////////////////////////////////


TCHAR cNonNarrowableCharEncoder::Identifier() const
{
    return char_identifier;
}


TCHAR cNonPrintableCharEncoder::Identifier() const
{
    return char_identifier;
}


TCHAR cQuoteCharEncoder::Identifier() const
{
    return char_identifier;
}

TCHAR cBackslashCharEncoder::Identifier() const
{
    return char_identifier;
}

//////////////////////////////////////////////////////////////////////////////
// UTILITIES
//////////////////////////////////////////////////////////////////////////////

bool cCharEncoderUtil::IsWhiteSpace(TCHAR ch)
{
    return (ch == '\r' || ch == '\n' || ch == '\t' || ch == '\v' || ch == ' ');
}

bool cCharEncoderUtil::IsPrintable(TCHAR ch)
{
#if USE_CLIB_LOCALE && !defined(__APPLE__)

    return (!isprint(ch)); // kludge for KAI

#else // USE_CLIB_LOCALE

    return (!std::isprint<TCHAR>(ch, std::locale()));

#endif // USE_CLIB_LOCALE
}

TSTRING cCharEncoderUtil::CharStringToHexValue(const TSTRING& str)
{
    TSTRING                 strOut;
    TSTRING::const_iterator at;

    for (at = str.begin(); at < str.end(); at++)
    {
        strOut += char_to_hex(*at); 
    }

    return strOut;
}


TSTRING cCharEncoderUtil::HexValueToCharString(const TSTRING& str)
{
    TSTRING                 strOut;
    TSTRING::const_iterator at;

    for (at = str.begin(); at < str.end(); at += TCHAR_AS_HEX__IN_TCHARS)
    {
        strOut += hex_to_char(at, at + TCHAR_AS_HEX__IN_TCHARS);
    }

    return strOut;
}

TCHAR cCharEncoderUtil::hex_to_char(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    static const TCHAR max_char = TSS_TCHAR_MAX;
    static const TCHAR min_char = TSS_TCHAR_MIN;
    
    if (first + TCHAR_AS_HEX__IN_TCHARS != last)
    {
        ThrowAndAssert(eBadHexConversion(TSTRING(first,last)));
    }

#if ARCHAIC_STL
    TSTRING in(first, last);
    TISTRINGSTREAM ss(in.c_str());
#else
    TISTRINGSTREAM ss(TSTRING(first,last));
#endif

    tss_classic_locale(ss);
    
    ss.fill(_T('0'));
    ss.setf(std::ios::hex, std::ios::basefield);
    
    unsigned long ch;
    ss >> ch;

    if (ss.bad() || ss.fail())
    {      
        ThrowAndAssert(eBadHexConversion(TSTRING(first, last)));
    }	
    if ((TCHAR)ch > TSS_TCHAR_MAX || (TCHAR)ch < TSS_TCHAR_MIN)
    {
        ThrowAndAssert(eBadHexConversion(TSTRING(first, last)));
    }
    
    return (TCHAR)ch;
}


TSTRING cCharEncoderUtil::char_to_hex(TCHAR ch)
{
    TOSTRINGSTREAM ss;
    tss_classic_locale(ss);
    ss.setf(std::ios::hex, std::ios::basefield);
    
    ss.fill(_T('0'));
    ss.width(TCHAR_AS_HEX__IN_TCHARS);

    ss << tss::util::char_to_size(ch);

    tss_mkstr(out, ss);
    
    if (ss.bad() || ss.fail() || out.length() != TCHAR_AS_HEX__IN_TCHARS)
    {
        ThrowAndAssert(eBadHexConversion(TSTRING(1, ch)));
    }
    return out;
}

TSTRING cCharEncoderUtil::DecodeHexToChar(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end)
{
    // get hex numbers -- 2 chars
    TSTRING str;
    size_t  n = 0;
    for ((*pcur)++; n < TCHAR_AS_HEX__IN_TCHARS && (*pcur) != end; n++, (*pcur)++)
    {
        str += *(*pcur);
    }

    if (n != TCHAR_AS_HEX__IN_TCHARS)
    {
      ThrowAndAssert(eBadDecoderInput());
    }
    
    // convert hex numbers
    return HexValueToCharString(str);
}

//////////////////////////////////////////////////////////////////////////////
// ENCODER MEMBERS
//////////////////////////////////////////////////////////////////////////////


cEncoder::cEncoder(int e, int f) : m_fFlags(f)
{
    // add encodings
    if (e & NON_NARROWABLE)
        m_encodings.push_back(new cNonNarrowableCharEncoder);
    if (e & NON_PRINTABLE)
        m_encodings.push_back(new cNonPrintableCharEncoder(AllowWhiteSpace()));
    if (e & BACKSLASH)
        m_encodings.push_back(new cBackslashCharEncoder);
    if (e & DBL_QUOTE)
        m_encodings.push_back(new cQuoteCharEncoder);

    // assert that we weren't passed anything freaky
    ASSERT(0 == (e & ~(NON_NARROWABLE | NON_PRINTABLE | BACKSLASH | DBL_QUOTE)));

    // add flags
    ASSERT(!((m_fFlags & ROUNDTRIP) && (m_fFlags & NON_ROUNDTRIP)));

#ifdef TSS_DO_SCHEMA_VALIDATION

    // check assumptions about encodings
    ValidateSchema();

#endif
}


cEncoder::~cEncoder()
{
    sack_type::iterator itr;
    for (itr = m_encodings.begin(); itr != m_encodings.end(); ++itr)
        delete *itr;
}

bool cEncoder::RoundTrip() const
{
    return (0 != (m_fFlags & ROUNDTRIP));
}

bool cEncoder::AllowWhiteSpace() const
{
    return (0 != (m_fFlags & ALLOW_WHITESPACE));
}

//////////////////////////////////////////////////////////////////////////////
// ENCODER BASIC FUNCTIONALITY
//////////////////////////////////////////////////////////////////////////////

void cEncoder::Encode(TSTRING& strIn) const
{
    // TODO:BAM -- reserve space for strOut as an optimization?
    TSTRING strOut; // encoded string we will build up

    TSTRING::const_iterator       cur   = strIn.begin(); // pointer to working position in strIn
    const TSTRING::const_iterator end   = strIn.end();   // end of strIn
    TSTRING::const_iterator       first = end;           // identifies beginning of current character
    TSTRING::const_iterator       last  = end;           // identifies end of current character

    // while get next char (updates cur)
    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        bool                      fCharEncoded = false; // answers: did char need encoding?
        sack_type::const_iterator atE;

        // for all encoders
        for (atE = m_encodings.begin(); atE != m_encodings.end(); atE++)
        {
            // does char need encoding?
            if ((*atE)->NeedsEncoding(first, last))
            {
                strOut += Encode(first, last, atE);
                fCharEncoded = true;
                break; // each char should only fail at most one
                       // encoding test, so it should be cool to quit
            }
        }

        if (!fCharEncoded)
        {
            strOut.append(first, last); // simply add current char to output since it needed no encoding
        }
    }

    // pass back encoded string
    strIn = strOut;
}

TSTRING
cEncoder::Encode(TSTRING::const_iterator first, TSTRING::const_iterator last, sack_type::const_iterator encoding) const
{
    // encode it
    if (RoundTrip())
        return (*encoding)->EncodeRoundtrip(first, last);
    else
        return (*encoding)->EncodePretty(first, last);
}

void cEncoder::Decode(TSTRING& strIn) const
{
    // TODO:BAM -- reserve space for strOut as an optimization?
    TSTRING strOut; // decoded string we will build up

    TSTRING::const_iterator       cur   = strIn.begin(); // pointer to working position in strIn
    const TSTRING::const_iterator end   = strIn.end();   // end of strIn
    TSTRING::const_iterator       first = end;           // identifies beginning of current character
    TSTRING::const_iterator       last  = end;           // identifies end of current character


    // while get next char (updates cur)
    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        // is this char the escape character?
        if (IsSingleTCHAR(first, last) && *first == iCharEncoder::EscapeChar())
        {
            // get to identifier
            if (!cCharUtil::PopNextChar(cur, end, first, last))
                ThrowAndAssert(eBadDecoderInput());

            // this algorithm assumes that all identifiers are single char
            // so anything following the escape char should be a
            // single-char identifier
            if (!IsSingleTCHAR(first, last))
                THROW_INTERNAL("displayencoder.cpp");

            // determine to which encoding the identifier belongs
            bool                      fFoundEncoding = false;
            sack_type::const_iterator atE;
            for (atE = m_encodings.begin(); atE != m_encodings.end(); atE++)
            {
                // is this the right encoding?
                if (*first == (*atE)->Identifier())
                {
                    // this is the correct encoding....
                    fFoundEncoding = true;

                    // ...so decode char
                    strOut += (*atE)->Decode(&first, end); // should modify cur

                    cur = first; // advance current char pointer

                    break; // no need to run other tests after
                           // this because all identifiers should be unique
                }
            }

            if (!fFoundEncoding)
                ThrowAndAssert(eUnknownEscapeEncoding(TSTRING(1, *first)));
        }
        else
        {
            strOut.append(first, last);
        }
    }

    strIn = strOut;
}

//////////////////////////////////////////////////////////////////////////////
// ENCODER SCHEMA VALIDATION
//////////////////////////////////////////////////////////////////////////////

void cEncoder::ValidateSchema() const
{
    ASSERT(OnlyOneCatagoryPerChar());
    ASSERT(AllIdentifiersUnique());
}


// only tests single TCHAR characters (but of those, tests all of them)
bool cEncoder::OnlyOneCatagoryPerChar() const
{
    // TODO:BAM - man, is there a better way to do this?
    TCHAR   ch = TSS_TCHAR_MIN;
    TSTRING ach(1, ch);

    if (ch != TSS_TCHAR_MAX)
    {
        do
        {
            bool fFailedATest = false;

            ach[0] = ch;
            for (sack_type::const_iterator atE = m_encodings.begin(); atE != m_encodings.end(); atE++)
            {
                if ((*atE)->NeedsEncoding(ach.begin(), ach.end()))
                {
                    if (fFailedATest)
                        return false; // each char can only fail one test
                    else
                        fFailedATest = true;
                }
            }
            ch++;
        } while (ch != TSS_TCHAR_MAX);
    }
    return true;
}


bool cEncoder::AllIdentifiersUnique() const
{
    TSTRING chars;
    for (sack_type::const_iterator atE = m_encodings.begin(); atE != m_encodings.end(); atE++)
    {
        TCHAR chID = (*atE)->Identifier();
        if (chars.find(chID) == TSTRING::npos)
            chars += chID;
        else
            return false;
    }
    return true;
}


bool cEncoder::AllTestsRunOnEncodedString(const TSTRING& s) const
{
    TSTRING::const_iterator       cur   = s.begin(); // pointer to working position in s
    const TSTRING::const_iterator end   = s.end();   // end of s
    TSTRING::const_iterator       first = end;       // identifies beginning of current character
    TSTRING::const_iterator       last  = end;       // identifies end of current character


    // while get next char (updates cur)
    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        sack_type::const_iterator atE;
        for (atE = m_encodings.begin(); atE != m_encodings.end(); atE++)
        {
            if ((*atE)->NeedsEncoding(first, last))
            {
                return false;
            }
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// cDisplayEncoder MEMBERS
//////////////////////////////////////////////////////////////////////////////


cDisplayEncoder::cDisplayEncoder(Flags f) : cEncoder(NON_NARROWABLE | NON_PRINTABLE | BACKSLASH | DBL_QUOTE, f)
{
}

void cDisplayEncoder::Encode(TSTRING& str) const
{
    cEncoder::Encode(str);
}

bool cDisplayEncoder::Decode(TSTRING& str) const
{
    cEncoder::Decode(str);
    return true; // TODO:BAM -- throw error!
}
