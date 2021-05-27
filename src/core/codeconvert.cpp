
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
// Name....: codeconvert.cpp
// Date....: 9/2/99
// Creator.: Brian McFeely (bmcfeely)
//
// Convert a string between locale NTMBS encoding and UCS2
//


/// Requirements

#include "stdcore.h"          // for: pch
#include "core/codeconvert.h" // for: These classes
#include "corestrings.h"      // for: Error Strings
#include "fsservices.h"       // for: strerror
#include "ntmbs.h"
#include "errno.h"


#ifdef DEBUG
#    define TSS_CCONV_VERBOSE // Uncomment for verbose tracing!
#endif


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DEFINES
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define TSS_USE_ICONV_CCONV16 HAVE_ICONV_H
#define TSS_USE_UCS2_CCONV16 (!(HAVE_ICONV_H) && WCHAR_REP_IS_UCS2 && WCHAR_IS_16_BITS)
#define TSS_USE_UCS2_CCONV32 (!(HAVE_ICONV_H) && WCHAR_REP_IS_UCS2 && WCHAR_IS_32_BITS)

#if ICONV_CONST_SOURCE
#    define ICONV_SOURCE_TYPE const char
#else
#    define ICONV_SOURCE_TYPE char
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Static Data
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

iCodeConverter* iCodeConverter::m_pInst = NULL;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iCodeConverter
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// has a maximum of 512 chars of output
std::string util_output_bytes(void* p, size_t n)
{
    TOSTRINGSTREAM ss;
    tss_classic_locale(ss);
    ss.setf(std::ios::hex, std::ios::basefield);
    
    for (size_t i = 0; i < n; i++)
    {
        ss.width(2);
        ss << toupper(tss::util::char_to_size(((char*)p)[i])) << " ";
    }

    tss_mkstr(s, ss);
    
    if (s.length() > 512)
    {
        s = "truncated output: " + s;
        s.resize(512);
    }

    return s;
}

iCodeConverter* iCodeConverter::GetInstance()
{
    if (!m_pInst)
    {
        m_pInst = CreateConverter();
    }

    ASSERT(m_pInst);
    return m_pInst;
}

void iCodeConverter::Finit()
{
    delete m_pInst;
    m_pInst = 0;
}

iCodeConverter* iCodeConverter::CreateConverter()
{
    cDebug d("iCodeConverter::CreateConverter()");

#if TSS_USE_ICONV_CCONV16

    if (cIconvConverter::Test())
    {
        d.TraceDebug("using cIconvConverter\n");
        return new cIconvConverter;
    }
    else if (cDoubleIconvConverter::Test())
    {
        d.TraceDebug("using cDoubleIconvConverter\n");
        return new cDoubleIconvConverter;
    }
    else
    {
        d.TraceDebug("using CreateGoodEnoughConverter\n");
        return CreateGoodEnoughConverter();
    }

#elif TSS_USE_UCS2_CCONV32

    d.TraceDebug("using cWcharIs32BitUcs2Converterer\n");
    return new cWcharIs32BitUcs2Converterer;

#elif TSS_USE_UCS2_CCONV16

    d.TraceDebug("using cWcharIs16BitUcs2Converterer\n");
    return new cWcharIs16BitUcs2Converterer;

#else // we don't have a proper converter, so fudge a little...

    d.TraceDebug("using CreateGoodEnoughConverter\n");
    return CreateGoodEnoughConverter();

#endif

    ASSERT(!"reachable");
}


iCodeConverter* iCodeConverter::CreateGoodEnoughConverter()
{
    //
    // let someone know that we don't work on non-english OSs...
    //
    cDebug d("iCodeConverter::CreateGoodEnoughConverter()");
    d.TraceAlways("database WILL NOT be portable to other platforms.\n");

    return new cGoodEnoughConverterer;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iconv Converters
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if TSS_USE_ICONV_CCONV16

#include <nl_types.h>
#include <langinfo.h>


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Locale Helpers for iconv()
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "core/ntmbs.h"

namespace /*Unique*/
{
void tss_dispatch_iconvopen_error()
{
    cDebug d("tss_dispatch_iconvopen_error");

    switch (errno)
    {
    case EMFILE:
        d.TraceDebug("EMFILE: {OPEN_MAX} files  descriptors  are  currently open in the calling process..\n");
        break;
    case ENFILE:
        d.TraceDebug("ENFILE: Too many files are currently open in the system.\n");
        break;
    case ENOMEM:
        d.TraceDebug("ENOMEM: Insufficient storage space is available.\n");
        break;
    case EINVAL:
        d.TraceDebug("EINVAL: The  conversion  specified  by  fromcode  and tocode  is  not  supported by the "
                     "implementation.\n");
        break;
    default:
        d.TraceDebug("UNKNOWN: Unknown error.\n");
        break;
    }
}

int tss_dispatch_errno(cDebug& d)
{
    // Reasons for failure:
    //  [EILSEQ] Input conv stopped due to an unknown input byte
    //  [E2BIG]  Input conversion stopped due to lack of space in the output
    //  [EINVAL] Input conversion stopped due to an incomplete character
    //  [EBADF]  The cd argument is not a valid open conversion descriptor.
    //                 errno
    d.TraceDebug(_T( "iconv failed with: %s\n" ), iFSServices::GetInstance()->GetErrString().c_str());

    /// RAD: Always throw on error (Let user Catch if they want to go on)
    switch (errno)
    {
    case EILSEQ:
    case EINVAL:
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_BADCHAR));
        break;

    case E2BIG:
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_OVERFLOW));
        break;

    case EBADF:
        throw eConverterUnknownCodepage(TSS_GetString(cCore, core::STR_UNKNOWN));
        break;
    default:
        throw eConverterFatal(TSS_GetString(cCore, core::STR_UNKNOWN));
    }

    return -1;
}

