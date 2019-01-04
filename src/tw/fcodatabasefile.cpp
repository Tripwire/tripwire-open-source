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
// fcodatabasefile.cpp
//

#include "stdtw.h"
#include "fcodatabasefile.h"
#include "core/archive.h"
#include "core/serializer.h"
#include "core/serializerutil.h"
#include "fco/fcogenre.h"
#include "fco/genreswitcher.h"
#include "fco/twfactory.h"
#include "fco/fconameinfo.h"
#include "core/fileheader.h"
#include "core/fsservices.h"
#include "db/blockrecordfile.h"


// TODO: May localizable strings need to be moved to string table

IMPLEMENT_TYPEDSERIALIZABLE(cFCODatabaseFile, _T("cFCODatabaseFile"), 0, 1)


cFCODatabaseFile::tEntry::tEntry(cGenre::Genre genre)
    //TODO -- ugh, this sucks! I need to add another interface to the database!
    : mDb(cGenreSwitcher::GetInstance()->GetFactoryForGenre((cGenre::Genre)genre)->GetNameInfo()->IsCaseSensitive(),
          cGenreSwitcher::GetInstance()->GetFactoryForGenre((cGenre::Genre)genre)->GetNameInfo()->GetDelimitingChar()),
      mGenre(genre)
{
}

cFCODatabaseFile::cFCODatabaseFile()
#ifdef DEBUG
    : mFileName(_T("Unknown file name"))
#else
    : mFileName(_T("")) // If we don't know the filename, lets just not have one in release mode.
#endif
{
}

