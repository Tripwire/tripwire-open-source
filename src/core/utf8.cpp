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
#include "stdcore.h"
#include "utf8.h"   // Required Deps
#include "tchar.h"
//#include "integ/tss.integ.strings.h"
//#include "integ/tss.integ.strings.inl"

#ifdef _UNICODE


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CLASS UTF8: Member Definitions
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NOTES: utf8 uses "lazy evaluation" so that members are only allocated
// if they are used. However, once they are evaluated, the evaluation results
// will be saved so that values may henceforth simply be returned. This 
// scheme gives the best time vs. space performance possible as the complexity
// of each operation is at most, linear.


const char* cUTF8::str() const THROW(std::bad_alloc)        // UTF8: Get as Narrow UTF8 
{
    if ( mUTF8Str == 0 )                                    // If mUTF8Str is NULL mUnicodeStr can't be
        mUTF8Str = cUTF8::allocate( mUnicodeStr );

    return mUTF8Str;
}

const wchar_t* cUTF8::ustr() const THROW(std::bad_alloc)    // UTF8: Get as UNICODE NTWCS
{
    if ( mUnicodeStr == 0 )
        mUnicodeStr = cUTF8::allocate( mUTF8Str );          // If mUnicodeStr is NULL mUTF8WideStr can't be

    return mUnicodeStr;
}

const wchar_t* cUTF8::wstr() const THROW(std::bad_alloc)    // UTF8: Get as Wide UTF8
{
    if ( mUTF8WideStr == 0 )
    {
        size_t N = ::strlen( str() );                       // WARNING: Get strlen NOT mbcslen!!!
        mUTF8WideStr = new wchar_t[ N + 1 ];                // This may throw bad_alloc to client

        for ( size_t n = 0; n < N; n++ )                    // index ordinal, not pointer
            mUTF8WideStr[n] = wchar_t( BYTE(mUTF8Str[n]) ); // WARNING: First convert to "unsigned char"

        mUTF8WideStr[N] = 0;                                // add a final NULL
    }

    return mUTF8WideStr;
}


/// UTF8: UTF8-Unicode Round-trip Conversions

wchar_t* cUTF8::allocate( const char* in ) THROW( std::bad_alloc )
{
    ASSERT( in /* && TSS_IsValidString( in )*/ );           // Verify Input

    size_t N  = ::MultiByteToWideChar( CP_UTF8, 0, in, -1, 0, 0 );
    wchar_t* out = new wchar_t[ N + 1 ];                    // Allocate required size

    
    //--Convert

	out[0] = 0x00;                                          // NOTE: Just in case we fail
    #ifdef _DEBUG
    //size_t nWritten = 
    #endif
    ::MultiByteToWideChar( CP_UTF8, 0, in, -1, out, N );
    out[N] = 0;
 
#ifdef _INTEG2                                              // Verify Output
    if ( nWritten == 0 )
    {
	    cDebug d( "cUTF8::allocate" );
		d.TraceError( "MultiByteToWideChar failed with %x\n", ::GetLastError() );
    }

    ASSERT( out && TSS_IsValidString( out, nWritten ) );
#endif

    return out;
}

char* cUTF8::allocate( const wchar_t* in ) THROW( std::bad_alloc )
{
	ASSERT( in /*&& TSS_IsValidString( in ) */);            // Verify Input

    // Allocate required size
    size_t N = ::WideCharToMultiByte( CP_UTF8, 0, in, -1,0,0,0,0 );
    char* out = new char[ N + 1 ];

    
    //--Convert

	out[0] = 0x00;                                          // NOTE: Just in case we fail
    #ifdef _DEBUG
    //size_t nWritten = 
    #endif
    ::WideCharToMultiByte( CP_UTF8, 0, in, -1, out, N, 0, 0 );
    out[N] = 0;
   
#ifdef _INTEG2                                              // Verify Output
    if ( nWritten == 0 )
    {
		cDebug d( "cUTF8::allocate" );
		d.TraceError( "WideCharToMultiByte failed with %x\n", ::GetLastError() );
    }

    ASSERT( out /*&& TSS_IsValidString( out, nWritten ) */);
#endif

    return out;
}

#endif	//_UNICODE