template<class BufferT, class SourceT> class ByteConvert
{
public:
    bool Convert(BufferT** ppBuf, size_t* pnBufferLeft, const SourceT** ppSrc, size_t* pnSourceLeft)
    {
        cDebug d("ByteConvert::Convert< BufferT, SourceT >()");

        //-- Copy manually into Buffer
        **ppBuf = cConvertUtil::ConvertNonChar(**ppSrc);

        //-- Decrement counters
        *pnSourceLeft -= sizeof(SourceT); // Decrement Source Left
        *pnBufferLeft -= sizeof(BufferT); // Decrement Buffer Left

        //-- Reposition pointers
        (*ppSrc)++; // Skip one SourceT fwd
        (*ppBuf)++; // Skip one BufferT fwd

        return true;
    }
};


class UTF8_Util
{
public:
    enum
    {
        INVALID_VALUE = 0xFF
    };
};

// convert to INVALID_VALUE and remember the byte value
template<class BufferT, class SourceT> class ToUTF8Convert
{
public:
    ToUTF8Convert(std::list<byte>& lb) : m_lb(lb)
    {
    }

    bool Convert(mbchar_t** ppBuf, size_t* pnBufferLeft, const dbchar_t** ppSrc, size_t* pnSourceLeft)
    {
        cDebug d("ToUTF8Convert::Convert< mbchar_t, dbchar_t >()");

        // store the non char value
        m_lb.push_back(cConvertUtil::ConvertNonChar(**ppSrc));

        // flag 'invalid char'
        **ppBuf = (mbchar_t)UTF8_Util::INVALID_VALUE;

        //-- Decrement counters
        *pnSourceLeft -= sizeof(dbchar_t); // Decrement Source Left
        *pnBufferLeft -= sizeof(mbchar_t); // Decrement Buffer Left

        //-- Reposition pointers
        (*ppSrc)++; // Skip one SourceT fwd
        (*ppBuf)++; // Skip one BufferT fwd

        return true;
    }

    bool Convert(mbchar_t** ppBuf, size_t* pnBufferLeft, const mbchar_t** ppSrc, size_t* pnSourceLeft)
    {
        cDebug d("ToUTF8Convert::Convert< char, char >()");

        // store non-char value
        m_lb.push_back(**ppSrc);

        // flag 'invalid char'
        **ppBuf = (char)UTF8_Util::INVALID_VALUE;

        //-- Decrement counters
        *pnSourceLeft -= sizeof(mbchar_t); // Decrement Source Left
        *pnBufferLeft -= sizeof(mbchar_t); // Decrement Buffer Left

        //-- Reposition pointers
        (*ppSrc)++; // Skip one SourceT fwd
        (*ppBuf)++; // Skip one BufferT fwd

        return true;
    }

private:
    // TODO:Get rid of this guy and just use a vector<BufferT*,BufferT> and
    // insert them when finished with second step
    std::list<byte>& m_lb;
};

template<class BufferT, class SourceT> class FromUTF8Convert
{
public:
    FromUTF8Convert(std::list<byte>& lb) : m_lb(lb)
    {
    }

    bool Convert(dbchar_t** ppBuf, size_t* pnBufferLeft, const mbchar_t** ppSrc, size_t* pnSourceLeft)
    {
        cDebug d("FromUTF8Convert::Convert< dbchar_t, mbchar_t >()");

        //-- Must be our flag char
        size_t value = tss::util::char_to_size(**ppSrc);
        if (value != (size_t)UTF8_Util::INVALID_VALUE)
        {
            d.TraceDebug("value was not flag char: %X...\n", value);
            return false;
        }

        //-- Get correct character from our byte array
        ASSERT(!m_lb.empty());
        mbchar_t newVal = (mbchar_t)tss::util::char_to_size(m_lb.front());
        m_lb.pop_front();

        //-- Copy character
        d.TraceDebug("converted to: %X\n", tss::util::char_to_size(newVal));
        **ppBuf = cConvertUtil::ConvertNonChar(newVal);

        //-- Decrement counters
        *pnSourceLeft -= sizeof(mbchar_t); // Decrement Source Left
        *pnBufferLeft -= sizeof(dbchar_t); // Decrement Buffer Left

        //-- Reposition pointers
        (*ppSrc)++; // Skip one mbchar_t fwd
        (*ppBuf)++; // Skip one dbchar_t fwd

        return true;
    }

    // Converts a UTF-8 value to corresponding UCS2 char (in the private
    // use range) whose value is 0xE000 < x < 0xE0FF in UCS2.
    // Must be of the form 11101110 100000xx 10xxxxxx
    bool Convert(mbchar_t** ppBuf, size_t* pnBufferLeft, const mbchar_t** ppSrc, size_t* pnSourceLeft)
    {
        cDebug d("FromUTF8Convert::Convert< mbchar_t, mbchar_t >()");
        /*
                const size_t NUM_CHARS = 3;

                //-- Must be our cast byte value
                if( *pnBufferLeft < NUM_CHARS )
                {
                    d.TraceDebug( "not enough buffer space to be our cast byte: %d\n", *pnBufferLeft );
                    return false;
                }

                size_t first  = tss::util::char_to_size( (*ppSrc)[0] );
                size_t second = tss::util::char_to_size( (*ppSrc)[1] );
                size_t third  = tss::util::char_to_size( (*ppSrc)[2] );

                d.TraceDebug( "have: first: 0x%08X, second: 0x%08X, third: 0x%08X\n",
                                first, second, third );

                //-- We expect the value to be UCS2_RESERVED_START + [0x00,0xFF]
                //   ( 0xE000 - 0xE0FF ) which would be of the form 
                //   11101110 100000xx 10xxxxxx in UTF-8
                if( ( first != 0xEEu ) ||
                    ( ( second & 0xFCu ) != 0x80u ) ||
                    ( ( third  & 0xC0u ) != 0x80u ) )
                {
                    d.TraceDebug( "Value was not UTF-8 for a char <= 0xE0FF and > 0xE000\n" );
                    return false;
                }

                //-- Get value of character
                size_t newVal = ( ( second & 0x03u ) << 6 ) | ( third & 0x3Fu );
                
                if( ( newVal > ( UTF8_Util::UCS2_RESERVED_START + 0xFF ) ) ||
                    ( newVal < UTF8_Util::UCS2_RESERVED_START ) )
                {
                    d.TraceDebug( "value was not in our private use range: %X...\n", newVal );
                    return false;
                }

                //-- Remove offset
                newVal -= UTF8_Util::UCS2_RESERVED_START;
                ASSERT( newVal <= 0xFF );

                //-- Copy character
                d.TraceDebug( "converted to: %X\n", newVal );
                **ppBuf = (mbchar_t)newVal;       
                                                
                //-- Decrement counters
                *pnSourceLeft -= NUM_CHARS * sizeof(mbchar_t);  // Decrement Source Left
                *pnBufferLeft -=             sizeof(mbchar_t);  // Decrement Buffer Left

                //-- Reposition pointers
                (*ppSrc) += NUM_CHARS;      // Skip two chars fwd
                (*ppBuf) += 1;              // Skip one char fwd

                return true;
                */

        //-- Must be our flag char
        size_t value = tss::util::char_to_size(**ppSrc);
        if (value != (size_t)UTF8_Util::INVALID_VALUE)
        {
            d.TraceDebug("value was not flag char: %X...\n", value);
            return false;
        }

        //-- Get correct character from our byte array
        ASSERT(!m_lb.empty());
        mbchar_t newVal = (mbchar_t)tss::util::char_to_size(m_lb.front());
        m_lb.pop_front();

        //-- Copy character
        d.TraceDebug("converted to: %X\n", tss::util::char_to_size(newVal));
        **ppBuf = newVal;

        //-- Decrement counters
        *pnSourceLeft -= sizeof(mbchar_t); // Decrement Source Left
        *pnBufferLeft -= sizeof(mbchar_t); // Decrement Buffer Left

        //-- Reposition pointers
        (*ppSrc)++; // Skip one mbchar_t fwd
        (*ppBuf)++; // Skip one dbchar_t fwd

        return true;
    }

private:
    // TODO:Get rid of this guy and just use a vector<BufferT*,BufferT> and
    // insert them when finished with second step
    std::list<byte>& m_lb;
};

