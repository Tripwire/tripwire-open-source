//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2017 Tripwire,
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
// fsdatasourceiter_t
#include "fs/stdfs.h"
#include "fs/fsdatasourceiter.h"
#include "core/fsservices.h"
#include "core/debug.h"
#include "twtest/test.h"
#include "fco/fco.h"

/*
static void PrintDb( cHierDatabase::iterator iter, cDebug d, bool bFirst = true )
{
    if( ! bFirst )
    {
        iter.Descend();
    }
    d.TraceDebug( "-- Processing directory %s\n", iter.GetCwd().c_str() );

    for( iter.SeekBegin(); ! iter.Done(); iter.Next() )
    {
        d.TraceDebug( "Processing entry %s\n", iter.GetName().c_str() );
        if( iter.CanDescend() )
        {
            d.TraceDebug( ">>Descending...\n" );
            PrintDb(iter, d, false);
        }
    }

    d.TraceDebug( "-- Done Processing directory %s\n", iter.GetCwd().c_str() );
}
*/

static void PrintIter( cFSDataSourceIter iter, cDebug& d )
{
    //
    //debug stuff
    //
    
    if( ! iter.CanDescend() )
    {
        d.TraceError( "Iterator cannot descend; returning!\n");
        return;
    }
    iter.Descend();
    iter.TraceContents();

    for( iter.SeekBegin(); ! iter.Done(); iter.Next() )
    {
        iFCO* pFCO = iter.CreateFCO();
        if( pFCO )
        {
            pFCO->TraceContents();
            pFCO->Release();
        }
        else
        {
            d.TraceError( "*** Create of FCO failed!\n");
        }
        if( iter.CanDescend() )
        {
            d.TraceDebug( ">>Descending...\n" );
            PrintIter(iter, d);
        }
    }
}


void TestFSDataSourceIter()
{
    cFSDataSourceIter   iter;
    cDebug              d("TestFSDataSourceIter");
    try
    {
        // go to my temp directory and iterate over everything!
        iter.SeekToFCO( cFCOName(_T("/tmp")) );
        //
        // print out everything below the iterator
        //
        PrintIter( iter, d );
    }
    catch( eError& e )
    {
            d.TraceError( "*** Caught exception %d %s\n", e.GetID(), e.GetMsg().c_str() );
        TEST( false );
    }
}


