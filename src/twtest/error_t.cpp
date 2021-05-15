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
// error_t.h -- the vcc exception test driver

#include "core/stdcore.h"
#include "core/error.h"
#include "core/errorgeneral.h"
#include "core/errorutil.h"
#include "twtest/test.h"
#include <iostream>

void TestError()
{
    bool threw = false;
    try
    {
        //       std::cout << "Before Exception" << std::endl;
        //       std::cout << "Line number before throw: " << __LINE__ << std::endl;
        throw eErrorGeneral(_T("This is an error!"));
        //       std::cout << "After Exception" << std::endl;
    }
    catch (eError& e)
    {
        threw = true;
        TEST(_tcscmp(e.GetMsg().c_str(), _T("This is an error!")) == 0);
        //       TCOUT << _T("Exception caught!\n\nID=") << e.GetID() << _T("\n\t") << e.GetMsg() << std::endl;
    }
    catch (...)
    {
        TEST(false);
    }

    TEST(threw);


    try
    {
        threw = false;
        throw eInternal("error_t.cpp");
    }
    catch (eInternal& e)
    {
        threw = true;
        TEST(_tcscmp(e.GetMsg().c_str(), _T("error_t.cpp")) == 0);
        //        TCOUT << _T("Internal error caught!\n\nID=") << e.GetID() << _T("\n\t") << e.GetMsg() << std::endl;
    }
    catch (...)
    {
        TEST(false);
    }

    TEST(threw);
}

void RegisterSuite_Error()
{
    RegisterTest("Error", "Basic", TestError);
}