cFCODatabaseFile::~cFCODatabaseFile()
{
    for (DbList::iterator i = mDbList.begin(); i != mDbList.end(); ++i)
    {
        delete *i;
    }
    mDbList.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
void cFCODatabaseFile::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("Database Read")));

    //
    // read the db header..
    //
    pSerializer->ReadObject(&mHeader);

    int32_t numGenre;
    pSerializer->ReadInt32(numGenre);
    for (int i = 0; i < numGenre; i++)
    {
        // read the genre number and throw if it is incorrect
        //
        int32_t iGenre;
        pSerializer->ReadInt32(iGenre);
        cGenre::Genre genre = (cGenre::Genre)iGenre;

        if (!cGenreSwitcher::GetInstance()->IsGenreRegistered(genre))
        {
            throw eSerializerInputStreamFmt(_T("Encountered unknown genre.  Can not read database on this platform."),
                                            mFileName,
                                            eSerializer::TY_FILE);
        }

        mDbList.push_back(new tEntry(genre));
        tEntry& entry = *(mDbList.back());
        //
        // read the db genre header..
        //
        pSerializer->ReadObject(&entry.mGenreHeader);
        //
        // get the spec list
        //
        pSerializer->ReadObject(&entry.mSpecList);
        //
        // get the database data
        //
        int32_t fileSize;
        pSerializer->ReadInt32(fileSize);
        //
        // write the hier database into a temp file...
        //
        cLockedTemporaryFileArchive* pArch = new cLockedTemporaryFileArchive();
        pArch->OpenReadWrite();

        cSerializerUtil::Copy(pArch, pSerializer, fileSize);
        //
        // associate the database with this file...
        //
        entry.mDb.Open(pArch);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
void cFCODatabaseFile::Write(iSerializer* pSerializer) const //throw( eFCODbFileTooBig )
{
    //
    // write the db header..
    //
    pSerializer->WriteObject(&mHeader);

    // iterate through the genres, writing each one...
    //
    pSerializer->WriteInt32(mDbList.size());
    //
    // TODO -- the database is not really const-correct; therefore I have the sick casts below...
    //
    for (DbList::iterator i = const_cast<DbList*>(&mDbList)->begin(); i != const_cast<DbList*>(&mDbList)->end(); ++i)
    {
        pSerializer->WriteInt32((*i)->mGenre);
        pSerializer->WriteObject(&(*i)->mGenreHeader);
        pSerializer->WriteObject(&(*i)->mSpecList);
        //
        // copy the database's data into the archive...
        //
        (*i)->mDb.Flush();
        cBidirArchive* pDbArch = (*i)->mDb.GetArchive();
        pDbArch->Seek(0, cBidirArchive::BEGINNING);

        // TODO:BAM -- eventually we should write it as a 64 bit,
        // but that would change the db format
        if (pDbArch->Length() > TSS_INT32_MAX)
            throw eFCODbFileTooBig();

        pSerializer->WriteInt32(static_cast<int32_t>(pDbArch->Length()));
        cSerializerUtil::Copy(pSerializer, pDbArch, pDbArch->Length());
    }
}

///////////////////////////////////////////////////////////////////////////////
// GetFileHeaderID()
///////////////////////////////////////////////////////////////////////////////
static cFileHeaderID gFCODatabaseFileHeaderID(_T("cFCODatabaseFile"));

const cFileHeaderID& cFCODatabaseFile::GetFileHeaderID()
{
    return gFCODatabaseFileHeaderID;
}


///////////////////////////////////////////////////////////////////////////////
// AddGenre
///////////////////////////////////////////////////////////////////////////////
void cFCODatabaseFile::AddGenre(cGenre::Genre genreId, cFCODatabaseFileIter* pIter)
{
    //
    // first, lets make sure this genre doesn't exist...
    //
    for (DbList::iterator i = mDbList.begin(); i != mDbList.end(); ++i)
    {
        if ((*i)->mGenre == genreId)
        {
            ASSERT(false);
            return;
        }
    }
    //
    // just add it to the front; I don't care about order
    //
    mDbList.push_back(new tEntry(genreId));
    if (pIter)
    {
        pIter->mIter = mDbList.end() - 1;
    }
    //
    // create a new prop displayer for the header.
    //
    mDbList.back()->mGenreHeader.SetPropDisplayer(cGenreSwitcher::GetInstance()
                                                      ->GetFactoryForGenre((cGenre::Genre)mDbList.back()->mGenre)
                                                      ->CreatePropDisplayer());
    //
    // open the database
    //
    //
    // the hier database will own destroying this...
    //
    cLockedTemporaryFileArchive* pArch = new cLockedTemporaryFileArchive();
    pArch->OpenReadWrite();
    mDbList.back()->mDb.Open(pArch);
}

void cFCODatabaseFile::SetFileName(const TSTRING& name)
{
    mFileName = name;
}

TSTRING cFCODatabaseFile::GetFileName() const
{
    return mFileName;
}

//-----------------------------------------------------------------------------
// cFCODatabaseFileIter
//-----------------------------------------------------------------------------
cFCODatabaseFileIter::cFCODatabaseFileIter(cFCODatabaseFile& dbFile) : mDbFile(dbFile)
{
    SeekBegin();
}

void cFCODatabaseFileIter::SeekBegin()
{
    mIter = mDbFile.mDbList.begin();
}

void cFCODatabaseFileIter::Next()
{
    ++mIter;
}

bool cFCODatabaseFileIter::Done() const
{
    return (mIter == mDbFile.mDbList.end());
}

void cFCODatabaseFileIter::SeekToGenre(cGenre::Genre genreId)
{
    for (SeekBegin(); !Done(); Next())
    {
        if (GetGenre() == genreId)
            return;
    }
}

void cFCODatabaseFileIter::Remove()
{
    ASSERT(!Done());
    if (!Done())
    {
        delete *mIter;
        mIter = mDbFile.mDbList.erase(mIter);
    }
}

cGenre::Genre cFCODatabaseFileIter::GetGenre() const
{
    ASSERT(!Done());
    return (*mIter)->mGenre;
}

cHierDatabase& cFCODatabaseFileIter::GetDb()
{
    ASSERT(!Done());
    return (*mIter)->mDb;
}

const cHierDatabase& cFCODatabaseFileIter::GetDb() const
{
    ASSERT(!Done());
    return (*mIter)->mDb;
}

cFCOSpecList& cFCODatabaseFileIter::GetSpecList()
{
    ASSERT(!Done());
    return (*mIter)->mSpecList;
}

const cFCOSpecList& cFCODatabaseFileIter::GetSpecList() const
{
    ASSERT(!Done());
    return (*mIter)->mSpecList;
}

const cFCODbGenreHeader& cFCODatabaseFileIter::GetGenreHeader() const
{
    ASSERT(!Done());
    return (*mIter)->mGenreHeader;
}

cFCODbGenreHeader& cFCODatabaseFileIter::GetGenreHeader()
{
    ASSERT(!Done());
    return (*mIter)->mGenreHeader;
}