bool tss_ReverseConvert(iconv_t revConv, const char* pConvertedFrom, size_t nFrom, char* pConvertedTo, size_t nTo)
{
    cDebug d("tss_ReverseConvert< B, S, C >()");
    d.TraceDebug("Converted from: %s\n", util_output_bytes((void*)pConvertedFrom, nFrom).c_str());
    d.TraceDebug("Converted to: %s\n", util_output_bytes((void*)pConvertedTo, nTo).c_str());

    char aBuffer[MB_LEN_MAX];
#    ifdef DEBUG
    for (size_t foo = 0; foo < sizeof(aBuffer); foo++)
        aBuffer[foo] = 0xCD;
#    endif
    char*  pBuf = &aBuffer[0];
    size_t nBuf = sizeof(aBuffer);

    const char* pSrc = pConvertedTo;
    size_t      nSrc = nTo;

    size_t nConv =                        // NOTE:
        iconv(revConv,                    // On return, these addresses
              (ICONV_SOURCE_TYPE**)&pSrc, // are set for one past the last
              &nSrc,                      // "item" converted successfully
              &pBuf,
              &nBuf);

    if (nConv == (size_t)-1)
    {
        d.TraceDebug("reverse conversion failed: iconv error\n");
        return false;
    }

    d.TraceDebug("sizeof( aBuffer ): %u, nBuf: %u, nFrom: %u\n", sizeof(aBuffer), nBuf, nFrom);
    d.TraceDebug("reverse conversion got: %s\n", util_output_bytes((void*)&aBuffer[0], sizeof(aBuffer) - nBuf).c_str());

    if (((sizeof(aBuffer) - nBuf) != nFrom) ||             // convertedFrom and check must be same size
        (0 != memcmp(pConvertedFrom, &aBuffer[0], nFrom))) // must be same value, too
    {
        d.TraceDebug("reverse conversion failed: converted to a different value\n");
        return false;
    }

    return true;
}

template<class CharT> bool tss_IsFlaggedCharacter(CharT ch)
{
    return false;
}

template<> bool tss_IsFlaggedCharacter<dbchar_t>(dbchar_t wch)
{
    return cConvertUtil::ValueInReservedRange(wch);
}

template<class BufferT, class SourceT>
int tss_ConvertOneCharacter(iconv_t      convForward,
                            iconv_t      convReverse,
                            const char** ppSource,
                            size_t*      pnSourceLeft,
                            char**       ppBuffer,
                            size_t*      pnBufferLeft
#    if (!SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST)
                            ,
                            BufferT /*dummy*/,
                            SourceT /*dummy*/
#    endif
)
{
    cDebug d("tss_ConvertOneCharacter< B, S, C >()");

    const char* pSrc = *ppSource;
    char*       pBuf = *ppBuffer;

    //-- HACK!!!! BAM -- see if it is one of our reserved UCS2 characters
    // TODO:BAM -- how does this affect our converters that don't use the UTF-8 step?
    if (tss_IsFlaggedCharacter(**((SourceT**)ppSource)))
    {
        d.TraceDebug("Found one of our unconvertable characters in the reserved range!\n");
        return -1;
    }

    //-- Try to find the number of items needed to get a complete character
    size_t nSrcTry;
    for (nSrcTry = sizeof(SourceT); nSrcTry <= *pnBufferLeft && nSrcTry <= MB_LEN_MAX; nSrcTry += sizeof(SourceT))
    {
        size_t nSrcLeft = nSrcTry;
        size_t nBufLeft = *pnBufferLeft; // Try to find a character in 'n' items

        // NOTE: On solaris sparc, iconv will attempt to NULL terminate the output,
        // so make sure that the buffer has space for a terminating NULL
        d.TraceDebug("attempting to convert with %u items\n", nSrcTry);
        size_t nConv =                        // NOTE:
            iconv(convForward,                // On return, these addresses
                  (ICONV_SOURCE_TYPE**)&pSrc, // are set for one past the last
                  &nSrcLeft,                  // "item" converted successfully
                  &pBuf,
                  &nBufLeft);

        if (nConv == (size_t)-1)
        {
            if (errno == EINVAL)
            {
                d.TraceDebug("more items needed\n");
                continue;
            }
            else
            {
                d.TraceDebug("iconv failed with %d (not EINVAL)\n", errno);
                return -1;
            }
        }
        else
        {
            if (tss_ReverseConvert(convReverse, *ppSource, pSrc - *ppSource, *ppBuffer, pBuf - *ppBuffer))
            {
                // Modify source items to return

                ASSERT(nSrcLeft == 0);

                *ppSource = pSrc;
                *ppBuffer = pBuf;
                *pnSourceLeft -= nSrcTry;
                *pnBufferLeft = nBufLeft;

                return nConv;
            }
            else
            {
                d.TraceDebug("reverse conversion failed\n");
                return -1;
            }
        }
    }

    //-- No valid character found in nBufferLeft or MB_LEN_MAX items
    d.TraceDebug("no valid character found after %u items.\n", nSrcTry - 1);
    return -1;
}


