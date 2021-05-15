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
//
// Name....: fconametranslator_t.cpp
// Date....: 08/3/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "fco/stdfco.h"
#include "fco/fconametranslator.h"
#include "fco/genreswitcher.h"
#include "fco/fconame.h"
#include "twtest/test.h"

void TestName(const TCHAR* pchName, const TCHAR* pchGenre);
void TestUnprintable(const TCHAR* pchName, const TCHAR* pchGenre);


void TestFCONameTranslator()
{
    TestName(_T("/not_quoted/tmp"), _T("FS"));
    TestName(_T("/\"quoted\"/tmp"), _T("FS"));
    TestName(_T("/tmp/\"freak\x08y\""), _T("FS"));
    TestUnprintable(_T("c:\\\"more freak\xFFy\""), _T("FS"));
}


void TestName(const TCHAR* pchName, const TCHAR* pchGenre)
{
    //
    // set up name translator
    //
    iFCONameTranslator* mpCurNT = NULL;
    cGenre::Genre       genre   = cGenreSwitcher::GetInstance()->StringToGenre(pchGenre);
    TEST(genre != cGenre::GENRE_INVALID);
    mpCurNT = cGenreSwitcher::GetInstance()->GetFactoryForGenre(genre)->GetNameTranslator();

    //
    // encode name
    //
    cFCOName fcoName(pchName);
    TSTRING  strName = mpCurNT->ToStringDisplay(fcoName, true);
    TCOUT << _T("from: <") << fcoName.AsString() << _T("> to:") << strName;

    //
    // unencode name
    //
    cFCOName fcoNameNew;
    TEST(mpCurNT->DisplayStringToFCOName(strName, fcoNameNew));
    TCOUT << _T("> back to: <") << fcoNameNew.AsString() << _T(">") << std::endl;

    //
    // check result
    //
    TEST(fcoNameNew == fcoName);
}


void TestUnprintable(const TCHAR* pchName, const TCHAR* pchGenre)
{
    //
    // set up name translator
    //
    iFCONameTranslator* mpCurNT = NULL;
    cGenre::Genre       genre   = cGenreSwitcher::GetInstance()->StringToGenre(pchGenre);
    TEST(genre != cGenre::GENRE_INVALID);
    mpCurNT = cGenreSwitcher::GetInstance()->GetFactoryForGenre(genre)->GetNameTranslator();

    //
    // encode name
    //
    cFCOName fcoName(pchName);
    TSTRING  strName = mpCurNT->ToStringDisplay(fcoName, true);
    TCOUT << _T("To:") << strName;

    //
    // unencode name
    //
    cFCOName fcoNameNew;
    TEST(mpCurNT->DisplayStringToFCOName(strName, fcoNameNew));

    //
    // check result
    //
    TEST(fcoNameNew == fcoName);
}

void RegisterSuite_FCONameTranslator()
{
    RegisterTest("FCONameTranslator", "Basic", TestFCONameTranslator);
}
