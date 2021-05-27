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
// twlocale.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdcore.h"
#include "twlocale.h"
#include "corestrings.h"

//TODO:mdb -- for some reason, this isn't being included on gcc/stlport
//            How is this being included for other platforms (I can't find
//            references to locale.h anywhere in the code!)
//
#if HAVE_GCC
#include <locale.h>
#endif //HAVE_GCC

#if IS_AROS
#   undef HAVE_TZSET
#endif

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================


//=========================================================================
// UTIL FUNCTION PROTOTYPES
//=========================================================================
static TSTRING& util_FormatTimeC(struct tm* ptm, TSTRING& strBuf);
static TSTRING& util_FormatTime(struct tm* ptm, TSTRING& strBuf);

#if !USES_CLIB_DATE_FUNCTION
static TSTRING& util_FormatTimeCPlusPlus(struct tm* ptm, TSTRING& strBuf);
#endif

//=========================================================================
// PUBLIC METHOD CODE
//=========================================================================

void cTWLocale::InitGlobalLocale()
{
    cDebug d("cTWLocale::InitGlobalLocale");

    d.TraceDetail("Attempting to set the program locale from the"
                  "default \"C\" locale to the system-default locale.");

#if USE_CLIB_LOCALE
    char* pchLocale = setlocale(LC_ALL, "");

    d.TraceDetail("C++ locale is incomplete or unavailable with this compiler, so"
                  "only changing the C-language locale.  Some C++ locale-specific functionality"
                  "may not function properly.");

    if (pchLocale)
    {
        d.TraceDebug("locale changed to the system-default locale (non-C++): <%s>\n", pchLocale);
    }
    else
    {
        d.TraceDebug("Error: unable to change locale to the system-default.\n"
                     "It is possible that there is no other locale than\n"
                     "the \"C\" locale on this platform.\n");
    }
#else
    std::locale l("");
    std::locale::global(l);
    d.TraceDebug("locale changed to the system default std::locale (C++): <%s>\n", l.name().c_str());
#endif
}

/*
TSTRING cTWLocale::FormatNumberAsHex( int32_t i )
{
    //
    // preconditions
    //
    ASSERT( sizeof( long ) >= sizeof( int32_t ) ); // must be able to cast to 'long'

    //
    // convert long to a string
    //
    TOSTRINGSTREAM sstr;
    tss_classic_locale(sstr);
    sstr.setf( std::ios::hex, std::ios::basefield );
    const std::num_put< TCHAR > *pnp = 0, &np = tss::GetFacet( sstr.getloc(), pnp );
    np.put( sstr, sstr, sstr.fill(), (long)i ); 

    //
    // return it
    //
    return( sstr.str() );
}
*/

template<class numT, class CharT> class cFormatNumberUtil
{
public:
    // TODO:BAM -- these functions should really maybe take a stringstream that has been
    //  already set up for formatting (basefield, locale, etc.)   that way we can merge
    //. the FormatNumberAsHex function as well
    //
    //=============================================================================
    // template< class numT, class CharT >
    // numT
    // cFormatNumberUtil::FormatNumber( const std::basic_string< CharT >& s, bool fCStyleFormatting = false  )
    //-----------------------------------------------------------------------------
    // EFFECTS: Does all actual formatting for FormatNumber methods
    static numT Format(const std::basic_string<CharT>& s, bool fCStyleFormatting)
    {
      
#if !ARCHAIC_STL      
        static const std::num_get<CharT>* png;
        std::ios::iostate            state;
        numT                              n;

	std::basic_istringstream<CharT>   ss(s);

        if (fCStyleFormatting)
	    tss_classic_locale(ss);

        tss::GetFacet(ss.getloc(), png).get(ss, std::istreambuf_iterator<CharT>(), ss, state, n);


        if ((state & std::ios::failbit) != 0)
            throw eTWLocaleBadNumFormat();

        return (n);
#else
	return atoi(s.c_str());
#endif
    }

    //=============================================================================
    // std::basic_string< CharT >&
    // Format( numT n, std::basic_string< CharT >& sBuf )
    //-----------------------------------------------------------------------------
    // EFFECTS: Does all actual formatting for FormatNumber methods
    //
    static std::basic_string<CharT>& Format(numT n, std::basic_string<CharT>& sBuf, bool fCStyleFormatting = false)
    {
#if !ARCHAIC_STL      
        static const std::num_put<CharT>* pnp;

        std::basic_ostringstream<CharT>   ss;

        if (fCStyleFormatting)
	    tss_classic_locale(ss);

        tss::GetFacet(ss.getloc(), pnp).put(ss, ss, ss.fill(), n);

#else
	strstream ss;
	ss << n;
#endif
        sBuf = ss.str();
        return (sBuf);	
    }
};

