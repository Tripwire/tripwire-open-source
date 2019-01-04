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
// fs.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdfs.h"
#include "fs.h"
#include "fserrors.h"
#include "fsfactory.h"
#include "fco/genreswitcher.h"

//=========================================================================
// PUBLIC METHOD CODE
//=========================================================================


#if IS_AIX
// For some stupid linker reason, this object does not get constructed under
// AIX. The cheap fix is to move this call to here from fspropdisplayer.cpp
#include "fspropdisplayer.h"
IMPLEMENT_TYPEDSERIALIZABLE(cFSPropDisplayer, _T("FSPropDisplayer"), 0, 1);
#endif


TSS_ImplementPackage(cFS)

cFS::cFS()
    : fsFactory(), fsGenre(GenreID(), _T("FS"), _T("Unix File System"), &fsFactory)
{
    //
    // set up dependencies
    //
    // TODO:BAM -- TSS_Dependency( ??? );

    //
    // register genre
    //
    cGenreSwitcher::RegisterFlags flags = cGenreSwitcher::REGISTER_FLAGS_DEFAULT;

    // TODO:BAM -- we include FS in the Win32 build but we don't want
    //             it to be the default genre for that build
    //             Remove this ifdef when we remove FS from the Win32 build
    cGenreSwitcher::GetInstance()->RegisterGenre(&fsGenre, flags);

    // register errors...
    //
    TSS_REGISTER_PKG_ERRORS(fs)
}
