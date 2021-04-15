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
#include "stdtripwire.h"
#include "core/debug.h"
#include "mailmessage.h"
#include <locale.h>
#include "core/stringutil.h"
#include "core/timeconvert.h"
#include "tw/systeminfo.h"
#include "core/file.h"
#include "core/ntmbs.h"


///////////////////////////////////////////////////////////////////////////////
cMailMessage::cMailMessage()
{
}


///////////////////////////////////////////////////////////////////////////////
cMailMessage::~cMailMessage()
{
}


///////////////////////////////////////////////////////////////////////////////
void cMailMessage::SetBody(const TSTRING& strBody)
{
    mstrBody = strBody;
}


///////////////////////////////////////////////////////////////////////////////
void cMailMessage::SetFrom(const TSTRING& strFrom)
{
    mstrFrom = strFrom;
}


///////////////////////////////////////////////////////////////////////////////
void cMailMessage::SetFromName(const TSTRING& strFromName)
{
    mstrFromName = strFromName;
}


///////////////////////////////////////////////////////////////////////////////
void cMailMessage::SetSubject(const TSTRING& strSubject)
{
    mstrSubject = strSubject;
}


///////////////////////////////////////////////////////////////////////////////
void cMailMessage::AddRecipient(const TSTRING& strRecipient)
{
    mvstrRecipients.push_back(strRecipient);
}


///////////////////////////////////////////////////////////////////////////////
void cMailMessage::AttachFile(const TSTRING& strFullPath)
{
    mvstrAttachments.push_back(strFullPath);
}


