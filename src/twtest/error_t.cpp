//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
#include "twtest/test.h"
#include <iostream>

void TestError()
{
//#pragma message( __FILE__ "(1) : TODO - implement this test file")

/*
	try
	{
		std::cout << "Before Exception" << std::endl;
		std::cout << "Line number before throw: " << __LINE__ << std::endl;
		THROW_ERROR(53, _T("This is an error!"));
		std::cout << "After Exception" << std::endl;
	}
	catch(eError& e)
	{
        TEST(e.GetErrorNum() == 53);
        TEST(_tcscmp(e.GetMsg().c_str(), _T("This is an error!")) == 0);
		TCOUT << _T("Exception caught!\n\tErrorNum=") << e.GetErrorNum() << _T("\n\t") << e.GetMsg() << std::endl;
	}

    try 
    {
        THROW_INTERNAL("error_t.cpp");
    }
    catch(eInternal& e)
    {
        TEST(e.GetErrorNum() == eInternal::ERR_INTERNAL);
        TCOUT << _T("Internal error caught!\n\tErrorNum=") << e.GetErrorNum() << _T("\n\t") << e.GetMsg() << std::endl;
    }
    catch(...)
    {
        TEST(false);
    }
*/
}