// NOTE: <n???Items> is number of T's (as opposed to the number of Characters
//       or Bytes) So for a NTWCS sequence, Item is the number of Characters
//       while for a NTMBS sequence, Item is the number of char's (Bytes).
//       The low-down is that n????Items represents the "Count of T's"

// NOTE: pBuffer should really have (nBufferItems+1) buffer items, because some
//       platforms use the last character to NULL terminate.
template<class BufferT, class SourceT, class ConvT>
int tss_Converter(iconv_t        convForward,
                  iconv_t        convReverse,
                  BufferT*       pBuffer,
                  size_t         nBufferItems,
                  const SourceT* pSource,
                  size_t         nSourceItems,
                  ConvT&         ConvertByte)
{
    cDebug d("tss_Converter< BufferT, SourceT >()");

#    ifdef DEBUG
    for (size_t s = nBufferItems; s; s--)
        pBuffer[s] = 0xCD;
    d.TraceDebug("sizeof buffer: %d, sizeof source: %d\n", sizeof(BufferT), sizeof(SourceT));
    d.TraceDebug("buffer size: %d, source size: %d\n", nBufferItems, nSourceItems);
    d.TraceDebug("source: %s\n", util_output_bytes((void*)pSource, nSourceItems * sizeof(SourceT)).c_str());
#    endif

#    ifndef HACK_FOR_SOLARIS // See NOTE above
    nBufferItems++;
#    endif
    //--Get BYTES to process for in and out sequences
    size_t nBufferLeft = nBufferItems * sizeof(BufferT);
    size_t nSourceLeft = nSourceItems * sizeof(SourceT);

    //--Setup BYTE iterators to in and out sequences

    BufferT*       pBuf = pBuffer;
    const SourceT* pSrc = pSource;

    while (nSourceLeft > 0)
    {
        // Convert as much of the sequence as we can.
        SourceT* pIconvSrc   = (SourceT*)pSrc;
        BufferT* pIconvDest  = (BufferT*)pBuf;
        size_t   nbIconvSrc  = (size_t)nSourceLeft;
        size_t   nbIconvDest = (size_t)nBufferLeft;

        size_t nConv =
            iconv(convForward, (ICONV_SOURCE_TYPE**)&pIconvSrc, &nbIconvSrc, (char**)&pIconvDest, &nbIconvDest);

        if (nConv == -1)
        {
            // NOTE: On solaris sparc, iconv will attempt to NULL terminate the output,
            // so make sure that the buffer has space for a terminating NULL
            size_t nConv = // NOTE:
#    if SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST
                tss_ConvertOneCharacter<BufferT, SourceT>(
#    else
                tss_ConvertOneCharacter(
#    endif
                    convForward,
                    convReverse,         // On return, these addresses
                    (const char**)&pSrc, // are set for one past the last
                    &nSourceLeft,        // "item" converted successfully
                    (char**)&pBuf,
                    &nBufferLeft
#    if (!SUPPORTS_EXPLICIT_TEMPLATE_FUNC_INST)
                    ,
                    BufferT(),
                    SourceT()
#    endif
                );
        }
        else
        {
            pSrc        = pIconvSrc;
            pBuf        = pIconvDest;
            nSourceLeft = nbIconvSrc;
            nBufferLeft = nbIconvDest;
        }

        if (nConv == (size_t)-1) // Indidates Conversion Error!
        {
            if (nBufferLeft <= 0)
            {
                d.TraceDebug("No buffer space left\n");
                return 0;
            }
            // TODO:BAM -- handle other iconv errors!

            //--Must attempt Manual Conversion.
            ASSERT((size_t)pSrc % sizeof(SourceT) == 0);
            ASSERT((size_t)pBuf % sizeof(BufferT) == 0);

            d.TraceDebug("conversion failed: %s\n", strerror(errno));
            d.TraceDebug("manually converting %X...\n", tss::util::char_to_size(*pSrc));

            //-- Convert characters and reposition counters and pointers
            if (!ConvertByte.Convert(&pBuf, &nBufferLeft, &pSrc, &nSourceLeft))
                return 0;
        }
    }

    d.TraceDebug("buffer out: %s\n",
                 util_output_bytes((void*)pBuffer, nBufferItems * sizeof(BufferT) - nBufferLeft).c_str());
    return nBufferItems - (nBufferLeft / sizeof(BufferT));
}

} // namespace

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cIconvConverter
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

cIconvConverter::cIconvConverter()
{
    Init();
}

cIconvConverter::~cIconvConverter()
{
    // close conversion handles
    cIconvUtil::CloseHandle(icFromDb);
    cIconvUtil::CloseHandle(icToDb);
}

