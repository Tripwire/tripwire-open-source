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
* Name....: ntmbs.cpp
* Date....: 08/31/99
* Creator.: rdifalco
*
* Routines to make NTMBS processing easier.
*/

#include "stdcore.h" // for: pch
#include "ntmbs.h"   // for: These Declarations


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module-wide Helpers
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef TSS_Raise // Should add file and line info in DEBUG mode!!!!
#    define TSS_Raise(Xcpt, pkg, ids) throw Xcpt(TSS_GetString(pkg, ids))

#endif //TSS_Raise


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// NTMBS Utils -- Definition
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if 0
/*
* TSS::mbsdec
*
*   ntmbs_t mbsdec( const_ntmbs_t beg, const_ntmbs_t psz );
*
* REQUIRES:
*   const_ntmbs_t beg = pointer to beginning of ntmbs
*   const_ntmbs_t psz = current mbchar_t pointer (legal MBCS boundary)
*   
* EFFECTS:
*   Move the supplied string pointer backwards by one character.
*   NTMBS characters are handled correctly.
*   
* POSTCONDITIONS:
*   Returns non-const pointer after moving it.
*   Returns NULL if beg >= psz.
*/
ntmbs_t
tss::mbsdec( const_ntmbs_t beg, const_ntmbs_t curr )
{
    cDebug dbg( "tss::mbsdec - " );
    dbg.TraceDebug( 
        "It is strongly reccommended you do not "
        "iterate an NTMBS backwards!\n" );

    if ( curr <= beg )
    {
        TSS_Raise( 
            eCharacterEncoding,
            cCore,
            core::STR_ERR_UNDERFLOW );
    }

    // Keep going back while invalid, find first valid.
    const_ntmbs_t prev = curr - 1;
    for ( ; prev > beg && ::mblen( (char*)prev, curr - prev ) == -1; --prev );

    if ( (size_t)(curr - prev) > MB_CUR_MAX )
    {
        prev = (curr - 1);

        dbg.TraceDebug( "Character Fault:%c\n", *prev );

        if ( (unsigned char)*prev > 0x7F )
            dbg.TraceDebug( "Normalizing.\n" );

        else
        {
            dbg.TraceDebug( "Incorrectly Formed, Cannot Normalize!\n" );
            TSS_Raise(
                eCharacterEncoding,
                cCore,
                core::STR_ERR_BADCHAR );
        }
    }

    return (ntmbs_t)prev;
}
#endif

/*
* TSS::mbsinc
*
*   ntmbs_t mbsinc( const_ntmbs_t psz );
*
* REQUIRES:
*   const_ntmbs_t psz = current mbchar_t pointer (legal MBCS boundary)
*   
* EFFECTS:
*   Move the supplied string pointer forward by one character.
*   NTMBS characters are handled correctly.
*   
* POSTCONDITIONS:
*   Returns non-const pointer after moving it.
*/
ntmbs_t tss::mbsinc(const_ntmbs_t psz)
{
    cDebug dbg("tss::mbsinc -");

    if (psz == 0)
        throw eCharacter(TSS_GetString(cCore, core::STR_ERR_ISNULL));

    int nBytes = ::mblen((char*)psz, MB_CUR_MAX);
    if (nBytes == -1)
    {
        dbg.TraceDebug("Character Fault: %c\n", *psz);
        if ((unsigned char)*psz > 0x7F)
        {
            dbg.TraceDebug("Normalizing.\n");
            return (((ntmbs_t)psz) + 1);
        }

        dbg.TraceDebug("Incorrectly Formed, Cannot Normalize!\n");
        TSS_Raise(eCharacterEncoding, cCore, core::STR_ERR_BADCHAR);
    }

    return (((ntmbs_t)psz) + nBytes);
}


/*
* TSS::mbsbytes
*
*   size_t mbsbytes( const_ntmbs_t psz, size_t nCount );
*
* REQUIRES:
*   const_ntmbs_t psz = current mbchar_t pointer (legal MBCS boundary)
*   size_t nCount = the number of character to calculate bytes for
*   
* EFFECTS:
*   Calculates the number of bytes in a character range:
*
*       while ( psz += mblen( psz ) <= nCount );
*   
* POSTCONDITIONS:
*   Returns size_t value indicating bytes in the range of [0,nCount).
*/
size_t tss::mbsbytes(const_ntmbs_t psz, size_t nCount)
{
    const_ntmbs_t at = psz;
    while (nCount--)
        at = tss::mbsinc(at);

    return (size_t)((char*)at - (char*)psz);
}

/// To Null terminator

size_t tss::mbsbytes(const_ntmbs_t psz)
{
    if (psz == 0)
        throw eCharacter(TSS_GetString(cCore, core::STR_ERR_ISNULL));


    const_ntmbs_t at;
    for (at = psz; *at; ++at)
        ;
    return (size_t)((char*)at - (char*)psz);
}


/*
* TSS::mbscount
*
*   size_t mbscount( const_ntmbs_t psz, size_t nBytes );
*
* REQUIRES:
*   const_ntmbs_t psz = current mbchar_t pointer (legal MBCS boundary)
*   size_t nBytes = the end of a byte range begining at psz + 0.
*   
* EFFECTS:
*   Calculate the number of legal MB characters in the specified byte range.
*
*       while ( psz += mblen( psz ) <= nCount );
*   
* POSTCONDITIONS:
*   Returns size_t value indicating characters in the range of [psz + 0, psz + nBytes).
*/
size_t tss_mbscount(const_ntmbs_t psz, size_t nBytes)
{
    size_t        nCount = 0;
    const_ntmbs_t at     = psz;
    const_ntmbs_t end    = psz + nBytes;
    for (; at < end; nCount++, at = tss::mbsinc(at))
        ;
    return nCount;
}

/// To Null terminator

size_t tss::mbscount(const_ntmbs_t psz)
{
    size_t nCount = 0;
    for (; *psz; psz = tss::mbsinc(psz), nCount++)
        ;
    return nCount;
}