/*
TSTRING cTWLocale::FormatNumberClassic( int32_t i )
{
    TSTRING s;
    return cFormatNumberUtil< long, TCHAR >::Format( i, s, true );
}

int32_t cTWLocale::FormatNumberClassic( const TSTRING& s )
{
    return cFormatNumberUtil< long, TCHAR >::Format( s, true );
}
*/

TSTRING& cTWLocale::FormatNumber(uint64_t ui, TSTRING& strBuf)
{
    // try to use the int64_t version
    if (ui <= (uint64_t)TSS_INT64_MAX)
        return (FormatNumber((int64_t)ui, strBuf));
    else
    {
        ASSERT(std::numeric_limits<double>::max() >= TSS_UINT64_MAX);
        return (cFormatNumberUtil<double, TCHAR>::Format((double)ui, strBuf));
    }
}

TSTRING& cTWLocale::FormatNumber(int64_t i, TSTRING& strBuf)
{
    // try to use the int32_t version
    if (i <= (int64_t)TSS_INT32_MAX)
        return (FormatNumber((int32_t)i, strBuf));
    else
    {
        ASSERT(std::numeric_limits<double>::max() >= TSS_INT64_MAX);
        return (cFormatNumberUtil<double, TCHAR>::Format((double)i, strBuf));
    }
}

TSTRING& cTWLocale::FormatNumber(uint32_t ui, TSTRING& strBuf)
{
    ASSERT(sizeof(unsigned long) >= sizeof(uint32_t)); // must be able to cast to 'ulong'
    return (cFormatNumberUtil<unsigned long, TCHAR>::Format((unsigned long)ui, strBuf));
}

TSTRING& cTWLocale::FormatNumber(int32_t i, TSTRING& strBuf)
{
    ASSERT(sizeof(long) >= sizeof(int32_t)); // must be able to cast to 'long'
    return (cFormatNumberUtil<long, TCHAR>::Format((long)i, strBuf));
}

TSTRING& cTWLocale::FormatTime(int64_t t, TSTRING& strBuf)
{
    // clear return string
    strBuf.erase();
#if HAVE_TZSET
    tzset();
#endif
    time_t     tmpTime = t;
    struct tm* ptm     = localtime(&tmpTime);
    if (ptm)
    {
        util_FormatTime(ptm, strBuf);
    }
    else
    {
        strBuf = TSS_GetString(cCore, core::STR_UNKNOWN_TIME);
    }

    return (strBuf);
}

//=========================================================================
// UTIL FUNCTION IMPLEMENTATION
//=========================================================================

TSTRING& util_FormatTime(struct tm* ptm, TSTRING& strBuf)
{
    ASSERT(ptm);

#if USES_CLIB_DATE_FUNCTION

    return util_FormatTimeC(ptm, strBuf);

#else

    return util_FormatTimeCPlusPlus(ptm, strBuf);

#endif
}


#if !USES_CLIB_DATE_FUNCTION
TSTRING& util_FormatTimeCPlusPlus(struct tm* ptm, TSTRING& strBuf)
{
    ASSERT(ptm);
    TOSTRINGSTREAM                     sstr;
    static const std::time_put<TCHAR>* ptp;

    //
    // format date
    //
#    if IS_MSVC // MSVC uses old put() signature which didn't have the fill character, (and uses proprietary '#')
    tss::GetFacet(sstr.getloc(), ptp).put(sstr, sstr, ptm, 'c', '#');
#    else
    tss::GetFacet(sstr.getloc(), ptp).put(sstr, sstr, sstr.fill(), ptm, 'c');
#    endif

    tss_stream_to_string(sstr, strBuf);
    return strBuf;
}
#endif


TSTRING& util_FormatTimeC(struct tm* ptm, TSTRING& strBuf)
{
    ASSERT(ptm);

    TCHAR achTimeBuf[256];

    /* XXX: This should check (#ifdef) for strftime - PH */
    size_t nbWritten = _tcsftime(achTimeBuf,
                                 countof(achTimeBuf),

#if IS_MSVC // MSVC uses proprietary '#'
                                 _T("%#c"),
#else
                                 _T("%c"),
#endif
                                 ptm);

    if (nbWritten)
        strBuf = achTimeBuf;
    else
        strBuf = TSS_GetString(cCore, core::STR_UNKNOWN_TIME);

    return strBuf;
}