/* static */
bool cIconvConverter::Test()
{
    bool   testResult = true;
    cDebug d("cCodeConverter::Test()");

    //
    // try to get the current code page id
    //
    const char* pCP = NULL;
    if (!cIconvUtil::GetCodePageID(&pCP))
    {
        d.TraceDebug("No code page.\n");
        return false;
    }
    d.TraceDebug("Got code page %s.\n", pCP);


    //
    // find if converters are available
    //
    testResult &= cIconvUtil::TestConverter(pCP, cIconvUtil::GetIconvDbIdentifier());
    testResult &= cIconvUtil::TestConverter(cIconvUtil::GetIconvDbIdentifier(), pCP);
    return testResult;
}


void cIconvConverter::Init()
{
    // NOTE:BAM -- if the platform doesn't have a XXX to UCS2 conversion,
    //          you can make one with the genxlt command....
    icToDb   = cIconvUtil::OpenHandle(cIconvUtil::GetIconvDbIdentifier(), cIconvUtil::GetCodePageID());
    icFromDb = cIconvUtil::OpenHandle(cIconvUtil::GetCodePageID(), cIconvUtil::GetIconvDbIdentifier());
}

int cIconvConverter::Convert(ntmbs_t pbz, size_t nBytes, const_ntdbs_t pwz, size_t nChars)
{
    cDebug d("cIconvConverter::Convert( ntdbs_t -> ntmbs_t )");

    if (pbz == 0 || (pwz == 0 && nChars))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    //
    // reset converter
    //
    cIconvUtil::ResetConverter(icFromDb);

    //
    // do conversion
    //

    ByteConvert<mbchar_t, dbchar_t> bc;
    int                             nConverted = tss_Converter(icFromDb, icToDb, pbz, nBytes, pwz, nChars, bc);
    if (!nConverted)
        return tss_dispatch_errno(d);
    ASSERT(nConverted < nBytes);

    return nConverted;
}

int cIconvConverter::Convert(ntdbs_t pwz, size_t nch, const_ntmbs_t pbz, size_t nBytes)
{
    cDebug d("cIconvConverter::Convert( ntmbs to ntdbs )");

    if (pwz == 0 || (pbz == 0 && nBytes))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    //
    // reset converter
    //
    cIconvUtil::ResetConverter(icToDb);

    //
    // do conversion
    //
    ByteConvert<dbchar_t, mbchar_t> bc;
    int                             nConverted = tss_Converter(icToDb, icFromDb, pwz, nch, pbz, nBytes, bc);
    if (!nConverted)
        return tss_dispatch_errno(d);
    ASSERT(nConverted <= nch);

    return nConverted;
}


/*


//-- Converts one SourceT char to one BufferT char
//   checks to see that no OS-implementation-defined
//   conversion took place...
template< class BufferT, class SourceT >
int
tss_Convert(   iconv_t  dbConv,
               BufferT* pBuffer, size_t nBufferItems,
         const SourceT* pSource, size_t nSourceItems )
{
    //
    //  Perform first conversion
    //

    //-- Get BYTES to process for in and out sequences
    size_t nBufferLeft1 = nBufferItems * sizeof(BufferT);
    size_t nSourceLeft1 = nSourceItems * sizeof(SourceT);

    //-- Set up BYTE iterators to in and out sequences
    char* pBuf1 = (char*)( pBuffer + 0 );
    char* pSrc1 = (char*)( pSource + 0 );
    
    //-- NOTE: On solaris sparc, iconv will attempt to NULL terminate the output,
    //   so make sure that the buffer has space for a terminating NULL
    size_t nConv1 =                     // NOTE:
        ::iconv( dbConv,                    // On return, these addresses                    
            (ICONV_SOURCE_TYPE*) &pSrc1,     // are set for one past the last
            &nSourceLeft1,                   // "item" converted successfully
            &pBuf1, 
            &nBufferLeft1 );

    //-- Did conversion succeed?
    if( nConv1 != 1 )
    {
        d.tracedebug();
        return 0;
    }
    size_t nSrcBytesUsed1 = pSrc1 - (char*)pSource;
    size_t nBufBytesUsed1 = pBuf1 - (char*)pBuffer;
    
    //
    //  Make sure no implementation-defined conversion occured
    //  So see that the reverse conversion works out to the same source char
    //
    
    //-- Get BYTES to process for in and out sequences
    SourceT secondBuf[ MB_LEN_MAX ];
    size_t nBufferLeft2 = MB_LEN_MAX * sizeof( SourceT );
    size_t nSourceLeft2 = ((Buffer*)pBuf) - pBuffer;
    
    //-- Set up BYTE iterators to in and out sequences
    char* pBuf2 = (char*)&secondBuf[0];
    char* pSrc2 = (char*)pBuffer;
    size_t nConv2 =
        ::iconv( mbConv,                    // On return, these addresses                    
            (ICONV_SOURCE_TYPE*) &pSrc2,     // are set for one past the last
            &nSourceLeft2,                   // "item" converted successfully
            &pBuf2, 
            &nBufferLeft2 );
    
    //-- Did conversion succeed?
    if( nConv2 != 1 )
    {
        d.tracedebug();
        return 0;
    }
    size_t nSrcBytesUsed2 = pSrc2 - (char*)&secondBuf[0];
    if( ( nSrcBytesUsed1 != nSrcBytesUsed2 ) ||
        ( 0 != memcmp( &secondBuf[0], pBuffer, nSrcBytesUsed1 ) ) )
    {
        d.tracedebug();
        return 0;
    }

    return( nSrcBytesUsed1 and nBufBytesUsed1 );
}
*/


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cDoubleIconvConverter
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

cDoubleIconvConverter::cDoubleIconvConverter()
{
    Init();
}

cDoubleIconvConverter::~cDoubleIconvConverter()
{
    // close conversion handles
    cIconvUtil::CloseHandle(icMbToUTF8);
    cIconvUtil::CloseHandle(icUTF8ToDb);
    cIconvUtil::CloseHandle(icDbToUTF8);
    cIconvUtil::CloseHandle(icUTF8ToMb);
}

