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
// fcodatabasefile.h
//
#ifndef __FCODATABASEFILE_H
#define __FCODATABASEFILE_H

class cHierDatabase;
class iFCOSpec;
class cFileHeaderID;
class cFCODatabaseFileIter;
class eArchive;
class eSerializer;
class cElGamalSigPublicKey;
class cElGamalSigPrivateKey;

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __HIERDATABASE_H
#include "db/hierdatabase.h"
#endif
#ifndef __FCOSPECLIST_H
#include "fco/fcospeclist.h"
#endif
#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif
#ifndef __HEADERINFO_H
#include "tw/headerinfo.h"
#endif
#ifndef __ERROR_H
#include "core/error.h"
#endif
#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif

//
// TODO -- support encrypting each entry seperately
//
// TODO -- figure out some way to templatize this class based on encryption method
//

TSS_EXCEPTION(eFCODbFile, eError);
TSS_EXCEPTION(eFCODbFileTooBig, eFCODbFile);

//-----------------------------------------------------------------------------
// cFCODatabaseFile -- class that manages a set of databases (for different genres
//      stored encrypted on disk
//-----------------------------------------------------------------------------
class cFCODatabaseFile : public iTypedSerializable
{
public:
    cFCODatabaseFile();
    virtual ~cFCODatabaseFile();

    void    SetFileName(const TSTRING& name);
    TSTRING GetFileName() const;
    // the file name is only used in exception throwing; it is not necessary to set it.

    void AddGenre(cGenre::Genre genreId, cFCODatabaseFileIter* pIter = 0); //throw (eArchive)
        // if pIter is not null, then it is pointing at the new node. This asserts that the
        // genre doesn't currently exist

    cFCODbHeader& GetHeader();

    static const cFileHeaderID& GetFileHeaderID();

    ///////////////////////////////
    // serialization interface
    ///////////////////////////////
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)


    typedef cFCODatabaseFileIter iterator;
    //-------------------------------------------------------------------------
    // cEntry -- a single entry in the database -- represents a genre's database
    //      note that if the entry hasn't been loaded from disk yet, all of its
    //      data members are NULL
    //-------------------------------------------------------------------------
    struct tEntry
    {
        explicit tEntry(cGenre::Genre genre);
        // the ctor will get the appropriate database construction parameters
        // based on the genre number.
        ~tEntry()
        {
        }

        cHierDatabase     mDb; // the database;
        cFCODbGenreHeader mGenreHeader;
        cFCOSpecList      mSpecList; // the spec used to create the database
        cGenre::Genre     mGenre;    // the genre this is associated with
    };

private:
    cFCODatabaseFile(const cFCODatabaseFile& rhs); //not impl
    void operator=(const cFCODatabaseFile& rhs);   //not impl

    typedef std::vector<tEntry*> DbList;
    friend class cFCODatabaseFileIter;

    cFCODbHeader mHeader;
    DbList       mDbList;   // the list of databases
    TSTRING      mFileName; // for cosmetic purposes only

    DECLARE_TYPEDSERIALIZABLE()
};

//-----------------------------------------------------------------------------
// cFCODatabaseFileIter
//-----------------------------------------------------------------------------
class cFCODatabaseFileIter
{
public:
    explicit cFCODatabaseFileIter(cFCODatabaseFile& dbFile);

    void SeekBegin();
    void Next();
    bool Done() const;
    int  Size() const
    {
        return mDbFile.mDbList.size();
    }

    void SeekToGenre(cGenre::Genre genreId);
    // Done() is true if the genre doesn't exist
    void Remove();
    // removes the current node from the database file

    cGenre::Genre            GetGenre() const;
    cHierDatabase&           GetDb();
    const cHierDatabase&     GetDb() const;
    cFCOSpecList&            GetSpecList();
    const cFCOSpecList&      GetSpecList() const;
    cFCODbGenreHeader&       GetGenreHeader();
    const cFCODbGenreHeader& GetGenreHeader() const;

private:
    cFCODatabaseFile&                  mDbFile;
    cFCODatabaseFile::DbList::iterator mIter;

    friend class cFCODatabaseFile;
};

//#############################################################################
// inline implementation
//#############################################################################

inline cFCODbHeader& cFCODatabaseFile::GetHeader()
{
    return mHeader;
}

#endif //__FCODATABASEFILE_H
