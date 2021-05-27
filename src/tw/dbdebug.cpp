//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
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
//dbdebug.cpp
//

#include "stdtw.h"

#include "dbdebug.h"
#include "db/hierdatabase.h"
#include "db/hierdbnode.h"
#include "db/blockrecordarray.h"
#include "db/blockfile.h"
#include "dbdatasource.h"
#include "fcodatabasefile.h"
#include "db/blockrecordfile.h"
#include "fco/twfactory.h"
#include "fco/fconametranslator.h"

#include "core/error.h"
#include "core/errorbucketimpl.h"
#include "core/debug.h"
#include "fco/genreswitcher.h"

#include "fco/fcopropdisplayer.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"

#include <utility>
#include <map>
#include <list>
#include <vector>
#include "core/tchar.h"
#include "core/types.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// UTILITY METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//static void util_PrintFCO(const iFCO* pFCO, iFCOPropDisplayer * pPropDisplayer);
static void util_InitBlockArray(cBlockRecordArray& block);
//static void util_MapHierRoot( std::map< std::pair< int, int>, int > dbMap );

//////////////////////////////////////////////////////////////////////////////////////////////////
// Insulated implementation:
//////////////////////////////////////////////////////////////////////////////////////////////////
struct cDbDebug_i
{
    cDbDebug_i(){};
    ~cDbDebug_i(){};

    typedef std::map<std::pair<int, int>, int> hierDbMap;
    // This is the data structure that we will use to match the information stored in a quantum database
    // with its hierarchical representation!  For now, it will hold the <int, int> tuples representing
    // valid offsets in each block (referenced by a block number).  The bool value will be set to false
    // on the first run, and then flipped to true for each entry that is found while traversing the
    // hierarchical database.
    typedef std::list<std::pair<int, int> > AddressList;

    hierDbMap mDbMap; // The map itself.
    AddressList
                mlMissingFromBlockFile; // A list for keeping track of nodes found in the hierarchy, but not the blockfile
    AddressList mlMissingFromHierarchy; // The opposite case.
};

cDbDebug::cDbDebug()
{
    mpData = new cDbDebug_i();
}

cDbDebug::~cDbDebug()
{
    delete mpData;
}


