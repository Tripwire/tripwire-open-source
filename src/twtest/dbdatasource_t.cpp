//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
// dbdatasource_t.cpp
#include "tw/stdtw.h"
#include "tw/dbdatasource.h"
#include "db/hierdatabase.h"
#include "core/fsservices.h"
#include "core/debug.h"
#include "core/error.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"
#include "fco/fco.h"
#include "twtest/test.h"
#include "fs/fsobject.h"

static void AddFile(cDbDataSourceIter& iter, const TSTRING& filename, bool with_data = false)
{
    if (iter.SeekTo(filename.c_str()))
        TCOUT << "Object " << filename << " already exists!" << std::endl;

    cFCOName fname(filename);

    if (with_data)
    {
        iFCO* pFCO = new cFSObject(fname);
        iter.AddFCO(filename, pFCO);
    }
    else
    {
        iter.AddFCO(filename, 0);
    }

    TEST(iter.HasFCOData() == with_data);
}

static void AddDirectory(cDbDataSourceIter& iter, const TSTRING& filename)
{
    if (iter.SeekTo(filename.c_str()))
        TCOUT << "Object " << filename << " already exists!" << std::endl;

    iter.AddFCO(filename, 0);
    iter.AddChildArray();

    TEST(iter.CanDescend());
}

static void RemoveDirectory(cDbDataSourceIter& iter, const TSTRING& filename)
{
    TCOUT << "Removing the child of " << filename << std::endl;
    if (iter.SeekTo(filename.c_str()))
    {
        iter.RemoveChildArray();
        iter.RemoveFCO();
    }
    else
    {
        TCOUT << "Unable to find object " << filename << std::endl;
    }
}

static void RemoveFile(cDbDataSourceIter& iter, const TSTRING& filename)
{
    TCOUT << "Removing object " << filename << std::endl;
    cFCOName fname(filename);
    iter.SeekToFCO(fname);

    if (iter.CanDescend())
    {
        TCOUT << "Can't delete object; it still has children." << std::endl;
    }
    else
    {
        iter.RemoveFCOData();
        iter.RemoveFCO();
    }
}

static void ChDir(cDbDataSourceIter& iter, const TSTRING& filename)
{
    if (filename.compare(_T("..")) == 0)
    {
        if (iter.AtRoot())
            TCOUT << "At root already" << std::endl;

        TCOUT << "Ascending..." << std::endl;
        iter.Ascend();
    }
    else
    {
        if (iter.SeekTo(filename.c_str()))
        {
            if (!iter.CanDescend())
                TCOUT << filename << " has no children; can't descend." << std::endl;

            TCOUT << "Descending into " << filename << std::endl;
            iter.Descend();
        }
        else
        {
            TCOUT << "Unable to find object " << filename << std::endl;
        }
    }
}

static void AssertData(cDbDataSourceIter& iter, const TSTRING& filename, bool should_have)
{
    bool exists = iter.SeekTo(filename.c_str());
    TEST(exists == should_have);

    if (exists)
    {
        bool has_data = iter.HasFCOData();
        TEST(has_data == should_have);

        if (has_data)
        {
            iFCO* pFCO = iter.CreateFCO();
            TEST(pFCO);
            TCOUT << "Roundtrip FCOName = " << pFCO->GetName().AsString() << std::endl;
            TSTRING expected = filename + "/";
            TEST(pFCO->GetName().AsString() == expected);
        }
    }
}

static void AssertExists(cDbDataSourceIter& iter, const TSTRING& filename, bool should_have)
{
    bool exists = iter.SeekTo(filename.c_str());
    TEST(exists == should_have);
}

static void AssertChildren(cDbDataSourceIter& iter, const TSTRING& filename, bool should_have)
{
    bool exists = iter.SeekTo(filename.c_str());

    if (exists)
    {
        bool has_children = iter.CanDescend();
        TEST(has_children == should_have);
    }
}

void TestDbDataSourceBasic()
{
    cHierDatabase db;
    std::string   dbpath = TwTestPath("test.db");
    db.Open(dbpath, 5, true);
    cDbDataSourceIter iter(&db);

    AddFile(iter, "file1", true);
    AddFile(iter, "file2", false);
    AddFile(iter, "file3", false);

    AddDirectory(iter, "dir1");
    AddDirectory(iter, "dir2");
    AddDirectory(iter, "dir3");

    AssertData(iter, "file1", true);

    ChDir(iter, "dir1");
    AddFile(iter, "dir1_file1");
    ChDir(iter, "..");

    RemoveFile(iter, "file1");
    RemoveFile(iter, "file2");

    AssertExists(iter, "file1", false);
    AssertExists(iter, "file2", false);
    AssertExists(iter, "file3", true);

    RemoveDirectory(iter, "dir2");

    AssertExists(iter, "dir1", true);
    AssertExists(iter, "dir2", false);
    AssertExists(iter, "dir3", true);

    AssertChildren(iter, "dir1", true);
    AssertChildren(iter, "dir3", true);
    AssertChildren(iter, "file3", false);

#ifdef DEBUG
    db.AssertAllBlocksValid();
#endif
}

void RegisterSuite_DbDataSource()
{
    RegisterTest("DbDataSource", "Basic", TestDbDataSourceBasic);
}
