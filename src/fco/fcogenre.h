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
// fcogenre.h
//

#ifndef __FCOGENRE_H
#define __FCOGENRE_H

//=========================================================================
// INCLUDES
//=========================================================================

#include "core/types.h"

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

//------------------------------------------------
// class cGenre --  encapsulates genre ID
//                  information.  Also a non-code
//                  registry for genre IDs.
//------------------------------------------------
class cGenre
{
public:
    typedef uint32_t Genre;


    //-----------------------------------------
    // Valid Platforms -- static and commpiled
    //-----------------------------------------
    enum Platform
    {
        PLATFORM_MASK_INVALID = 0x00000000,
        PLATFORM_MASK_NT      = 0x00010000,
        PLATFORM_MASK_UNIX    = 0x00020000
    };

    //--------------------------------------------------------------------
    // Valid Genres -- dynamic so not compiled except for 'invalid' value
    //--------------------------------------------------------------------
    enum ValidGenres
    {
        //---------------------------------------------------------------
        // this the non-compiled registry of genre IDs.
        // all genre IDs should be listed here to avoid any
        // duplicate IDs.
        //---------------------------------------------------------------

        /*
        GENRE_NTFS              = 0x00010001,
        GENRE_NTREG             = 0x00010002,
        GENRE_FS                = 0x00020001,
        */
        GENRE_INVALID = 0x00000000
    };

    //----------------------------------------
    // don't let C++ create default functions
    //----------------------------------------
    cGenre();
    ~cGenre();
    cGenre(const cGenre&);
    cGenre&       operator=(const cGenre&);
    cGenre*       operator&();
    const cGenre* operator&() const;

}; // eoc: cGenre

#endif // __FCOGENRE_H
