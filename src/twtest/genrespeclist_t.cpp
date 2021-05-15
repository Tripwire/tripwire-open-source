//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
// genrespeclist_t.cpp
//

#include "fco/stdfco.h"
#include "fco/genrespeclist.h"
#include "twtest/test.h"
#include "fco/fcospecimpl.h"
#include "fs/fs.h"

void TestGenreSpecList()
{
    cDebug d("TestGenreSpecList");
    d.TraceDebug("Entering...\n");

    // cGenreSpecListPair
    cGenreSpecListPair gslPair;

    TEST(gslPair.GetGenre() == cGenre::GENRE_INVALID);

    gslPair.SetGenre(cFS::GenreID());
    TEST(gslPair.GetGenre() == cFS::GenreID());

    cFCOSpecList  speclist;
    cFCOSpecImpl* fsSpec = new cFCOSpecImpl(_T("test fsspce name"), NULL);

    speclist.Add(fsSpec);
    fsSpec->Release();

    gslPair.SetSpecList(speclist);
    TEST(gslPair.GetSpecList().Lookup(fsSpec)->GetName() == fsSpec->GetName());

    cGenreSpecListPair gslPair2(gslPair);
    TEST(gslPair.GetGenre() == gslPair2.GetGenre());
    TEST(gslPair.GetSpecList().Lookup(fsSpec)->GetName() == gslPair2.GetSpecList().Lookup(fsSpec)->GetName());

    cGenreSpecListPair gslPair3;
    gslPair3 = gslPair;
    TEST(gslPair.GetGenre() == gslPair3.GetGenre());
    TEST(gslPair.GetSpecList().Lookup(fsSpec)->GetName() == gslPair3.GetSpecList().Lookup(fsSpec)->GetName());

    // cGenreSpecListVector
    cGenreSpecListVector gslVector;
    gslVector.push_back(gslPair);
    gslVector.push_back(gslPair2);
    gslVector.push_back(gslPair3);

    TEST(gslVector[0].GetGenre() == gslPair.GetGenre());
    TEST(gslVector[0].GetSpecList().Lookup(fsSpec) != 0);
    TEST(gslVector[0].GetSpecList().Lookup(fsSpec)->GetName() == gslPair.GetSpecList().Lookup(fsSpec)->GetName());

    TEST(gslVector[1].GetGenre() == gslPair2.GetGenre());
    TEST(gslVector[1].GetSpecList().Lookup(fsSpec) != 0);
    TEST(gslVector[1].GetSpecList().Lookup(fsSpec)->GetName() == gslPair2.GetSpecList().Lookup(fsSpec)->GetName());

    TEST(gslVector[2].GetGenre() == gslPair3.GetGenre());
    TEST(gslVector[2].GetSpecList().Lookup(fsSpec) != 0);
    TEST(gslVector[2].GetSpecList().Lookup(fsSpec)->GetName() == gslPair3.GetSpecList().Lookup(fsSpec)->GetName());

    d.TraceDebug("All tests passed.\n");
}


void RegisterSuite_GenreSpecList()
{
    RegisterTest("GenreSpecList", "Basic", TestGenreSpecList);
}