/* static */
bool cDoubleIconvConverter::Test()
{
    bool   testResult = true;
    cDebug d("cDoubleIconvConverter::Test()");

    //
    // try to get the current code page id
    //
    const char* pCP = NULL;
    if (!cIconvUtil::GetCodePageID(&pCP))
    {
        d.TraceDebug("No code page.\n");
        return false;
    }
    ASSERT(pCP && *pCP);
    d.TraceDebug("Got code page %s.\n", pCP);


    //
    // find if converters are available
    //
    testResult &= cIconvUtil::TestConverter(cIconvUtil::GetMiddleIdentifier(), pCP);
    testResult &= cIconvUtil::TestConverter(cIconvUtil::GetIconvDbIdentifier(), cIconvUtil::GetMiddleIdentifier());
    testResult &= cIconvUtil::TestConverter(cIconvUtil::GetMiddleIdentifier(), cIconvUtil::GetIconvDbIdentifier());
    testResult &= cIconvUtil::TestConverter(pCP, cIconvUtil::GetMiddleIdentifier());

    return testResult;
}


void cDoubleIconvConverter::Init()
{
    icMbToUTF8 = cIconvUtil::OpenHandle(cIconvUtil::GetMiddleIdentifier(), cIconvUtil::GetCodePageID());
    icUTF8ToDb = cIconvUtil::OpenHandle(cIconvUtil::GetIconvDbIdentifier(), cIconvUtil::GetMiddleIdentifier());
    icDbToUTF8 = cIconvUtil::OpenHandle(cIconvUtil::GetMiddleIdentifier(), cIconvUtil::GetIconvDbIdentifier());
    icUTF8ToMb = cIconvUtil::OpenHandle(cIconvUtil::GetCodePageID(), cIconvUtil::GetMiddleIdentifier());
}

int cDoubleIconvConverter::Convert(ntmbs_t pbz, size_t nBytes, const_ntdbs_t pwz, size_t nChars)
{
    cDebug d("cDoubleIconvConverter::Convert( ntdbs_t -> ntmbs_t )");

    //
    // initial checking
    //
    if (pbz == 0 || (pwz == 0 && nChars))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    //////////////////////////////////////////////////
    // do conversion to UTF8
    //////////////////////////////////////////////////
    //
    // reset first converter
    //
    cIconvUtil::ResetConverter(icDbToUTF8);

    //
    // create middle buffer
    //
    size_t  nBufBytes = nChars * MB_LEN_MAX;
    ntmbs_t pszBuffer = (ntmbs_t)::operator new(nBufBytes + 1);
    TW_UNIQUE_PTR<mbchar_t>        pBuf(pszBuffer);

    //
    // do first conversion
    //
    std::list<byte>                   lb; // buffer for invalid bytes
    ToUTF8Convert<mbchar_t, dbchar_t> tc(lb);
    int nConverted = tss_Converter(icDbToUTF8, icUTF8ToDb, pszBuffer, nBufBytes, pwz, nChars, tc);
    if (!nConverted)
        return tss_dispatch_errno(d);
    ASSERT(nConverted <= nBufBytes);

    //////////////////////////////////////////////////
    // do conversion to MB char
    //////////////////////////////////////////////////
    //
    // reset second converter
    //
    cIconvUtil::ResetConverter(icUTF8ToMb);

    //
    // do second conversion
    //
    FromUTF8Convert<mbchar_t, mbchar_t> fc(lb);
    nConverted = tss_Converter(icUTF8ToMb, icMbToUTF8, pbz, nBytes, pszBuffer, nConverted, fc);
    if (!nConverted)
        return tss_dispatch_errno(d);
    ASSERT(nConverted <= nBytes);

    //
    // return number of bytes converted
    //
    return nConverted;
}

