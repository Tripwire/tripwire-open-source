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
// utf8.h
//

// UTF8 is used to map UNICODE characters to NTBS targets
// that would normally not be able to dealt with. UTF8
// ensures symetrical round-trip conversions. In other
// words an unmapable UNICODE character converted to UTF8
// will always convert back to the same UNICODE value.
//
// For more information, go to:
//  mk:@MSITStore:\\SQUID\MSDN%20January%201999\MSDN\kbwindev.chm::/Source/win32sdk/q175392.htm

#ifndef __UTF8_H
#define __UTF8_H

#include <iostream>              // operator<<

//#ifndef __TSS_INTEG_STRINGS_H
//#include "integ/tss.integ.strings.h"   // IsValidAddress and IsValidString
//#endif
#ifndef __DEBUG_H
#include "debug.h"
#endif

//-----------------------------------------------------------------------------
// convenience macros 
//
// use these when printing a string ( TCHAR* ) to stdout, stderr, of printf-style
// function to safely convert the string to something printable
//-----------------------------------------------------------------------------
#ifdef _UNICODE
#	define TSS_UTF8( x ) cUTF8( x ).wstr()
#else
#	define TSS_UTF8( x ) x
#endif

#ifdef _UNICODE		// this class is only needed in unicode builds...

//-----------------------------------------------------------------------------
// Entities Declared in this Module
//-----------------------------------------------------------------------------

class cUTF8;

std::ostream& operator<<( std::ostream&, const cUTF8& );
std::wostream& operator<<( std::wostream&, const cUTF8& );


/// Class Declaration

#ifdef _THROW_SUPPORTED
#define THROW( x )   throw( x )
#define THROW_0      throw()
#else
#define THROW( x )
#define THROW_0
#endif

//=============================================================================
// class cUTF8: Encapsulate UTF8 Encoding
//
// SYNOPSIS: 
//  Allows unmappable UNICODE characters to be embedded into Narrow strings
//  and displayable by narrow display streams.
//
// CONSTRAINTS:
//  Cannot default or copy construct or assign UTF8 instances. Each UTF8
//  instance is an exclusive owner of its resources for its life-time.
//  This cuts down on the amount of redundant allocation and copying that
//  is required by the class.
//=============================================================================

class cUTF8
{
    /// Interface.

    public:

        cUTF8( const char* )         THROW( std::bad_alloc );
        cUTF8( const wchar_t* )      THROW( std::bad_alloc );
        ~cUTF8();

        const char*    str()  const THROW( std::bad_alloc );  // Narrow UTF8
        const wchar_t* wstr() const THROW( std::bad_alloc );  // Wide UTF8
        const wchar_t* ustr() const THROW( std::bad_alloc );  // Unicode


    /// Implementation.

    protected:

        static char* allocate( const wchar_t* )  THROW( std::bad_alloc );
        static wchar_t* allocate( const char*    )  THROW( std::bad_alloc );

    protected:

        cUTF8( const cUTF8& );                // Disallow
        cUTF8& operator=( const cUTF8& );     // Disallow

    private: 

        mutable wchar_t* mUnicodeStr;  // UNICODE String
        mutable char*    mUTF8Str;  // UTF8 Bytes String
        mutable wchar_t* mUTF8WideStr;  // UTF8 Wide Character String
};

//-----------------------------------------------------------------------------
// UTF8 Inlines: Members and Global Binary Non-Member Operators
//-----------------------------------------------------------------------------

// Create from NTBS UTF8
inline cUTF8::cUTF8( const char* psz ) THROW( std::bad_alloc ) 
:   mUnicodeStr( cUTF8::allocate( psz ) ),
    mUTF8Str( 0 ),
    mUTF8WideStr( 0 )
{
    ASSERT( mUnicodeStr || mUTF8Str );
}

// Create from NTWCS UNICODE
inline cUTF8::cUTF8( const wchar_t* wsz ) THROW( std::bad_alloc ) 
:   mUnicodeStr( 0 ),
    mUTF8Str( cUTF8::allocate( wsz ) ),
    mUTF8WideStr( 0 )
{
    ASSERT( mUnicodeStr || mUTF8Str );
}

inline cUTF8::~cUTF8()
{
    delete [] mUnicodeStr;   // Delete UNICODE (if exists)
    delete [] mUTF8Str;   // Delete UTF8 (NTBS)
    delete [] mUTF8WideStr;   // Delete UTF8 (NTWCS)
}


/// Operators

inline std::ostream& operator<<( std::ostream& out, const cUTF8& str )
{
   out << str.str();
   return out;
}

inline std::wostream& operator<<( std::wostream& wout, const cUTF8& str )
{
   wout << str.wstr();  // CAUTION: Don't send UNICODE string, it won't display!
   return wout;
}


#endif //_UNICODE

#endif/*_TSS_UTF8_H*/

