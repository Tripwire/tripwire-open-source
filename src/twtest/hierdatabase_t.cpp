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
// hierdatabase_t
#include "db/stddb.h"
#include "db/hierdatabase.h"
#include "test.h"
#include "core/error.h"

/*static void PrintDb( cHierDatabase::iterator iter, cDebug d, bool bFirst = true )
{
    if( ! bFirst )
    {
        iter.Descend();
    }
    d.TraceDebug( "-- Processing directory %s\n", iter.GetCwd().c_str() );

    for( iter.SeekBegin(); ! iter.Done(); iter.Next() )
    {
        d.TraceDebug( "Processing entry %s\n", iter.GetName() );
        if( iter.CanDescend() )
        {
            d.TraceDebug( ">>Descending...\n" );
            PrintDb(iter, d, false);
        }
    }

    d.TraceDebug( "-- Done Processing directory %s\n", iter.GetCwd().c_str() );
}*/

static void GetNoun( TSTRING& noun )
{
    static TSTRING prevNoun;
    TCIN >> noun;
    if( noun.compare( _T("!$") ) == 0 )
    {
        noun = prevNoun;
    }
    prevNoun = noun;
}

///////////////////////////////////////////////////////////////////////////////
// TestHierDatabaseInteractive -- this provides an interactive interface to 
//      the database
///////////////////////////////////////////////////////////////////////////////
void TestHierDatabaseInteractive()
{
    cDebug d( "TestHierDatabaseInteractive" );
    try
    {
        cHierDatabase db;
        //db.Open( _T("c:/tmp/tw.hdb"), 5, true);
        db.Open( _T("c:/tmp/tw.db"), 5, false);
        cHierDatabase::iterator iter(&db);

        while( true )
        {
            TSTRING verb, noun;
            TCOUT << _T(">>");
            TCIN >> verb;
            //
            // ok, now we switch on the command...
            //
            //-----------------------------------------------------------------
            // quit
            //-----------------------------------------------------------------
            if( verb.compare( _T("quit") ) == 0 )
            {
                // the quit command...
                break;
            }
            //-----------------------------------------------------------------
            // print
            //-----------------------------------------------------------------
            if( verb.compare( _T("print") ) == 0 )
            {
                // the print command...
                ASSERT( false );
                // TODO -- Implement this!
            }
            //-----------------------------------------------------------------
            // mkdir
            //-----------------------------------------------------------------
            else if( verb.compare( _T("mkdir") ) == 0 )
            {
                GetNoun(noun);
                TCOUT << "Making a child of " << noun << std::endl;
                if( iter.SeekTo( noun.c_str() ) )
                {
                    iter.CreateChildArray();
                }
                else
                {
                    TCOUT << "Unable to find object " << noun << std::endl;
                }
            }
            //-----------------------------------------------------------------
            // mk
            //-----------------------------------------------------------------
            else if( verb.compare( _T("mk") ) == 0 )
            {
                GetNoun(noun);
                TCOUT << "Making object " << noun << std::endl;
                if( iter.SeekTo( noun.c_str() ) )
                {
                    TCOUT << "Error: object already exists!" << std::endl;
                }
                else
                {
                    iter.CreateEntry( noun );
                }
            }
            //-----------------------------------------------------------------
            // rmdir
            //-----------------------------------------------------------------
            else if( verb.compare( _T("rmdir") ) == 0 )
            {
                GetNoun(noun);
                TCOUT << "Removing the child of " << noun << std::endl;
                if( iter.SeekTo( noun.c_str() ) )
                {
                    //TODO -- check that it has an empty child
                    iter.DeleteChildArray();
                }
                else
                {
                    TCOUT << "Unable to find object " << noun << std::endl;
                }
            }
            //-----------------------------------------------------------------
            // rm
            //-----------------------------------------------------------------
            else if( verb.compare( _T("rm") ) == 0 )
            {
                GetNoun(noun);
                TCOUT << "Removing object " << noun << std::endl;
                if( iter.SeekTo( noun.c_str() ) )
                {
                    if( iter.CanDescend() )
                    {
                        TCOUT << "Can't delete object; it still has children." << std::endl;
                    }
                    else
                    {
                        iter.DeleteEntry();
                    }
                }
                else
                {
                    TCOUT << "Unable to find object " << noun << std::endl;
                }
            }
            //-----------------------------------------------------------------
            // pwd
            //-----------------------------------------------------------------
            else if( verb.compare( _T("pwd") ) == 0 )
            {
                TCOUT << iter.GetCwd() << std::endl;
            }
            //-----------------------------------------------------------------
            // ls
            //-----------------------------------------------------------------
            else if( verb.compare( _T("ls") ) == 0 )
            {
                int cnt = 0;
                for( iter.SeekBegin(); ! iter.Done(); iter.Next(), cnt++ )
                {
                    TCOUT << "[" << cnt ;
                    if( iter.CanDescend() )
                    {
                        TCOUT << "]*\t" ;
                    }
                    else
                    {
                        TCOUT << "]\t" ;
                    }
                    TCOUT << iter.GetName() << std::endl;
                }
            }
            //-----------------------------------------------------------------
            // cd
            //-----------------------------------------------------------------
            else if( verb.compare( _T("cd") ) == 0 )
            {
                GetNoun(noun);
                if( noun.compare( _T("..") ) == 0 )
                {
                    if( iter.AtRoot() )
                    {
                        TCOUT << "Can't ascend above root." << std::endl;
                    }
                    else
                    {
                        TCOUT << "Ascending..." << std::endl;
                        iter.Ascend();
                    }
                }
                else
                {
                    if( iter.SeekTo( noun.c_str() ) )
                    {
                        if( iter.CanDescend() )
                        {
                            TCOUT << "Descending into " << noun << std::endl;
                            iter.Descend();
                        }
                        else
                        {
                            TCOUT << noun << " has no children; can't descend." << std::endl;
                        }
                    }
                    else
                    {
                        TCOUT << "Unable to find object " << noun << std::endl;
                    }
                }
            }
            //-----------------------------------------------------------------
            // rmdir
            //-----------------------------------------------------------------
            else if( verb.compare( _T("rmdir") ) == 0 )
            {
                GetNoun(noun);
                if( iter.SeekTo( noun.c_str() ) )
                {
                    if( iter.CanDescend() )
                    {
                        TCOUT << "Deleting child of " << iter.GetName() << std::endl;
                        iter.DeleteChildArray();
                    }
                    else
                    {
                        TCOUT << noun << " has no child; can't delete." << std::endl;
                    }
                }

            }
            //-----------------------------------------------------------------
            // rm
            //-----------------------------------------------------------------
            else if( verb.compare( _T("rm") ) == 0 )
            {
                GetNoun(noun);
                if( iter.SeekTo( noun.c_str() ) )
                {
                    if( iter.CanDescend() )
                    {
                        TCOUT << noun << " doesn't exist; can't delete." << std::endl;
                    }
                    else
                    {
                        TCOUT << "Deleting  " << iter.GetName() << std::endl;
                        iter.DeleteEntry();
                    }
                }

            }
            //-----------------------------------------------------------------
            // set
            //-----------------------------------------------------------------
            else if( verb.compare( _T("set") ) == 0 )
            {
                GetNoun(noun);
                if( iter.SeekTo( noun.c_str() ) )
                {
                    if( iter.HasData() )
                    {
                        iter.RemoveData();
                    }
                    TSTRING data;
                    TCIN >> data;
                    iter.SetData( (int8*)data.c_str(), data.length() + 1 );
                    TCOUT << "Setting " << noun << "'s data to " << data << std::endl;
                }
                else
                {
                    TCOUT << "Can't find object " << noun << std::endl;
                }

            }
            //-----------------------------------------------------------------
            // get
            //-----------------------------------------------------------------
            else if( verb.compare( _T("get") ) == 0 )
            {
                GetNoun(noun);
                if( iter.SeekTo( noun.c_str() ) )
                {
                    if( ! iter.HasData() )
                    {
                        TCOUT << "Object has no data!" << std::endl;
                    }
                    else
                    {
                        int32 dummyLength;
                        TCOUT << noun << "'s data is: " << (TCHAR*)iter.GetData(dummyLength) << std::endl;
                    }
                }
                else
                {
                    TCOUT << "Can't find object " << noun << std::endl;
                }

            }

            // make sure the file is still valid...
            //
#ifdef _BLOCKFILE_DEBUG
            db.AssertAllBlocksValid() ;
#endif
        }

        TCOUT << "Exiting..." << std::endl;

    }
    catch( eError& e )
    {
        d.TraceError( "Exception caught: %d %s\n", e.GetID(), e.GetMsg().c_str() );
        TEST( false );
    }

}