int cDoubleIconvConverter::Convert(ntdbs_t pwz, size_t nch, const_ntmbs_t pbz, size_t nBytes)
{
    cDebug d("cDoubleIconvConverter::Convert( ntmbs to ntdbs )");

    if (pwz == 0 || (pbz == 0 && nBytes))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    //////////////////////////////////////////////////
    // do conversion to UTF-8 char
    //////////////////////////////////////////////////
    //
    // reset first converter
    //
    cIconvUtil::ResetConverter(icMbToUTF8);

    //
    // create middle buffer
    //
    size_t  nBufBytes = nBytes * MB_LEN_MAX;
    ntmbs_t pszBuffer = (ntmbs_t)::operator new(nBufBytes + 1);
    TW_UNIQUE_PTR<mbchar_t>        pBuf(pszBuffer);

    //
    // do first conversion
    //
    std::list<byte>                   lb; // buffer for invalid bytes
    ToUTF8Convert<mbchar_t, mbchar_t> tc(lb);
    int nConverted = tss_Converter(icMbToUTF8, icUTF8ToMb, pszBuffer, nBufBytes, pbz, nBytes, tc);
    if (!nConverted)
        return tss_dispatch_errno(d);
    ASSERT(nConverted <= nBufBytes);

    //////////////////////////////////////////////////
    // do conversion to DB char
    //////////////////////////////////////////////////
    //
    // reset second converter
    //
    cIconvUtil::ResetConverter(icUTF8ToDb);

    //
    // do second conversion
    //
    FromUTF8Convert<dbchar_t, mbchar_t> fc(lb);
    nConverted = tss_Converter(icUTF8ToDb, icDbToUTF8, pwz, nch, pszBuffer, nConverted, fc);
    if (!nConverted)
        return tss_dispatch_errno(d);
    ASSERT(nConverted <= nch);

    return nConverted;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cIconvUtil
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* static */
iconv_t cIconvUtil::OpenHandle(const char* pTo, const char* pFrom)
{
    cDebug  d("cIconvUtil::OpenHandle");
    iconv_t ic;

    ic = iconv_open(pTo, pFrom);
    if (ic == (iconv_t)-1)
        throw eConverterUnsupportedConversion();

    d.TraceDebug("opened %s to %s conversion\n", pFrom, pTo);
    return ic;
}

/* static */
bool cIconvUtil::GetCodePageID(const char** ppCP)
{
    *ppCP = nl_langinfo(CODESET);

    return (*ppCP && **ppCP);
}

/* static */
const char* cIconvUtil::GetCodePageID()
{
    const char* pCurCodePage;

    if (!GetCodePageID(&pCurCodePage))
        return NULL;

    return pCurCodePage;
}

/* static */
const char* cIconvUtil::GetIconvDbIdentifier()
{
#    ifdef WORDS_BIGENDIAN
    return "UTF-16BE";
#    else
    return "UTF-16LE";
#    endif
}

/* static */
const char* cIconvUtil::GetMiddleIdentifier()
{
    return "UTF-8";
}


/* static */
void cIconvUtil::ResetConverter(iconv_t ic)
{
    char*  p = 0;
    size_t s = 0;

    size_t i = iconv(ic, (ICONV_SOURCE_TYPE**)&p, &s, &p, &s);
    if (i == (size_t)-1)
    {
        ASSERT(false);
        throw eConverterReset();
    }
}

/* static */
void cIconvUtil::CloseHandle(iconv_t ic)
{
    int ret = iconv_close(ic);
    ASSERT(ret != -1);
}

/* static */
bool cIconvUtil::TestConverter(const char* pTo, const char* pFrom)
{
    if (!pTo || !pFrom)
        return false;

    cDebug d("cIconvUtil::TestConverter()");

    iconv_t i = iconv_open(pTo, pFrom);
    if ((iconv_t)-1 == i)
    {
        d.TraceDebug("No %s to %s conversion.\n", pFrom, pTo);
        tss_dispatch_iconvopen_error();
        return false;
    }
    else
    {
        d.TraceDebug("Valid conversion for %s to %s.\n", pFrom, pTo);
        cIconvUtil::CloseHandle(i);
        return true;
    }
}

#endif //TSS_USE_ICONV_CCONV16


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cWcharIs32BitUcs2Converterer
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace /*Unique*/
{
//-- Ensures 1-to-1 mb to wide character mapping by doing a reverse conversion
//   and comparing the results
int tss_mbtowc(wchar_t* pwch, const mbchar_t* pch, size_t nBytes)
{
    // convert forward
    int nNarrow = ::mbtowc(pwch, pch, nBytes);
    if (nNarrow == -1)
        return -1;

    // convert backwards
    mbchar_t ach[MB_LEN_MAX];
    int      nWide = ::wctomb(ach, *pwch);
    if (nWide == -1)
        return -1;

    // compare...
    if ((nNarrow > (int)nBytes) || (0 != memcmp(ach, pch, nNarrow)))
        return -1;

    // success!
    return nNarrow;
}

//-- Ensures 1-to-1 mb to wide character mapping by doing a reverse conversion
//   and comparing the results
int tss_wctomb(mbchar_t* pch, wchar_t wch)
{
    // convert forward
    int nWide = ::wctomb(pch, wch);
    if (nWide == -1)
        return -1;

    // convert backwards
    wchar_t wchTest;
    int     nNarrow = ::mbtowc(&wchTest, pch, MB_CUR_MAX);
    if (nNarrow == -1)
        return -1;

    // compare...
    if (wchTest != wch)
        return -1;

    // success!
    return nWide;
}

int tss_wcstombs(ntmbs_t pbz, const_ntwcs_t pwz, size_t nCount)
{
    cDebug d("tss_wcstombs");

    size_t nConv;
    size_t N;
    for (nConv = 0, N = 0; *pwz; ++pwz, pbz += N, ++nConv)
    {
        N = tss_wctomb(pbz, *pwz);
        if (N == (size_t)-1)
        {
            *pbz = cConvertUtil::ConvertNonChar((dbchar_t)*pwz);
            N    = 1;
        }
    }

    return (int)nConv;
}


int tss_mbstowcs(ntwcs_t pwz, const_ntmbs_t pbz, size_t nBytes)
{
    cDebug d("tss_mbstowcs");
    if (!pbz)
        return 0;
    
    size_t        N;
    size_t        nConv;
    const_ntmbs_t end = &pbz[nBytes];
    for (nConv = 0, N = 0; *pbz; pbz += N, ++pwz, ++nConv)
    {
        N = tss_mbtowc(pwz, pbz, end - pbz);
        if (N == (size_t)-1)
        {
            d.TraceDebug("manually converting %X...\n", cConvertUtil::ConvertNonChar(*pbz));

            *pwz = (wchar_t)cConvertUtil::ConvertNonChar(*pbz);
            N    = 1;
        }
    }

    return (int)nConv;
}
} // namespace

#if WCHAR_IS_32_BITS

int cWcharIs32BitUcs2Converterer::Convert(ntmbs_t pbz, size_t nBytes, const_ntdbs_t pwz, size_t nCount)
{
    if (pbz == 0 || (pwz == 0 && nCount))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    // cast our dbchar_t to wchar_t's first
    std::vector<wchar_t> buf(nCount + 1);
    wchar_t*             pwzBuffer = &buf[0];

    for (size_t n = 0; n < nCount; n++)
        pwzBuffer[n] = pwz[n];
    pwzBuffer[nCount] = 0x00; // null terminate

    int nConv = tss_wcstombs(pbz, pwzBuffer, nBytes);
    if ((size_t)nConv == (size_t)-1)
    {
        cDebug d("cWcharIs32BitUcs2Converterer::Convert( ntdbs to ntmbs )");
        char*  psz = new char[nCount * 2];
        char*  at  = psz;
        while (*pwzBuffer)
            *at++ = *pwzBuffer++;
        *at = 0x00;
        d.TraceDebug("Invalid Input: [%s]\n", psz);
        d.TraceDebug("%s\n", util_output_bytes((void*)pwz, nCount * sizeof(wchar_t)).c_str());

        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_BADCHAR));
    }

    return nConv;
}

