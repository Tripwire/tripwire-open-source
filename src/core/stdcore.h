//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2018 Tripwire,
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
// Name....: stdcore.h
// Date....: 05/07/99
// Creator.: Robert DiFalco (rdifalco)
//
// Standard includes for core, and therefore the entire project
//

#ifndef __STDCORE_H
#define __STDCORE_H


//--Include Core Platform Declarations

#include "platform.h"

//--Disable some level 4 warnings

#if IS_MSVC

#    define NOMINMAX //--Disable min/max macros

#include <xstddef> // ARGH!! This resumes certain warnings!

#    pragma warning(disable : 4786) // Token Name > 255
#    pragma warning(disable : 4127) // constant expression (e.g. ASSERT)
#    pragma warning(disable : 4291) // Incorrectly called when new(nothrow)!!!
#    pragma warning(disable : 4097) // Type synonyms are good!
#    pragma warning(disable : 4511) // Can't create copy ctor! (e.g.Private CTOR)
#    pragma warning(disable : 4512) // Can't create assignment!(e.g.Private CTOR)
#    pragma warning(disable : 4100) // Formal argument not used
#    pragma warning(disable : 4710) // Compiler did not inline function

#    ifndef DEBUG                       // ASSERT's are disabled give a lot of these
#        pragma warning(disable : 4702) // ---Unreachable Code
#    endif

/* NOTE:RAD:05/09/1999 -- Plaguers STDCPP implementation is SOOO bad
    it creates all these errors when compiling at W4. By doing a push
    before disabling them, we can still generate them for ourselves
    without a lot of noise. The warnings before this push, we want to
    always disable. */

#    pragma warning(push)

#    pragma warning(disable : 4663) // Old template specialization syntax
#    pragma warning(disable : 4018) // Signed unsigned mismatch
#    pragma warning(disable : 4245) // Signed unsigned mismatch
#    pragma warning(disable : 4663) // Use new template specialization syntax: template<>
#    pragma warning(disable : 4701) // local variable 'X' may be used without having been initialized
#    pragma warning(disable : 4510) // 'X' : default constructor could not be generated
#    pragma warning(disable : 4610) // struct 'X' can never be instantiated - user defined constructor required
#    pragma warning(disable : 4146) // unary minus operator applied to unsigned type, result still unsigned
#    pragma warning(disable : 4244) // '=' : conversion from 'unsigned int' to 'char', possible loss of data
#    pragma warning(disable : 4511)
#    pragma warning(disable : 4503)

#endif // #if IS_MSVC


//--Include Standard CPP Declarations

#include <cstdio>
#include <new>
#include <memory>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <stack>
#include <locale>
#include <limits>

#if IS_MSVC
#    pragma warning(pop) // Resume W4 msgs for TSS code
#endif

//--Include Platform Specific Declarations


//--Include Common Core Declarations

#include "./tchar.h"
#include "./types.h"
#include "./error.h"
#include "./debug.h"


#endif //__STDCORE_H