///////////////////////////////////////////////////////////////////////////////
// util_InitBlockArray
///////////////////////////////////////////////////////////////////////////////
static void util_InitBlockArray(cBlockRecordArray& block)
{
    if (!block.Initialized())
    {
        block.InitForExistingBlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
// util_MapHierRoot : Map the Root and RootArray in the HierDatabase:
///////////////////////////////////////////////////////////////////////////////
static void util_MapHierRoot(std::map<std::pair<int, int>, int>* dbMap)
{
    cDbDebug_i::hierDbMap::iterator i = dbMap->find(std::pair<int, int>(0, 0));
    //dbMap->insert( cDbDebug_i::hierDbMap::value_type( std::pair< int, int > ( 0, 0 ), 1 ) );
    //
    // This insert statement should work to change the value, but it doesn't ????
    (*i).second = 1;

    i           = dbMap->find(std::pair<int, int>(0, 1));
    (*i).second = 1;

    i           = dbMap->find(std::pair<int, int>(0, 2));
    (*i).second = 1;
}

///////////////////////////////////////////////////////////////////////////////
// DisplayDbMap -- Displays the entire map.
///////////////////////////////////////////////////////////////////////////////
void cDbDebug::DisplayDbMap()
{
    int loop_count = 0;

    for (cDbDebug_i::hierDbMap::iterator i = cDbDebug::mpData->mDbMap.begin(); i != cDbDebug::mpData->mDbMap.end();
         ++i, ++loop_count)
    {
        TCOUT.width(6);
        TCOUT << (*i).first.first << _T(",") << (*i).first.second << _T(" ");
        TCOUT << _T("(") << (*i).second << _T(")");
        if ((loop_count % 5) == 0)
            TCOUT << std::endl;
    }
    TCOUT << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
// GetHierDbMap() -- Returns a pointer to the map
/////////////////////////////////////////////////////////////////////////////////
cDbDebug::hierDbMap* cDbDebug::GetHierDbMap(void)
{
    return &(mpData->mDbMap);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Execute :
//      Drives the DebugDb mode of tripwire, which will only be used for internal use.
//////////////////////////////////////////////////////////////////////////////////////////////////////
void cDbDebug::Execute(cFCODatabaseFileIter& dbIter, const TSTRING& dbFile)
{

    cDebug   d("cDebugDb::Execute");
    cDbDebug DbDebug;
    //
    //A derived class with extra methods for obtaining blockfile information.
    cDebugHierDb* db;
    //
    // Cast the database object that we have inherited from the "common" command line
    // information to a cDebugHierDb.  This will expose all the information that we
    // need to debug the database.
    //
    db = static_cast<cDebugHierDb*>(&(dbIter.GetDb()));

    try
    {
        cDebugHierDbIter pIter(db);

        //
        //First, map the state of the underlying quantyum database.  Store this info. in
        //the hierDbMap.
        //
        DbDebug.MapQuantumDatabase(*db);

        //DbDebug.DisplayDbMap(); //TODO : get rid of these display calls.

        //
        // Uncomment ManipDb() to allow manipulation of the database
        // This may be handy if one does not want to do a full update...
        //DbDebug.ManipDb( dbIter );

        // Next, traverse the hierarchical overlay and "tag" all entries in the map that
        // we see:
        ASSERT(pIter.AtRoot());
        //
        // check to make sure we are at the root of the hierarchy, if so, map the root
        // before calling the recursive traversal function.
        //
        util_MapHierRoot(DbDebug.GetHierDbMap());

        DbDebug.TraverseHierarchy(pIter /*, db*/);

        //DbDebug.DisplayDbMap(); //TODO: get rid of these display calls.
        //
        // Finally, iterate over the map and see if there is any data that the hierarchy
        // does not account for:
        //
        DbDebug.CongruencyTest();
        //
        // Output the results
        //
        DbDebug.OutputResults();

    } //try

    catch (eError& e)
    {
        cErrorReporter::PrintErrorMsg(e);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// MapQuantumDatabase --
//
// Our objective is to map all the information that the database is currently storing.  It
// may be necessary to do some hacking about with the database object in order to expose
// all the information necessary to accomplish this.
//
// Addition: 2/15/99: We want to print out statistics on the current state of the block
//  file, so let's do it here ( since we're traversing it to map the addresses, anyway ).
//////////////////////////////////////////////////////////////////////////////////////////////
void cDbDebug::MapQuantumDatabase(cDebugHierDb& db)
{
    //mirroring the implementation in blockrecordfile.h:
    typedef std::vector<cBlockRecordArray> BlockArray;

    //Using the two added methods to gain access to the low-level implementation of the
    //quantum database.
    BlockArray* pBlkArray = db.myGetBlockArray();

    // Print some statistics on the blockfile if we're in debug mode:
#ifdef _BLOCKFILE_DEBUG
    db.myGetBlockFile()->TraceContents();
#endif
    // Prints as much information about the current state of the block file as can be
    // assessed.

    // time to iterate over the database:
    std::vector<cBlockRecordArray>::iterator i;
    int                                      count = 0;

    for (i = (*pBlkArray).begin(); i != (*pBlkArray).end(); ++i, ++count)
    {
        util_InitBlockArray(*i);
        //This is necessary to make sure that each block is initialized as we iterate
        //over the block array.
        //
        // Print out statistics on this block, if we're in debug mode:
#ifdef _BLOCKFILE_DEBUG
        i->TraceContents();
#endif


        //Search through all the indexes and determine which of them references valid
        //information.  Store these <int, int> pairs ( the first int being the value of
        //count....
        for (int j = 0; j <= i->GetNumItems(); ++j)
        {
            if (i->IsItemValid(j))
            {
                //
                // We found a valid node in the database, so store it in the map.
                //
                mpData->mDbMap.insert(cDbDebug_i::hierDbMap::value_type(std::pair<int, int>(count, j), 0));
            }
            // if not, just don't store it.  The index is no longer valid.
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// TraverseHierarchy --
//
// A recursive function for traversing a hierarchical database, accounting for all the data
// structures that make up the object.  Looks at data addresses as well as structures that are
// necessary to maintain the hierarchy ( namely cHierArrayInfo records ).
////////////////////////////////////////////////////////////////////////////////////////////////
void cDbDebug::TraverseHierarchy(cDebugHierDbIter pIter)
{
    if (!pIter.CanDescend())
        return; //Done with this call.

    pIter.Descend();
    //
    //Descend once from the root, into the first child set. Also, descend once for each recursive call.
    //

    for (pIter.SeekBegin(); !pIter.Done(); pIter.Next())
    {
        //Try to match the parent's address in the database.
        if (!pIter.Done())
        {
            if (pIter.myGetEntryArrayIt() == pIter.myGetEntryArray().begin())
            //We're dealing with a cHierArrayInfo object, so treat it differently.
            {
                //
                // Let's map the parent address, and the array address, so we account for all the data structures
                // that make up this particular node.
                //
                MapHierDbNodes(mpData->mDbMap,
                               std::pair<int, int>(pIter.myGetArrayInfo().mParent.mBlockNum,
                                                   pIter.myGetArrayInfo().mParent.mIndex),
                               pIter);
                // and the array...
                MapHierDbNodes(
                    mpData->mDbMap,
                    std::pair<int, int>(pIter.myGetArrayInfo().mArray.mBlockNum, pIter.myGetArrayInfo().mArray.mIndex),
                    pIter);
            }
            else
            //This is a regular cHierEntry, so look at the Data and Child (if it exists ).
            {

                cDebugHierDbIter::EntryArray::iterator lEntryArrayIt = pIter.myGetEntryArrayIt();

                MapHierDbNodes(mpData->mDbMap,
                               std::pair<int, int>(lEntryArrayIt->mData.mBlockNum, lEntryArrayIt->mData.mIndex),
                               pIter);

                --lEntryArrayIt;
                // Get the address of this node by examining the previous next pointer in the entry list.
                //

                //
                // Map the next peer entry in the list. TODO: This may very well be overkill... if so, lose this call to MapHierDbNodes.
                //
                MapHierDbNodes(mpData->mDbMap,
                               std::pair<int, int>(lEntryArrayIt->mNext.mBlockNum, lEntryArrayIt->mNext.mIndex),
                               pIter);
                //
                // We also want to map the address of the child array, if non-null.
                //
                if (lEntryArrayIt->mChild.mBlockNum != -1)
                    MapHierDbNodes(mpData->mDbMap,
                                   std::pair<int, int>(lEntryArrayIt->mChild.mBlockNum, lEntryArrayIt->mChild.mIndex),
                                   pIter);
                //
                // Finally, map the address of the data.
                //
                MapHierDbNodes(mpData->mDbMap,
                               std::pair<int, int>(lEntryArrayIt->mData.mBlockNum, lEntryArrayIt->mData.mIndex),
                               pIter);
            }

        } //if
        //
        // Check to see if this particular node has a child array. If so, make recursive call.
        //
        if (pIter.CanDescend())
        {
            //This node has children, call myself
            TraverseHierarchy(pIter);
        }
    } //for
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MapHierDbNodes --
//
// Maps a given address in the hierarchy.  This function looks up the address in the map and flips an integer flag
// to record the event.  If the address is not found, it is placed in a list of addresses for error output.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void cDbDebug::MapHierDbNodes(std::map<std::pair<int, int>, int>& dbMap,
                              std::pair<int, int>                 address,
                              cDebugHierDbIter&                   pIter)
{
    cDbDebug_i::hierDbMap::iterator i = dbMap.find(std::pair<int, int>(address.first, address.second));

    if (i != dbMap.end())
    {
        (*i).second = 1;
        //Flip the integer flag to 1, since we've found the address in the hierarchy.
    }
    else
        mpData->mlMissingFromBlockFile.push_back(std::pair<int, int>(address.first, address.second));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CongruencyTest --
//
// Iterate over the map and see if there are any entries that haven't been tagged by our
// hierarchy traversal.  If there are, push the offending addresses onto the address list.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cDbDebug::CongruencyTest(void)
{
    cDbDebug_i::hierDbMap::iterator i = mpData->mDbMap.begin();
    for (; i != mpData->mDbMap.end(); ++i)
    {
        if ((*i).second != 1)
        {
            //We've found something in the blockfile map that is not accounted for by the
            //hierarchy.  Add this address to the appropriate list.
            //TCOUT<< (*i).first.first << _T(",") << (*i).first.second << std::endl;
            //TCOUT<< (*i).second <<std::endl;
            mpData->mlMissingFromHierarchy.push_back(std::pair<int, int>((*i).first.first, (*i).first.second));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OutputResults --
//
// Output the results of the congruency test.  Print information contained in the two
// "error lists" if they have entries.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cDbDebug::OutputResults(void)
{
    TCOUT << _T("\nResults of Blockfile / Hierarchy congruency tests: \n") << std::endl;

    // Check to see if our error lists are empty.  If so, output a message that indicates
    // that the two databases checked clean.
    if (mpData->mlMissingFromHierarchy.empty() && mpData->mlMissingFromBlockFile.empty())
        TCOUT << _T("The database representations match!\n") << std::endl;
    else
    {
        // Check to see if there was anything in the hierarchy that was not accounted for by the
        // blockfile (quantum database).
        if (!mpData->mlMissingFromBlockFile.empty())
        {
            TCOUT << _T(" Objects (referenced by address) found in the hierarchy that were not\n");
            TCOUT << _T(" accounted for by the underlying quantum database: \n");

            cDbDebug_i::AddressList::iterator i = mpData->mlMissingFromBlockFile.begin();
            for (; i != mpData->mlMissingFromBlockFile.end(); ++i)
                TCOUT << _T("(") << (*i).first << _T(",") << (*i).second << _T(")") << std::endl;
        }
        else
            TCOUT << _T(" All objects in Blockfile accounted for!\n");

        //check to see if there was anything in the blockfile that was not accounted for by the
        // hierarchy:
        if (!mpData->mlMissingFromHierarchy.empty())
        {
            TCOUT << _T(" Database information (referenced by a <blocknumber, offset> address)\n");
            TCOUT << _T(" not accounted for by the hierarchy :\n");

            cDbDebug_i::AddressList::iterator i = mpData->mlMissingFromHierarchy.begin();
            for (; i != mpData->mlMissingFromHierarchy.end(); ++i)
                TCOUT << _T("(") << (*i).first << _T(",") << (*i).second << _T(")") << std::endl;
        }
        else
            TCOUT << _T("All objects in Hierarchy accounted for!\n");
    } //else
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin code for manipulating Database -- This is just a straight port from the DbExplore code.  It's presence is
// purely for convenience, and for debugging this class.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
static inline bool IsSpace( TCHAR c )
{
    return ( (c == _T(' ')) || (c == _T('\t')) || (c == _T('\r')) || (c == _T('\n')) || (c == _T('\0')) );
}

static inline bool IsEnd( TCHAR c )
{
    return ( (c == _T('\0')) );
}

///////////////////////////////////////////////////////////////////////////////
// util_PrintFCO
///////////////////////////////////////////////////////////////////////////////
static void util_PrintFCO( const iFCO* pFCO, const iFCOPropDisplayer* pDisplayer )
{
    TCOUT.setf(std::ios::left);
    
    TCOUT   << "------- " 
            << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( pFCO->GetName() )
            << " -------" 
            << std::endl;
    //
    // iterate over all of the properties
    //
    const iFCOPropSet* pPropSet = pFCO->GetPropSet();
    cFCOPropVector v        = pPropSet->GetValidVector();
    for( int i=0; i < pPropSet->GetNumProps(); i++ )
    {   
        if( v.ContainsItem( i ) )
        {
            TCOUT << "[";
            TCOUT.width(2);
            TCOUT << i << "]" ;
            TCOUT.width(25);
            TCOUT << pPropSet->GetPropName(i);
            TCOUT.width(0);
            TCOUT << pDisplayer->PropAsString( pFCO, i ) << std::endl;
        }
    }
    TCOUT << "--------------------------------------------" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// GetNoun
//      note -- you can only call this once after the verb (it eats the rest
//          of the line!)
///////////////////////////////////////////////////////////////////////////////
static void GetNoun( TSTRING& noun )
{
    static TSTRING prevNoun;
    TCHAR buf[1024];

    TCIN.getline( buf, 1024 );
    //
    // find the end of the noun...
    //
    TCHAR* pStart   = buf;
    TCHAR* end      = &buf[1023];
    while( IsSpace(*pStart) && (! IsEnd(*pStart)) && (pStart < end) )
        pStart++;

    if( IsEnd( *pStart ) || (pStart >= end) )
    {
        // no noun!
        noun = _T("");
        return;
    }
    TCHAR* pCur     = pStart;
    bool bQuote = false;
    if( *pCur == _T('\"') )
    {
        bQuote = true;
        pCur++;
        pStart++;
    }
    while( pCur < end )
    {
        if( (! bQuote) && IsSpace(*pCur) )
            break;

        if( *pCur == _T('\"') && bQuote)
            break;

        pCur++;
    }
    noun.assign(pStart, (pCur - pStart));

    if( noun.compare( _T("!$") ) == 0 )
    {
        noun = prevNoun;
    }
    prevNoun = noun;
}

///////////////////////////////////////////////////////////////////////////////
// SeekTo -- seeks to the named noun; this takes care of interpriting special 
//      nouns. If the seek fails, then Done() is true and false is returned.
//      If noun is a special character, then it is altered to what it mapped to.
///////////////////////////////////////////////////////////////////////////////
static bool SeekTo(cDbDataSourceIter* pIter, TSTRING& noun )
{
    if( noun[0] == _T('*') )
    {
        // interprite the rest of the string as a number index to seek to...
        int index = _ttoi( &noun[1] );
        if( index < 0 )
            return false;
        pIter->SeekBegin();
        for( int i=0; ((i < index) && (! pIter->Done())); i++, pIter->Next() )
        {

        }
        if( pIter->Done() )
            return false;
        noun = pIter->GetShortName();
        return true;
    }
    else
        return ( pIter->SeekTo( noun.c_str() ) );
}

void cDbDebug::ManipDb( cFCODatabaseFileIter& dbIter )
{
    cDbDataSourceIter* pIter        = new cDbDataSourceIter( &dbIter.GetDb(), dbIter.GetGenre() );
    const iFCOPropDisplayer* pDisplayer = dbIter.GetGenreHeader().GetPropDisplayer();

    ////////////////////////////
    // the main event loop...
    ////////////////////////////
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
            GetNoun(noun);
            if( SeekTo( pIter, noun ) )
            {
                if( pIter->HasFCOData() )
                {
                    iFCO* pFCO = pIter->CreateFCO();
                    util_PrintFCO( pFCO, pDisplayer );
                    pFCO->Release();
                }
                else
                {
                    TCOUT << "Object has no data associated with it." << std::endl;
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
            TCOUT << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( pIter->GetParentName() ) << std::endl;
        }
        //-----------------------------------------------------------------
        // ls
        //-----------------------------------------------------------------
        else if( verb.compare( _T("ls") ) == 0 )
        {
            int cnt = 0;
            for( pIter->SeekBegin(); ! pIter->Done(); pIter->Next(), cnt++ )
            {
                TCOUT << "[" << cnt ;
                if( pIter->CanDescend() )
                {
                    TCOUT << "]*\t" ;
                }
                else
                {
                    TCOUT << "]\t" ;
                }
                TCOUT << pIter->GetShortName() << std::endl;
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
                if( pIter->AtRoot() )
                {
                    TCOUT << "Can't ascend above root." << std::endl;
                }
                else
                {
                    TCOUT << "Ascending..." << std::endl;
                    pIter->Ascend();
                }
            }
            else
            {
                if( SeekTo( pIter, noun ) )
                {
                    if( pIter->CanDescend() )
                    {
                        TCOUT << "Descending into " << noun << std::endl;
                        pIter->Descend();
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
        // cg
        //-----------------------------------------------------------------
        else if( verb.compare( _T("cg") ) == 0 )
        {
            GetNoun(noun);

            cGenre::Genre newGenre = cGenreSwitcher::GetInstance()->StringToGenre( noun.c_str() );

            if (newGenre != cGenre::GENRE_INVALID)
            {
                dbIter.SeekToGenre( newGenre );
                if( !dbIter.Done() )
                {
                    TCOUT << _T("Changing to Genre ") << noun << std::endl;
                    //
                    // create a new db iter for the new genre (db iters can only be
                    // assocaited with a single genre :-( )
                    //
                    delete pIter;
                    pIter = new cDbDataSourceIter( &dbIter.GetDb(), newGenre );
                }
                else
                {
                    TCOUT << _T("Unable to find Genre ") << noun << std::endl;
                }
            }
            else
            {
                TCOUT << _T("Invalid Genre ") << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // pwg
        //-----------------------------------------------------------------
        else if( verb.compare( _T("pwg") ) == 0 )
        {
            TCOUT << _T("Current Genre: ") << cGenreSwitcher::GetInstance()->GenreToString( (cGenre::Genre)dbIter.GetGenre(), true ) << std::endl;
        }

        //-----------------------------------------------------------------
        // mkdir
        //-----------------------------------------------------------------
        else if( verb.compare( _T("mkdir") ) == 0 )
        {
            GetNoun(noun);
            TCOUT << "Making a child of " << noun << std::endl;
            if( pIter->SeekTo( noun.c_str() ) )
            {
                pIter->AddChildArray();
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
            if( pIter->SeekTo( noun.c_str() ) )
            {
                TCOUT << "Error: object already exists!" << std::endl;
            }
            else
            {
                pIter->AddFCO( noun, 0 );   // add a null fco for now
            }
        }
        //-----------------------------------------------------------------
        // rmdir
        //-----------------------------------------------------------------
        // TODO -- still needs to be implemented in the pIterator class!
        //
        else if( verb.compare( _T("rmdir") ) == 0 )
        {
            GetNoun(noun);
            TCOUT << "Removing the child of " << noun << std::endl;
            if( pIter->SeekTo( noun.c_str() ) )
            {
                //TODO -- check that it has an empty child
                pIter->RemoveChildArray();
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
            if( pIter->SeekTo( noun.c_str() ) )
            {
                if( pIter->CanDescend() )
                {
                    TCOUT << "Can't delete object; it still has children." << std::endl;
                }
                else
                {
                    pIter->RemoveFCO();
                }
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // help
        //-----------------------------------------------------------------
        if( verb.compare( _T("help") ) == 0 )
        {
            TCOUT   << _T("Commands: ")             << std::endl
                    << _T(" cd <dir_name>")         << std::endl
                    << _T(" pwd ")                  << std::endl
                    << _T(" ls ")                   << std::endl
                    << _T(" print <object_name>")   << std::endl
                    << _T(" cg (FS | NTFS | NTREG)")<< std::endl
                    << _T(" pwg")                   << std::endl
                    << _T(" quit")                  << std::endl;
        }

        // make sure the file is still valid...
        //
    }

    delete pIter;
    TCOUT << "Exiting..." << std::endl;
}
*/
