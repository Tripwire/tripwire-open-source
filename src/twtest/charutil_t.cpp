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
//
// Name....: charutil_t.cpp
// Date....: 10/22/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "core/stdcore.h"

#ifdef TSS_TEST

#include "test/utx.h"
#include "charutil.h"
#include "debug.h"
#include "errorbucketimpl.h"


///////////////////////////////////////////////////////////////////////////////
// cCharEncoderTest
///////////////////////////////////////////////////////////////////////////////
class cCharEncoderTest
{
public:

    void PrintChars( const TSTRING& str )
    {
        const TCHAR* cur = str.begin();
        const TCHAR* end = str.end();
        const TCHAR* first = NULL;
        const TCHAR* last  = NULL;

        while( cCharUtil::PopNextChar( cur, end, first, last ) )
        {
            TCOUT << _T("char length: ") << (int)(last - first) << std::endl;

            TCOUT << _T("char: <");
            for( const TCHAR* at = first; at != last; at++ )
            {
                if( at != first )
                    TCOUT << _T(",");
                TCOUT << (int)*at;
            }
            TCOUT << _T(">") << std::endl;
        }
        
        TCOUT << _T("----------------------------") << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Basic
    ///////////////////////////////////////////////////////////////////////////    
    void Basic( tss::TestContext& ctx )
    {
        try
        {
            PrintChars( _T("foo") );
            PrintChars( _T("fo\x2354") );
        }
        catch( eError& e )
        {
            cErrorReporter::PrintErrorMsg( e ); 
            ASSERT(false);
        }
    }
};

TSS_BeginTestSuiteFrom( cCharEncoderTest )

    TSS_AddTestCase( Basic );
        
TSS_EndTestSuite( cCharEncoderTest )

#endif // TSS_TEST

// eof: charutil_t.cpp
