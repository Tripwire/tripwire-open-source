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
///////////////////////////////////////////////////////////////////////////////
// fspropdisplayer_t -- cFSPropDisplayer test driver

#include "fs/stdfs.h"
#include "fs/fspropdisplayer.h"
#include "fs/fspropcalc.h"
#include "fs/fsobject.h"
#include "twtest/test.h"
#include "core/serializerimpl.h"

class cTestFSPropDisplayer
{
public:
    static void Test();
};


void TestFSPropDisplayer()
{
    cTestFSPropDisplayer::Test();
}


void cTestFSPropDisplayer::Test()
{
    char   i;
    cDebug d("TestFSPropDisplayer");

    cFSPropDisplayer* pPD = new cFSPropDisplayer();

    for (i = 0; i < 26; i++)
    {
        TCHAR c;

        c = i + 'a';
        pPD->AddUsernameMapping(i, TSTRING(5, c));
        c = i + 'A';
        pPD->AddGroupnameMapping(i, TSTRING(5, c));
    }

    cFSPropDisplayer* pPDNew = new cFSPropDisplayer();

    for (i = 1; i < 26; i++)
    {
        TCHAR c;

        c = i + 'a';
        pPDNew->AddUsernameMapping(i, TSTRING(10, c));
        c = i + 'A';
        pPDNew->AddGroupnameMapping(i, TSTRING(10, c));
    }

    pPDNew->Merge(pPD);


    ////////////////////////
    // write pd
    cFileArchive outFile;
    outFile.OpenReadWrite(TwTestPath("tmp.pd").c_str());
    cSerializerImpl outSer(outFile, cSerializerImpl::S_WRITE);

    outSer.Init();

    pPD->Write(&outSer);
    outSer.Finit();

    outFile.Close();


    ////////////////////////
    // read pd
    cFileArchive inFile;
    inFile.OpenRead(TwTestPath("tmp.pd").c_str());
    cSerializerImpl inSer(inFile, cSerializerImpl::S_READ);

    cFSPropDisplayer* pPDRead = new cFSPropDisplayer();
    inSer.Init();

    pPDRead->Read(&inSer);
    inSer.Finit();

    TEST(*pPD == *pPDRead);

    TSTRING strRet;
    for (i = 0; i < 26; i++)
    {
        if (pPDNew->GetUsername(i, strRet))
            d.TraceDebug("username for uid %d: <%s>\n", i, strRet.c_str());
        else
            d.TraceDebug("no username for uid %d\n", i);

        if (pPDNew->GetGroupname(i, strRet))
            d.TraceDebug("groupname for gid %d: <%s>\n", i, strRet.c_str());
        else
            d.TraceDebug("no groupname for gid %d\n", i);

        d.TraceDebug("\n");
    }

    delete pPD;
    delete pPDNew;
    delete pPDRead;

    return;
}

void RegisterSuite_FSPropDisplayer()
{
    RegisterTest("FSPropDisplayer", "Basic", TestFSPropDisplayer);
}
