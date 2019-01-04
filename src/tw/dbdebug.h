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
//dbdebug.h

#ifndef __DBDEBUG_H
#define __DBDEBUG_H

#ifndef __FCODATASOURCEITER_H
#include "fco/fcodatasourceiter.h"
#endif
#ifndef __HIERDATABASE_H
#include "db/hierdatabase.h"
#endif
#ifndef __SREFCOUNTOBJ_H
#include "core/srefcountobj.h"
#endif
#ifndef __HIERDBNODE_H
#include "db/hierdbnode.h"
#endif

// STL stuff...
#include <map>
#include <list>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
class cBlockFile;
class cFCODatabaseFileIter;
struct cDbDebug_i;
class cDbDataSourceIter;
struct DbMapNode;
class cFCOName;

/////////////////////////////////////////////////////////////////////////////////
// cDebugHierDb --
//      This derived class is a hack for accessing data that is usually protected.
//      A static_cast is used to morph this class back into a (more candid)
//      cHierDatabase..
/////////////////////////////////////////////////////////////////////////////////

class cDebugHierDb : public cHierDatabase
{
public:
    cDebugHierDb() : cHierDatabase()
    {
    }

    typedef std::vector<cBlockRecordArray> BlockArray;

    //
    // Methods for accessing the two data members of cBlockRecordFile that we
    // are interested in.
    cBlockFile* myGetBlockFile()
    {
        return GetBlockFile();
    }
    BlockArray* myGetBlockArray()
    {
        return GetBlockArray();
    }

private:
};

//////////////////////////////////////////////////////////////////////////////////
// An iterator for the cDebugHierDb object.  I need to get at some data
// in this object, too...
class cDebugHierDbIter : public cHierDatabaseIter
{
public:
    //
    // Type Defines:
    //
    typedef std::vector<cHierEntry> EntryArray;

    // Ctor, Dtor:
    ~cDebugHierDbIter()
    {
    }
    explicit cDebugHierDbIter(cHierDatabase* pDb) : cHierDatabaseIter(pDb)
    {
    }

    //
    // Methods for obtaining protected members of cHierDatabaseIter:
    //
    cHierAddr myGetCurrentAddr()
    {
        return GetCurrentAddr();
    }
    // We need this method to match the hierarchy information with that of the
    // quantum database.

    //
    // Methods for accessing data members:
    // TODO: Get rid of the stuff I don't end up using.
    //
    EntryArray::iterator& myGetEntryArrayIt()
    {
        return mIter;
    }
    EntryArray& myGetEntryArray()
    {
        return mEntries;
    }
    cHierArrayInfo& myGetArrayInfo()
    {
        return mInfo;
    }
    cHierAddr& myGetInfoAddr()
    {
        return mInfoAddr;
    }
    cHierDbPath& myGetCurrPath()
    {
        return mCurPath;
    }
};

/////////////////////////////////////////////////////////////////////////////////
// cDbDebug --
//      A tripwire mode for probing the hierarchical database
/////////////////////////////////////////////////////////////////////////////////
//TODO: Would I ever want to create this class on the stack?
class cDbDebug
{
public:
    //Ctor, Dtor
    cDbDebug();
    ~cDbDebug();

    static void Execute(cFCODatabaseFileIter& dbIter, const TSTRING& dbFile);
    void        MapQuantumDatabase(cDebugHierDb& db);
    // Traverses a blockfile and records all valid addresses
    void TraverseHierarchy(cDebugHierDbIter pIter);
    // Traverses a hierarchical database and "updates" the map for addresses that it finds.
    void MapHierDbNodes(std::map<std::pair<int, int>, int>& dbMap, std::pair<int, int>, cDebugHierDbIter&);
    // Changes the map for a given address, in order to record entries that were found in the hierarchy.
    void CongruencyTest(void);
    // Looks at the map to see if all entries have been accounted for.
    void OutputResults(void);
    // Gives a clean report only if the two database representations match exactly.

    //
    // The map that will be used to test for congruency between the quantum database
    // and the hierarchical database.
    //
    typedef std::map<std::pair<int, int>, int> hierDbMap;

    hierDbMap* GetHierDbMap(void);
    void       DisplayDbMap(void);
    // Displays the whole map TODO: Somehow the spacing in the output got messed up...
    //void          ManipDb             ( cFCODatabaseFileIter& dbIter );
    // A hook to DbExplore-like code.  For convenience only.

private:
    // (somewhat) Insulated implementation:
    cDbDebug_i* mpData;
};


#endif //__DBDEBUG_H