int cWcharIs32BitUcs2Converterer::Convert(ntdbs_t pwz, size_t nCount, const_ntmbs_t pbz, size_t nBytes)
{
    cDebug d("cWcharIs32BitUcs2Converterer::Convert( ntmbs to ntdbs )");

    // Validate Input
    if (pwz == 0 || (pbz == 0 && nBytes))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    // mb to wc to a buffer of wide chars then....
    std::vector<wchar_t> buf(nCount);
    wchar_t*             pwzBuffer = &buf[0];

    int nConv = tss_mbstowcs(pwzBuffer, pbz, nCount);
    if (nConv == -1)
    {
        d.TraceDebug("Invalid Input: [%s]\n", pbz);
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_BADCHAR));
    }

    // ...cast those chars to dbchar_ts
    for (size_t at = 0; at < (size_t)nConv; ++at, ++pwzBuffer)
    {
        wchar_t wch = *pwzBuffer;
        if (wch > 0xFFFF)
        {
            d.TraceDebug("found wchar_T > 0xFFFF: %X\n", wch);
            throw eConverterFatal(_T("Cannot truncate wchar_t to dbchar_t"));
        }

        pwz[at] = (dbchar_t)wch;
    }
    pwz[nConv] = 0x00;

    return nConv;
}


#endif // WCHAR_IS_32_BITS

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cWcharIs16BitUcs2Converterer
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if WCHAR_IS_16_BITS

int cWcharIs16BitUcs2Converterer::Convert(ntmbs_t pbz, size_t nbMB, const_ntdbs_t pwz, size_t nch)
{
    // Validate Input
    if (pbz == 0 || (pwz == 0 && nch))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    int nConverted = tss_wcstombs(pbz, pwz, nbMB);
    if (nConverted == -1)
    {
        cDebug  d("cWcharIs16BitUcs2Converterer[ntdbs->ntmbs]");
        ntmbs_t psz = new char[nch * 2];
        ntmbs_t at  = psz;
        while (*pwz)
            *at++ = (mbchar_t)*pwz++;
        *at = 0x00;
        d.TraceDebug("Invalid Input: [%s]\n", psz);
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_BADCHAR));
    }

    return nConverted;
}

int cWcharIs16BitUcs2Converterer::Convert(ntdbs_t pwz, size_t nch, const_ntmbs_t pbz, size_t nBytes)
{
    // Validate Input
    if (pbz == 0 || (pwz == 0 && nch))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    int nConverted = tss_mbstowcs(pwz, pbz, nch);
    if (nConverted == -1)
    {
        cDebug d("cWcharIs16BitUcs2Converterer::Convert( ntmbs to ntdbs )");
        d.TraceDebug("Invalid Input: [%s]\n", pbz);
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_BADCHAR));
    }

    return nConverted;
}

#endif // WCHAR_IS_16_BITS


int cGoodEnoughConverterer::Convert(ntmbs_t pbz, size_t nBytes, const_ntdbs_t pwz, size_t nCount)
{
    if (pbz == 0 || (pwz == 0 && nCount))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    char* at = pbz;

    if (pwz)
    {
        const dbchar_t* dat = pwz;
        while (*dat)
        {
            if (*dat > 0xFF)
            {
                *at = cConvertUtil::ConvertNonChar(*dat);
            }
            else
            {
                *at = (char)*dat;
            }

            at++;
            dat++;
        }
    }

    *at = 0x00;

    return ((size_t)at - (size_t)pbz);
}

int cGoodEnoughConverterer::Convert(ntdbs_t pwz, size_t nCount, const_ntmbs_t pbz, size_t nBytes)
{
    // Validate Input
    if (pwz == 0 || (pbz == 0 && nBytes))
        throw eConverterFatal(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    dbchar_t* dat = pwz;

    if (pbz)
    {
        const char* at = pbz;
        while (*at)
        {
            if ((unsigned char)*at > (unsigned char)0x7Fu)
            {
                *dat = cConvertUtil::ConvertNonChar(*at);
            }
            else
            {
                *dat = (uint16_t)(unsigned char)*at;
            }

            dat++;
            at++;
        }
    }
    *dat = 0x0000;

    return (((size_t)dat - (size_t)pwz) / sizeof(dbchar_t));
}

dbchar_t cConvertUtil::ConvertNonChar(mbchar_t ch)
{
    cDebug d("cConvertUtil::ConvertNonChar( mbchar_t to dbchar_t )");

    if (!ValueInReservedRange(ch))
    {
        d.TraceDebug("Invalid Input: [%X]\n", ch);
        throw eConverterFatal(_T("mbchar_t is not high ascii!"));
    }

    dbchar_t wch = (dbchar_t)(tss::util::char_to_size(ch) + TSS_UCS2_RESERVED_START);

    d.TraceDebug("Converted 0x%08X to 0x%08X\n", tss::util::char_to_size(ch), tss::util::char_to_size(wch));

    return (wch);
}

mbchar_t cConvertUtil::ConvertNonChar(dbchar_t wch)
{
    cDebug d("cConvertUtil::ConvertNonChar( dbchar_t to mbchar_t )");

    if (!ValueInReservedRange(wch))
    {
        d.TraceDebug("Invalid Input: [%X]\n", wch);
        throw eConverterFatal(_T("Cannot truncate dbchar_t to mbchar_t"));
    }

    mbchar_t ch = (mbchar_t)(wch - TSS_UCS2_RESERVED_START);

    d.TraceDebug("Converted 0x%08X to 0x%08X\n", tss::util::char_to_size(wch), tss::util::char_to_size(ch));

    return (ch);
}

bool cConvertUtil::ValueInReservedRange(dbchar_t wch)
{
    size_t s = tss::util::char_to_size(wch);
    return ((s >= TSS_UCS2_RESERVED_START) && (s <= TSS_UCS2_RESERVED_END));
}

bool cConvertUtil::ValueInReservedRange(mbchar_t ch)
{
    size_t s = tss::util::char_to_size(ch);
    return ((s >= TSS_HIGH_ASCII_START) && (s <= TSS_HIGH_ASCII_END));
}