///////////////////////////////////////////////////////////////////////////////
bool cMailMessage::Send()
{
    // this is a pure virtual method. You should never end up here.
    ASSERT(false);
    return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// Returns true if enough things have been set so that a useful email message can be sent
//
bool cMailMessage::Ready()
{
    // You must specify 'from', 'to', the subject
    if (mstrFrom.length() == 0 || mvstrRecipients.size() == 0 || mstrSubject.length() == 0)
    {
        return false;
    }

    // make sure we can at least send something...
    if (mstrBody.empty())
    {
        mstrBody = _T("\r\n");
    }

    return true;
}

bool cMailMessage::GetAttachmentsAsString(std::string& s)
{
    s.erase();
    bool allOK = true;
    for (std::vector<TSTRING>::const_iterator at = mvstrAttachments.begin(); at != mvstrAttachments.end(); ++at)
    {
        s += "\r\n";

        cFile file;
        try
        {
            file.Open(at->c_str(), cFile::OPEN_READ);


            // Simply stream the file into the socket.
            // This will append the file as additional text.
            const cFile::File_t cBufSize = 2048;
            char                buf[cBufSize];
            cFile::File_t       bytes;

            while ((bytes = file.Read(buf, cBufSize)) != 0)
            {
                ASSERT(bytes > 0 && bytes <= cBufSize); // ensures typecast below
                ASSERT((std::string::size_type)bytes < std::string().max_size());

                s += std::string(buf, (size_t)bytes);
            }

            file.Close();
        }
        catch (eFile&)
        {
            // TODO: There is no method of reporting detailed information on
            // errors if they occur.  Perhaps someday there will be. At the moment
            // it does not seem necessary as the only attached files that will
            // be sent are temporary files that tripwire has just created. Thus
            // the likelyhood of failure is low.
            file.Close();
            allOK = false;
        }
    }

    // Send a new line to be sure the file ends properly
    // Failure to do this could result in the "<CRLF>.<CRLF>" begin acknowledged as
    // the end of the email message.
    s += "\r\n";

    return allOK;
}

std::string cMailMessage::Create822Header()
{
#if !ARCHAIC_STL  
    std::ostringstream ss;
#else
    strstream ss;
#endif
    
    std::string        strToList;
    for (std::vector<TSTRING>::size_type i = 0; i < mvstrRecipients.size(); i++)
    {
        if (strToList.length() > 0)
            strToList += ", ";

        strToList += cStringUtil::TstrToStr(mvstrRecipients[i]);
    }

    ss << cMailMessageUtil::FormatAddressHeader("MIME-Version", "1.0");

    TSTRING strDate;
    if (cMailMessageUtil::ReadDate(strDate))
        ss << cMailMessageUtil::FormatAddressHeader("Date", cStringUtil::TstrToStr(strDate));

    std::string fromAddress;
    if (!mstrFromName.empty())
    {
        fromAddress = "\"";
        fromAddress += cStringUtil::TstrToStr(mstrFromName);
        fromAddress += "\" <";
        fromAddress += cStringUtil::TstrToStr(mstrFrom);
        fromAddress += ">";
    }
    else
    {
        fromAddress = cStringUtil::TstrToStr(mstrFrom);
    }

    ss << cMailMessageUtil::FormatAddressHeader("From", fromAddress);
    ss << cMailMessageUtil::FormatAddressHeader("To", strToList);
    ss << cMailMessageUtil::FormatNonAddressHeader("Subject", cStringUtil::TstrToStr(mstrSubject));
    ss << cMailMessageUtil::FormatAddressHeader("Content-Type", "text/plain");

    return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cMailMessageUtil
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// gets the date in the RFC822 compliant form "Tue, 16 Mar 1999 10:53:17 -0800 (PST)"
// TODO:BAM -- make windows and unix use same function
bool cMailMessageUtil::ReadDate(TSTRING& strDateBuf)
{
    bool fGotDate = false;

#if HAVE_STRFTIME

    TCHAR      szDate[1024];
    time_t     current_time = time(NULL);
    struct tm* tm           = localtime(&current_time);

    const TCHAR* szFormat = _T("%a, %d %b %Y %H:%M:%S %z");

    size_t numChars = _tcsftime(szDate, countof(szDate), szFormat, tm);

    if (numChars != 0)
    {
        strDateBuf = szDate;
        fGotDate   = true;
    }

#else

    int64_t now  = cSystemInfo::GetExeStartTime();
    strDateBuf = cTimeUtil::GetRFC822Date(cTimeUtil::TimeToDateGMT(now));
    fGotDate   = true;

#endif // HAVE_STRFTIME

    return fGotDate;
}


static const char* util_Get_IANA_CharSet()
{
    const char* pCP = setlocale(LC_CTYPE, NULL);

    if (pCP && pCP[0])
    {
        std::string s = pCP;
        // TODO:BAM -- change this to ctype::tolower when all compilers
        // have a proper locale impl. or better yet move to core/twlocale.cpp
        std::transform(s.begin(), s.end(), s.begin(), tolower);
        if (std::string::npos != s.find("jp") || std::string::npos != s.find("jap"))
        {
            return "ISO-2022-JP";
        }
    }

    // default
    return "US-ASCII";
}


static bool NeedsEncoding(char ch)
{
    return ((ch < 33) || (ch > 60 && ch < 62) || (ch > 126));
}

static std::string EncodeChar(char ch)
{
    TOSTRINGSTREAM ss;
    tss_classic_locale(ss);
    ss.setf(std::ios::hex, std::ios::basefield);    
    
    ss.fill('0');

    ss.width(2);

    ss << (unsigned int)(unsigned char)ch;
    tss_end(ss);
    
    ASSERT(ss.str().length() == 2);

    // Make sure the hex is uppercase
    std::string s = ss.str();
    tss_free(ss);
    
    std::transform(s.begin(), s.end(), s.begin(), toupper);

    return s;
}


/* static */
iMimeEncoding* iMimeEncoding::GetInstance()
{
#define TSS_USE_QUOTED_PRINTABLE_MIME

    static iMimeEncoding* pME =

#if defined(TSS_USE_BASE64_MIME)
        new cBase64Encoding;
#elif defined(TSS_USE_QUOTED_PRINTABLE_MIME)
        new cQuotedPrintableEncoding;
#else
#    error what is iMimeEncoding?
#endif

    return pME;
}

// TODO:BAM -- line breaks
// TODO:BAM -- ToEncoding( const std::string& sIn, int maxLineLen = -1, int maxLen = -1 )
std::string cQuotedPrintableEncoding::Encode(const std::string& sIn, int maxLineLen, int maxLen)
{
    static const size_t _CHAR_AS_HEX_LEN  = 2;
    static const size_t _ENCODED_CHAR_LEN = _CHAR_AS_HEX_LEN + sizeof('=');

    std::string sOut;
    sOut.resize(sIn.size() * _ENCODED_CHAR_LEN); // optimize.  one char can turn into 3

    std::string::const_iterator at;
    std::string::size_type      i       = 0;
    std::string::size_type      lineLen = 0;
    for (at = sIn.begin(); at != sIn.end(); ++at, lineLen += _ENCODED_CHAR_LEN)
    {
        if (NeedsEncoding(*at))
        {
            ASSERT((unsigned char)*at <= 0xFF);

            std::string sTmp = EncodeChar(*at);
            ASSERT(sTmp.length() == _CHAR_AS_HEX_LEN);

            sOut[i++] = '=';

            std::copy(&sTmp[0], &sTmp[_CHAR_AS_HEX_LEN], &sOut[i]);
            ASSERT(_CHAR_AS_HEX_LEN > 0);
            i += _CHAR_AS_HEX_LEN;
        }
        else
        {
            sOut[i++] = *at;
        }

        //
        // if string is too long, just quit
        //
        if (maxLen != -1 && i >= maxLen - _ENCODED_CHAR_LEN)
        {
            break;
        }

        //
        // check line len
        //
        if (maxLineLen != -1 && lineLen >= maxLineLen - _ENCODED_CHAR_LEN)
        {
            // append EOL
            sOut[i++] = '=';
            sOut[i++] = '\r';
            sOut[i++] = '\n';
            lineLen   = 0;
        }
    }

    sOut.resize(i);

    return sOut;
}

std::string cMailMessageUtil::CreateEncodedText(const std::string& text)
{
    cDebug d("cMailMessageUtil::CreateEncodedText");

    d.TraceDebug("came in as: %s\n", text.c_str());

    std::string s;


    s += "=?";
    s += util_Get_IANA_CharSet();

    s += "?";
    s += iMimeEncoding::GetInstance()->GetEncodedWordIdentifier();
    s += "?";
    s += iMimeEncoding::GetInstance()->Encode(text, -1, _MAX_RFC822_HEADER_LEN);
    s += "?=";

    ASSERT(s.length() < _MAX_RFC822_LINE_LEN - _EOL_LEN);

    d.TraceDebug("came out as: %s\n", s.c_str());

    return s;
}

// TODO:BAM -- note that address headers can only have usascii chars
// TODO:BAM -- lines can only be 76 chars long -- enforce that!
std::string cMailMessageUtil::FormatAddressHeader(const std::string& name, const std::string& addr)
{
    cDebug d("cMailMessageUtil::FormatAddressHeader");

    d.TraceDebug("name: %s, value: %s\n", name.c_str(), addr.c_str());

    std::string s;

    s += name;
    s += ": ";
    s += addr;
    s += "\r\n";

    d.TraceDebug("came out as: %s\n", s.c_str());

    return s;
}

// TODO:BAM -- lines can only be 76 chars long -- enforce that!
std::string cMailMessageUtil::FormatNonAddressHeader(const std::string& name, const std::string& valC)
{
    cDebug d("cMailMessageUtil::FormatNonAddressHeader");

    d.TraceDebug("name: %s, value: %s\n", name.c_str(), valC.c_str());

    std::string val = valC;
    ASSERT(val != "To");
    ASSERT(val != "From");
    ASSERT(val != "Sender");
    ASSERT(val != "cc");
    ASSERT(val != "bcc");
    // ...

    std::string s;

    s += name;
    s += ": ";

    if (HasNonAsciiChars(val))
        s += CreateEncodedText(val);
    else
        s += val;

    s += "\r\n";


    d.TraceDebug("came out as: %s\n", s.c_str());

    ASSERT(s.length() < _MAX_RFC822_LINE_LEN - _EOL_LEN);

    return s;
}


bool cMailMessageUtil::HasNonAsciiChars(const std::string& s)
{
    for (std::string::const_iterator at = s.begin(); at != s.end(); ++at)
    {
        if ((unsigned char)*at > (unsigned char)0x7F)
            return true;
    }

    return false;
}

// converts lone \n's to \r\n
std::string& cMailMessageUtil::LFToCRLF(std::string& sIn)
{
    cDebug d("cMailMessageUtil::LFToCRLF");

    if (sIn.empty())
        return sIn;

    std::string sOut;

    // worst case: could be just '\n's
    // in which case we'd double the length
    sOut.resize(sIn.size() * 2);

    bool lastCharCR = false;

    std::string::size_type       stringSize = 0;
    std::string::const_iterator  at;
    std::string::difference_type charSize;
    for (at = sIn.begin(), charSize = ((*at ? at + 1 : at) - at); at != sIn.end();
         at += charSize, charSize   = ((*at ? at + 1 : at) - at))
    {
        ASSERT(charSize > 0);

        if (*at == '\n')
        {
            if (!lastCharCR)
            {
                d.TraceDebug("Adding LF\n");
                sOut[stringSize++] = '\r';
            }

            lastCharCR = false;
        }
        else if (*at == '\r')
        {
            lastCharCR = true;
        }
        else
        {
            lastCharCR = false;
        }

        std::copy(at, at + charSize, &sOut[stringSize]);
        stringSize += charSize;
    }

    sOut.resize(stringSize);
    sIn = sOut;
    return sIn;
}


// #define RDIFALCO_BASE64_IMPLEMENTATION
#ifndef RDIFALCO_BASE64_IMPLEMENTATION
/*
static 
std::string 
util_Base64Encode( const uint8_t b[3], int size )
{
    // TODO:BAM -- what about endianness?
    ASSERT( size > 0 && size <= 3 );
    static char v64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string s;

    if( size >= 1 )
    {
        // encode A
        s += v64[ b[0] >> 2 ];
    }
    
    if( size >= 2 )
    {
        // encode B,C
        s += v64[ ( ( b[0] & (uint8_t)0x3 ) << 4 ) | ( b[1] >> 4 ) ];
        
        if( size == 3 )
            s += v64[ ( ( b[1] & (uint8_t)0xF ) << 2 ) | ( b[2] >> 6 ) ];
        else
            s += v64[ ( ( b[1] & (uint8_t)0xF ) << 2 ) ];
    }
    
    if( size >= 3 )
    {
        // encode D
        s += v64[ b[2] & (uint8_t)0x3F ];
    }

    // padding
    if( size == 1 )
        s += "==";
    else if( size == 2 )
        s += "=";

    return s;
}

// MS _defines_ 'min' and 'max', so I can't use std::min(). 
template< class T >
const T& MS_SUCKS_min( const T& a, const T& b )
{
    return( b < a ? b : a );
}


/// More consistent with the rest of our Convert interfaces

// NOTE: Works when we don't want to allocate anything new

const std::string::value_type*
cMailMessageUtil::ConvertBase64( 
    std::string&  sEncode,
    const uint8_t*   pchSrc,
    size_t        nchSrc )
{
    sEncode.assign( ToBase64( pchSrc, nchSrc ) );
    return sEncode.c_str();
}


std::string 
cMailMessageUtil::ToBase64( const uint8_t* p, size_t size )
{
    ASSERT( sizeof( uint8_t ) == sizeof( uint8_t ) ); // everything breaks otherwise
    std::string s;    

    const int MAX_WORKING_BYTES = 3;
    const int CHARS_PER_WORKING_BYTES = 4;
    const int MAX_CHARS_PER_BYTE = 2; // should be 
    ASSERT( MAX_CHARS_PER_BYTE > CHARS_PER_WORKING_BYTES/MAX_WORKING_BYTES );
    const int NERVOUS_LINE_BUFFER = 10;
    const uint8_t* at = p;
    uint8_t buf[ MAX_WORKING_BYTES ];
    int nbLeft;
    int nbWorking;
    int nchCurLine;
    std::string::size_type i = 0; // where we are in the output string

    s.resize( size * MAX_CHARS_PER_BYTE );


    for (  
            nchCurLine = 0, nbLeft = size, nbWorking = MS_SUCKS_min( MAX_WORKING_BYTES, nbLeft );
            nbLeft > 0;
            nchCurLine += CHARS_PER_WORKING_BYTES, at += nbWorking, nbLeft -= nbWorking, nbWorking = MS_SUCKS_min( MAX_WORKING_BYTES, nbLeft )
        )
    {
        ASSERT( nbWorking > 0 );

        // fill out the working buffer
        std::copy( at, at + nbWorking, &buf[0] );
        
        // encode from working buffer to 'enc' string
        std::string enc = util_Base64Encode( buf, nbWorking );

        // append 'enc' string to output string
        for( std::string::const_iterator at = enc.begin();
             at != enc.end();
             at++ )
        {
            s[i++] = *at;
        }

        // make sure we aren't over the max line length
        if( nchCurLine > ( _MAX_RFC822_LINE_LEN - NERVOUS_LINE_BUFFER ) ) // NERVOUS_LINE_BUFFER for a little margin of error
        {
            s[i++] = '\r';
            s[i++] = '\n';
            nchCurLine = 0;
        }
    }

    s.resize( i );
    return s;
}
*/
#else //RDIFALCO_BASE64_IMPLEMENTATION


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// RAD:10311999 -- A Different Try
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace /*Unique*/
{

#    ifdef TSS_MAKE_EOL_CRLF
const uint8_t _aszEoL[] = "\r\n";
size_t _EOL_LEN = 2;

#    else
const uint8_t _aszEoL[] = "\n";
size_t     _EOL_LEN  = 1;

#    endif

const uint8_t _abBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "0123456789+/";

inline size_t tss_base64_amplitude() // TODO: Make a Constant
{
    /* 
        * NOTE:RAD -- Calculate Max Incoming Bytes:
        *
        *   eol_bytes = ( UINT_MAX / _MAX_RFC822_LINE_LEN ) * _EOL_LEN;
        *   max_bytes = UINT_MAX - eol_bytes;
        *   cvt_bytes = max_bytes / 4 * 3;    
        */
    return (((UINT_MAX - (UINT_MAX / _MAX_RFC822_LINE_LEN * _EOL_LEN)) / 4) * 3) + 1;
}

inline size_t tss_encode_base64_size(size_t nchSource)
{
    ASSERT(nchSource < tss_base64_amplitude()); // NOTE:

    /* NOTE: The formula is --
            + bring source bytes to an even divisible of 3 (the padding bytes)
            + number of source bytes to converted (3:4 conversion ratio)
            + ( number of lines ) * ( size of end of line string ) */

    size_t nchPadding = nchSource % 3;
    size_t nchOut = ((nchSource + nchPadding) / 3) * 4;
    size_t nchEol = (nchOut / (_MAX_RFC822_LINE_LEN - _EOL_LEN)) * _EOL_LEN;

    return nchOut + nchEol + 2;
}

void tss_encode_digit_base64(const uint8_t*& pchSrc, size_t& nchSrcLeft, uint8_t*& pchBuf, size_t& nchBufLeft)
{
    // NOTE:RAD -- Redundant and ugly but very fast!!

    ASSERT(nchBufLeft > 4); // One for NULL

    switch (nchSrcLeft)
    {
    case 0:
    {
        ASSERTMSG(false, "Invalid call to encode_base64_digit!");
        break;
    }
    case 1:
    {
        int ch1 = int(*pchSrc++ & 0xFF);

        *pchBuf++ = _abBase64[(ch1 & 0xFC) >> 2];
        *pchBuf++ = _abBase64[(ch1 & 0x03) << 4];
        *pchBuf++ = '=';
        *pchBuf++ = '=';

        nchSrcLeft = 0; // NOTE:
        break;
    }
    case 2:
    {
        int ch1 = int(*pchSrc++ & 0xFF);
        int ch2 = int(*pchSrc++ & 0xFF);

        *pchBuf++ = _abBase64[((ch1 & 0xFC) >> 2)];
        *pchBuf++ = _abBase64[((ch1 & 0x03) << 4) | ((ch2 & 0xF0) >> 4)];
        *pchBuf++ = _abBase64[((ch2 & 0x0F) << 2)];
        *pchBuf++ = '=';

        nchSrcLeft = 0; // NOTE:
        break;
    }
    default:
    {

        int ch1 = int(*pchSrc++ & 0xFF);
        int ch2 = int(*pchSrc++ & 0xFF);
        int ch3 = int(*pchSrc++ & 0xFF);

        *pchBuf++ = _abBase64[((ch1 & 0xFC) >> 2)];
        *pchBuf++ = _abBase64[((ch1 & 0x03) << 4) | ((ch2 & 0xF0) >> 4)];
        *pchBuf++ = _abBase64[((ch2 & 0x0F) << 2) | ((ch3 & 0xC0) >> 6)];
        *pchBuf++ = _abBase64[(ch3 & 0x3F)];

        nchSrcLeft -= 3; // NOTE: Probably greater than 3
        break;
    }
    }

    nchBufLeft -= 4; // Always the same due to padding!
};


size_t tss_encode_base64(const uint8_t* pchSource, size_t nchSource, uint8_t* pchBuffer, size_t nchBuffer)
{
    const size_t _ERROR = std::string::npos; // -1;


    //--Assert Preconditions

    if (pchSource == 0 || nchSource == 0)
        return _ERROR;


    //--If pchBuffer == 0, User is requesting Size

    size_t nchBuf = tss_encode_base64_size(nchSource);

    if (pchBuffer == 0) // Requesting Size
        return nchBuf;

    if (nchBuf > nchBuffer) // DOH!!
        return _ERROR;


    //--Get three characters at a time and encode them (watching linelen)

    uint8_t* pchBuf = (pchBuffer + 0);
    const uint8_t* pchSrc = (pchSource + 0);


    const size_t _max_linelen = (_MAX_RFC822_LINE_LEN - _EOL_LEN);
    size_t nLineLen = 0;

    for (; nchSource;)
    {
        // Like iconv: Increment pointers and decrement sizes
        tss_encode_digit_base64(pchSrc, nchSource, pchBuf, nchBuf);

        nLineLen += 4;

        if (nchSource == 0 || nLineLen > _max_linelen)
        {
            nLineLen = 0; // RESET:

            const uint8_t* pchEol = &_aszEoL[0];
            while (*pchEol)
                *pchBuf++ = *pchEol++;
        }
    }

    *pchBuf = 0x00; // CAUTION: Null Terminate!!!

    return (pchBuf - pchBuffer);
}
} // namespace


const std::string::value_type*
cMailMessageUtil::ConvertBase64(std::string& sEncode, const uint8_t* pchSrc, size_t nchSrc)
{
    size_t nch = tss_encode_base64(pchSrc, nchSrc, 0, 0); // Like mbstowcs

    sEncode.reserve(nch); // Make it big enough but don't resize
    sEncode.resize(0);    // String must be clean in case of exception!

    const char* pch = sEncode.c_str(); // Get Pointer (won't change)

    size_t nLength = // Action
        tss_encode_base64(pchSrc, nchSrc, (uint8_t*)pch, nch);

    if (nLength == std::string::npos)
        throw std::bad_alloc();

    sEncode.resize(nLength); // Now it's okay to resize

    return pch; // So user can do stuff like "out << ConvertBase64()";
}


std::string cMailMessageUtil::ToBase64(const uint8_t* pchSrc, size_t nchSrc)
{
    // NOTE: It sucks to use std::string this way! Should be
    //       passed in by reference.

    std::string sucks;
    cMailMessageUtil::ConvertBase64(sucks, pchSrc, nchSrc);
    return sucks;
}
#endif //RDIFALCO_BASE64_IMPLEMENTATION
