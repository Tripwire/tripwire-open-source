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
///////////////////////////////////////////////////////////////////////////////
// tchar.h
//
// VCC version of TCHAR.H that is cross platform compatible between UNIX and
// Windows.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Short Documentation:
// (put here at the top of the file so you don't have to go searching for it)
//
// Here are some of the important "T" versions of functions you should use
// when wishing to abstract single-byte/doublebyte chars using this header.
//
// Old Function/Symbol              "T" version
// --------------------             ------------------------------------
// char name[256];                  TCHAR name[256];
// printf("Hi there %s\n", name);   _tprintf(_T("Hi there %s"), name);
// strlen(name);                    _tcslen(name);
// strcpy(name, "Dave");            _tcscpy(name, _T("Dave");
// strcat(name, " the man");        _tcscat(name, _T(" the man"));
// sizeof(name)                     sizeof(name)/sizeof(TCHAR); (or countof(name), see macro below)
// string foo; (STL string)         TSTRING foo;
///////////////////////////////////////////////////////////////////////////////

#ifndef __TCHAR_H
#define __TCHAR_H

// A little macro that's useful for finding the number of characters in a TCHAR ARRAY
#define countof(x) (sizeof(x) / sizeof(x[0]))


#include <string.h>
#include <string>
#include <sstream>
#include <iostream>

#define TCHAR char
#define _tmain main

typedef std::string        TSTRING;
typedef std::stringstream  TSTRINGSTREAM;
typedef std::ostringstream TOSTRINGSTREAM;
typedef std::istringstream TISTRINGSTREAM;
typedef std::ostream       TOSTREAM;
typedef std::istream       TISTREAM;
typedef std::ofstream      TOFSTREAM;
typedef std::ifstream      TIFSTREAM;

// iostream abstractions
#    define TCIN std::cin
#    define TCOUT std::cout
#    define TCERR std::cerr

// other functions
#    define _ftprintf fprintf
#    define _stprintf sprintf
#    define _itot itoa
#    define _ttoi atoi
#    define _istdigit isdigit
#    define _tcsftime strftime
#    define _vtprintf vprintf
#    define _vftprintf vfprintf
#    define _topen open
#    define _tfdopen fdopen
#    define _tfopen fopen
#    define _tmkdir mkdir
#    define _tcscpy strcpy
#    define _tgetenv getenv
#    define _taccess access
#    define _tcreat creat
#    define _tunlink unlink
#    define _tcscmp strcmp
#    define _tcsicmp strcasecmp
#    define _totlower tolower
#    define _totupper toupper
#    define _tcslen strlen
#    define _tcscat strcat
#    define _tcsncmp strncmp

// other abstractions
#    define TUNLINK unlink

// string representation
#    if defined(_T)
        // run it right over with a bulldozer, tripwire doesn't seem
        // to use ctype.h's _T   -PH
#        undef _T
#    endif
#    define _T(x) x

// misc...
#    ifndef __cdecl
#        define __cdecl
#    endif

#endif // __TCHAR_H
