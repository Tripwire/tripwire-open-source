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
// twlocale.h
//

#ifndef __TWLOCALE_H
#define __TWLOCALE_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __TYPES_H
#include "types.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

TSS_EXCEPTION(eTWLocale, eError);
TSS_EXCEPTION(eTWLocaleBadNumFormat, eError);

///////////////////////////////////////////////
// cTWLocale
///////////////////////////////////////////////
//
//  contains:
//      Utility functions that are wrappers for
//      facet functionality.
//
///////////////////////////////////////////////
class cTWLocale
{
public:
    static void InitGlobalLocale();
    // initializes global locale to the default locale

    //
    // basic functionality
    //
    static TSTRING& FormatNumber(int32_t   i, TSTRING& strBuf);
    static TSTRING& FormatNumber(int64_t   i, TSTRING& strBuf);
    static TSTRING& FormatNumber(uint32_t ui, TSTRING& strBuf);
    static TSTRING& FormatNumber(uint64_t ui, TSTRING& strBuf);
    static TSTRING& FormatNumber(double d, TSTRING& strBuf);
    // returns the locale-specific representation of the given cardinal number
    /*
    static TSTRING  FormatNumberClassic( int32_t i );
    static int32_t    FormatNumberClassic( const TSTRING& s );
        // returns the C-locale representation of the given cardinal number
*/
    //      disabled this since nobody's using it
    //    static TSTRING  FormatNumberAsHex( int32_t x );
    // locale-independant

    static TSTRING& FormatTime(int64_t t, TSTRING& strBuf);
    // returns the local time and date formatted according to the current locale.
    // t is the number of seconds elapsed since midnight (00:00:00),
    // January 1, 1970, coordinated universal time
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

#if !ARCHAIC_STL
namespace tss
{

////////////////////////////////////////////////
// GetFacet
//
// Abstracts std::use_facet since Win32 and KAI
// each implement it in a non-standard way.
////////////////////////////////////////////////
template<class FacetT> inline const FacetT& GetFacet(const std::locale& l, const FacetT* pf)
{
#if USE_STD_CPP_LOCALE_WORKAROUND
    return std::use_facet(l, pf);
#else
    return std::use_facet<FacetT>(l);
    (void)pf; // This is C++ standard
#endif
}
} // namespace tss
#endif

#endif //__TWLOCALE_H
