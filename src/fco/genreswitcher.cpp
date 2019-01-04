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
// genreswitcher.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdfco.h"
#include "genreswitcher.h"
#include "core/errorgeneral.h"
#include "genreinfo.h"
#include "core/errorutil.h"

#if HAVE_STRINGS_H // for strcasecmp on RTEMS
#include <strings.h>
#endif

//=========================================================================
// STATIC MEMBERS
//=========================================================================

//=========================================================================
// PUBLIC METHOD CODE
//=========================================================================

// use to get global instance of this class
/* static */ cGenreSwitcher* cGenreSwitcher::GetInstance()
{
    static cGenreSwitcher s_Instance;
    return &s_Instance;
}

cGenreSwitcher::cGenreSwitcher() : m_pDefaultGenre(NULL), m_curGenre(cGenre::GENRE_INVALID)
{
}

cGenreSwitcher::~cGenreSwitcher()
{
}

// set the current genre
// thows an eError if newGenre is invalid
void cGenreSwitcher::SelectGenre(cGenre::Genre g)
{
    iTWFactory::SetFactory(GetFactoryForGenre(g));
    m_curGenre = g;
}

cGenre::Genre cGenreSwitcher::CurrentGenre()
{
    return m_curGenre;
}

cGenre::Genre cGenreSwitcher::GetDefaultGenre()
{
    cGenre::Genre defGenre = cGenre::GENRE_INVALID;

    if (m_pDefaultGenre != NULL)
        defGenre = m_pDefaultGenre->m_ID;

    ASSERT(defGenre != cGenre::GENRE_INVALID);
    return defGenre;
}

void cGenreSwitcher::RegisterGenre(const cGenreInfo* pGI, RegisterFlags flags /* = REGISTER_FLAGS_NONE */)
{
    cDebug d("cGenreSwitcher::RegisterGenre");

    ASSERT(NULL != pGI);
    ASSERT(pGI->IsValid());

    d.TraceDebug(
        _T("ID: %x, long name: %s, short name: %s\n"), pGI->m_ID, pGI->m_sLongName.c_str(), pGI->m_sShortName.c_str());

    //
    // can only be one of each genre ID
    //
    if (m_vGenres.find(pGI) != m_vGenres.end())
    {
        // TODO: correct behavior?  Should this be allowable/ignored?
        THROW_INTERNAL("genreswitcher.cpp"); // Attempt to register duplicate genre ID.
    }

    //
    // save genre info
    //
    m_vGenres.push_back(pGI);

    //
    // set default genre if appropriate
    //
    if (flags & REGISTER_FLAGS_DEFAULT)
    {
        // can only be one default genre
        // TODO: should we allow more than one lib to vie for default genre on a platform?
        if (NULL != m_pDefaultGenre)
            THROW_INTERNAL("genreswitcher.cpp"); // "Two genres claim default genre status." );

        // save as default genre
        m_pDefaultGenre = pGI;
    }
}


bool cGenreSwitcher::IsGenreRegistered(cGenre::Genre g)
{
    return (m_vGenres.find(g) != m_vGenres.end());
}


bool cGenreSwitcher::IsGenreAppropriate(cGenre::Genre g)
{
    const uint32_t platformMask = cGenre::PLATFORM_MASK_UNIX;
    return ((platformMask & g) != 0);
}


iTWFactory* cGenreSwitcher::GetFactoryForGenre(cGenre::Genre g)
{
    cGenreInfoVec::const_iterator i = m_vGenres.find(g);

    if (i == m_vGenres.end())
    {
        ThrowAndAssert(INTERNAL_ERROR("Switch to invalid genre factory"));
    }

    ASSERT((*i)->m_pFactory != NULL);
    return ((*i)->m_pFactory);
}


// Convert an enum to a string.  Verbose flag specifies
// whether to return "NTReg" or "NT Registry".
const TCHAR* cGenreSwitcher::GenreToString(cGenre::Genre g, bool verbose)
{
    cGenreInfoVec::const_iterator i = m_vGenres.find(g);
    if (i != m_vGenres.end())
    {
        ASSERT((*i)->m_ID != cGenre::GENRE_INVALID);
        return (verbose ? (*i)->m_sLongName.c_str() : (*i)->m_sShortName.c_str());
    }

    // didn't recognize genre ID
    ASSERT(false);
    return (NULL);
}

// Narrow and wide character conversions.  These functions will be
// able to intelligently handle case sesitivity and verbose or narrow
// versions of the string.
// GENRE_INVALID will be returned on error.


cGenre::Genre cGenreSwitcher::StringToGenre(const TCHAR* sz)
{
    // must convert to TSTRING
    TSTRING sGenre(sz);

    cGenreInfoVec::const_iterator i = m_vGenres.find(sGenre);
    if (i != m_vGenres.end())
    {
        return ((*i)->m_ID);
    }

    return (cGenre::GENRE_INVALID);
}

/* TODO: This is old code - dmb
cGenre::Genre cGenreSwitcher::StringToGenre( const wchar_t* wsz )
{
    // must convert to TSTRING
    TSTRING sGenre = cStringUtil::WstrToTstr( wsz );

    cGenreInfoVec::const_iterator i = _vGenres.find( sGenre );
    if( i != _vGenres.end() )
    {
        return( (*i)->_ID );
    }

    return( cGenre::GENRE_INVALID );
}
*/

//=========================================================================
// PRIVATE METHOD CODE
//=========================================================================

// returns true if there is a cGenreInfo stored in _vGenres that has an ID which
// matches this pGI's ID
cGenreInfoVec::const_iterator cGenreInfoVec::find(cGenre::Genre g) const
{
    const_iterator i;
    for (i = begin(); i != end(); ++i)
    {
        ASSERT((*i)->m_ID != cGenre::GENRE_INVALID);

        if (g == (*i)->m_ID)
            return i;
    }

    return i;
}

cGenreInfoVec::const_iterator cGenreInfoVec::find(const cGenreInfo* pGI) const
{
    ASSERT(NULL != pGI);
    return (find(pGI->m_ID));
}

// sGenre can be either short or long name
cGenreInfoVec::const_iterator cGenreInfoVec::find(const TSTRING& sGenre) const
{
    const_iterator i;
    for (i = begin(); i != end(); ++i)
    {
        ASSERT((*i)->m_ID != cGenre::GENRE_INVALID);

        //
        // compare against long and short name
        //
        if (_tcsicmp(sGenre.c_str(), (*i)->m_sLongName.c_str()) == 0 ||
            _tcsicmp(sGenre.c_str(), (*i)->m_sShortName.c_str()) == 0)
            break;
    }

    return (i);
}
