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
// genreswitcher.h
//

#ifndef __GENRESWITCHER_H
#define __GENRESWITCHER_H

//=========================================================================
// INCLUDES
//=========================================================================

#include "fcogenre.h"
#include "genreinfo.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <vector>

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================


//-------------------------------------------------
//
// class cGenreInfoVec --
//
//-------------------------------------------------

class cGenreInfoVec : public std::vector<const cGenreInfo*>
{
public:
    //-------------------------------------------------------------
    // Util Functions
    //-------------------------------------------------------------
    const_iterator find(const cGenreInfo* pGI) const;
    // returns true if there is a cGenreInfo stored in m_vGenres that has an ID which
    // matches this pGI's ID
    const_iterator find(cGenre::Genre g) const;
    // finds by genre ID
    const_iterator find(const TSTRING& sGenre) const;
    // finds both long and short names
};

//-------------------------------------------------
//
// class cGenreSwitcher --
//
//-------------------------------------------------

// classes that register to the cGenreSwitcher are responsible for setting
// a default value.
// access through GetInstance()
class cGenreSwitcher
{
public:
    //-------------------------------------------------------------
    // Singleton Manipulation
    //-------------------------------------------------------------
    static cGenreSwitcher* GetInstance();

    //-------------------------------------------------------------
    // Genre Selection
    //-------------------------------------------------------------
    void SelectGenre(cGenre::Genre newGenre);
    // set the current genre
    // thows an eError if newGenre is invalid

    cGenre::Genre CurrentGenre();
    // returs the current genre, or cGenre::GENRE_INVALID if no genre
    // has be selected since construction

    iTWFactory* GetFactoryForGenre(cGenre::Genre);
    // returns the tw factory appropriate for the given genre
    // throws eError if genre is invalid.

    cGenre::Genre GetDefaultGenre();
    // returns ID of default registered genre

    //-------------------------------------------------------------
    // Genre Checking
    //-------------------------------------------------------------
    bool IsGenreAppropriate(cGenre::Genre g);
    // is this genre supposed to be used with this platform?
    bool IsGenreRegistered(cGenre::Genre g);
    // is this genre recognized?

    //-------------------------------------------------------------
    // Genre Name/ID Conversion
    //-------------------------------------------------------------
    const TCHAR* GenreToString(cGenre::Genre genre, bool verbose = false);
    // Convert an enum to a string.  Verbose flag specifies
    // whether to return "NTReg" or "NT Registry".
    // NULL will be returned on error.

    cGenre::Genre StringToGenre(const TCHAR* sz);
    // TODO: remove this - dmb cGenre::Genre StringToGenre( const wchar_t* wsz );
    // Narrow and wide character conversions.  These functions will be
    // able to intelligently handle case sesitivity and verbose or narrow
    // versions of the string.
    // GENRE_INVALID will be returned on error.

    //-------------------------------------------------------------
    // Genre Registration
    //-------------------------------------------------------------
    enum RegisterFlags
    {
        REGISTER_FLAGS_NONE    = 0x00,
        REGISTER_FLAGS_DEFAULT = 0x01
    };
    void RegisterGenre(const cGenreInfo* pGI, RegisterFlags flags = REGISTER_FLAGS_NONE);
    //  cGenreSwitcher does not own the cGenreInfo

    ~cGenreSwitcher();

private:
    //-------------------------------------------------------------
    // Ctor and Dtor -- only access to cGenreSwitcher is through GetInstance()
    //-------------------------------------------------------------
    cGenreSwitcher();

    //-------------------------------------------------------------
    // Private Data Members
    //-------------------------------------------------------------
    const cGenreInfo* m_pDefaultGenre; // points to the default registered genre
    cGenreInfoVec     m_vGenres;       // stores all registered genre information
    cGenre::Genre     m_curGenre;      // points to the current genre
};

#endif //__GENRESWITCHER_H
