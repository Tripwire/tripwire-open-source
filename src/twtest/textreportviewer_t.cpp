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
// textreportviewer_t.cpp -- tests textreportviewer
//

#include "tw/stdtw.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "tw/fcoreport.h"
#include "fco/fcospecimpl.h"
#include "fco/fcosetimpl.h"
#include "fs/fsobject.h"
#include "core/serializerimpl.h"
#include "core/archive.h"
#include "twtest/test.h"
#include "core/errorbucketimpl.h"
#include "tw/textreportviewer.h"
#include "fs/fspropset.h"
#include "fs/fspropcalc.h"
#include "core/fsservices.h"
#include <iostream>
#include <fstream>
#include "fco/fcospechelper.h"
#include "fco/fcospecattr.h"
#include "tw/fcoreportutil.h"
#include "tw/headerinfo.h"

//#ifdef FIXED_TRV_TEST_SUITE

void MakeFile(TSTRING& fcoNameMakeMe);
void MakeDir(const TCHAR* const lpszDirName);

#if 0 // Not used untill this test is fixed
// we use this instead of TraceContents() so we can test the report iterators.
static void TraceReport(const cFCOReport& r, cDebug& d)
{
    d.TraceDebug("Global Error Queue:\n");
    r.GetErrorQueue()->TraceContents();

    cFCOReportGenreIter genreIter(r);
    int                 genreCount = 0;

    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        d.TraceDebug("> Genre [%d]:\n", genreCount);

        cFCOReportSpecIter specIter(genreIter);
        int                ct = 0;

        for (specIter.SeekBegin(); !specIter.Done(); specIter.Next(), ct++)
        {
            d.TraceDebug(">>> Spec [%d]:\n", ct);
            TEST(specIter.GetSpec());
            specIter.GetSpec()->TraceContents();
            specIter.GetErrorQueue()->TraceContents();

            d.TraceDebug(">>> Added Files:\n");
            specIter.GetAddedSet()->TraceContents();
            d.TraceDebug(">>> Removed Files:\n");
            specIter.GetRemovedSet()->TraceContents();

            // trace out changed files
            cFCOReportChangeIter changeIter(specIter);
            int                  changeCtr = 0;
            for (changeIter.SeekBegin(); !changeIter.Done(); changeIter.Next(), changeCtr++)
            {
                d.TraceDebug(">>>>> Changed fco [%d]\n", changeCtr);
                d.TraceDebug(">>>>>   Old FCO:\n");
                changeIter.GetOld()->TraceContents();
                d.TraceDebug(">>>>>   New FCO:\n");
                changeIter.GetNew()->TraceContents();
                changeIter.GetChangeVector().TraceContents();
            }
        }
    }
}
#endif

/*
 //
 // basic functionality
 //
 void DisplayReportAndHaveUserUpdateIt( const TSTRING& edName, ReportingLevel level = FULL_REPORT ); //throw (eFSServices, eTextReportViewer, eInternal);
 // outputs the given report to a temp file, opens an editor, has
 // the user view changes to the database, and, by selecting FCO entries,
 // chooses which changes to write to the database.  Unchecked entries
 // are removed from the report
 // edName is the name of the editor to use to update the report
 
 virtual void PrintTextReport( const TSTRING& strFilename, ReportingLevel level = FULL_REPORT ); //throw (eTextReportViewer);
 // if strFilename is "-", will print to TCOUT
 virtual void PrintTextReport( TOSTREAM& ostr, ReportingLevel level = FULL_REPORT ); //throw (eTextReportViewer);
 // prints the report to the specified ostream
 */


void TestTextReportViewer()
{
    skip("TestTextReportViewer needs to be cleaned up & fixed, currently disabled");

#if 0
    cFCOReport  report;
    cFCOReportGenreIter genreIter(report);
    cFCOReportSpecIter specIter(genreIter);

    cDebug d("TestFCOReport");
    
    cFCOName fcoNameSpec1;
    cFCOName fcoNameSpec2;
    TSTRING  fcoNameTempFile;
    try
    {
        iFSServices* pFSServices = iFSServices::GetInstance();
        TEST( pFSServices );

        TSTRING fcoNameTempDir;
        pFSServices->GetTempDirName( fcoNameTempDir );
        
        fcoNameSpec1 = fcoNameTempDir += _T("SPEC1/");
        fcoNameSpec2 = fcoNameTempDir += _T("SPEC2/");
        
        tw_mkdir( fcoNameTempDir.c_str(), 0777 );
        tw_mkdir( fcoNameSpec1.AsString().c_str(), 0777 );
        tw_mkdir( fcoNameSpec2.AsString().c_str(), 0777 );
        
        fcoNameTempFile = fcoNameTempDir += _T("twtempXXXXXX");
        pFSServices->MakeTempFilename( fcoNameTempFile );
    }
    catch(const eFSServices& /* e */)
    {
        // TODO: properly handle error
        TEST( false );
    }
 
    // need two prop calcs because.....
    // if cFSPropCalc::VisitFSObject succeeds, cFSPropCalc stores the FCO in
    // an internal set (why, I don't know), and the cFCOSet TESTs that the same 
    // FCO isn't inserted more than once.  But since we visit changed FCOs twice 
    // in this test routine, we need two calcs.  Make sense?  Oh, well.
    cFSPropCalc* pPropCalc = new cFSPropCalc;
    cFSPropCalc* pPropCalc2 = new cFSPropCalc;
 
    
    cFCOSpecStopPointSet *pStopPts = new cFCOSpecStopPointSet;
    cFCOSpecImpl*    pSpec          = new cFCOSpecImpl( fcoNameSpec1.AsString(), NULL, pStopPts);
    cFCOSpecAttr* pAttr         = new cFCOSpecAttr;
    
 
    cFCOPropVector v;    
    for( int i = 0; i < 32; i++ )
        v.AddItem( i );
    
    pPropCalc->SetPropVector(v);
    pPropCalc2->SetPropVector(v);

    TSTRING fcoNameMakeMe;
    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/added_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  addedFCO        = new cFSObject( cFCOName(fcoNameMakeMe));
    pPropCalc->VisitFSObject( *addedFCO );

    
    // MakeTempFile can't handle strings with escaped quotes, so we'll have to do this ourselves
    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/\"quoted\\_and_backslashed_file1\"");    
    //TOFSTREAM file1( fcoNameMakeMe.c_str() );
    //TEST( file1 );
    //file1.close();

  
    cFSObject*  addedFCO2       = new cFSObject( cFCOName(fcoNameMakeMe) );
    //pPropCalc->VisitFSObject( *addedFCO2 );
    
    
    // MakeTempFile can't handle strings with escaped quotes, so we'll have to do this ourselves
    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/quoted_file\"2\"XXXXXX");
    //TOFSTREAM file2( fcoNameMakeMe.c_str() );
    //TEST( file2 );
    //file2.close();

    cFSObject*  addedFCO3       = new cFSObject( cFCOName(fcoNameMakeMe) );
    //pPropCalc->VisitFSObject( *addedFCO3 );

    
    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/removed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  removedFCO      = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *removedFCO );

    
    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/removed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  removedFCO2     = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *removedFCO2 );
    

    pSpec->SetStartPoint( fcoNameSpec1 );
    pAttr->SetName( fcoNameSpec1.AsString() );
    pAttr->SetSeverity(53);
    pStopPts->Add( cFCOName( fcoNameSpec1.AsString() + _T("/End1")) );
    report.AddSpec(0x00020001, pSpec, pAttr, &specIter); // TODO:bam - use cFS::Genre
    pAttr->Release();    

    TEST(specIter.GetAddedSet());
    TEST(specIter.GetRemovedSet());

    specIter.GetAddedSet()->Insert(addedFCO);
    specIter.GetAddedSet()->Insert(addedFCO2);
    specIter.GetAddedSet()->Insert(addedFCO3);
    specIter.GetRemovedSet()->Insert(removedFCO);
    specIter.GetRemovedSet()->Insert(removedFCO2);

    // make changed FCO1
    cFCOPropVector changedPropVector1;

    
    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/changed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  oldChangedFCO   = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *oldChangedFCO );
    (static_cast<cFSPropSet*> (oldChangedFCO->GetPropSet()))->SetSize(123);
    //(static_cast<cFSPropSet*> (oldChangedFCO->GetPropSet()))->SetUID(_T("old"));

    cFSObject*  newChangedFCO   = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc2->VisitFSObject( *newChangedFCO );
    (static_cast<cFSPropSet*> (newChangedFCO->GetPropSet()))->SetSize(666);
    //(static_cast<cFSPropSet*> (newChangedFCO->GetPropSet()))->SetUID(_T("new"));

    changedPropVector1.AddItem(cFSPropSet::PROP_SIZE);
    changedPropVector1.AddItem(cFSPropSet::PROP_UID);
    report.AddChangedFCO(specIter, oldChangedFCO, newChangedFCO, changedPropVector1);

    
    // make changed FCO2
    cFCOPropVector changedPropVector2;

    fcoNameMakeMe = fcoNameSpec1.AsString() + _T("/changed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  oldChangedFCO2  = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *oldChangedFCO2 );
    
    //(static_cast<cFSPropSet*> (oldChangedFCO2->GetPropSet()))->SetGSID( _T("S-1-1-0") );

    cFSObject*  newChangedFCO2  = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc2->VisitFSObject( *newChangedFCO2 );
    //(static_cast<cFSPropSet*> (newChangedFCO2->GetPropSet()))->SetGSID( _T("S-1-1-1") );

    //changedPropVector2.AddItem(cFSPropSet::PROP_GSID);
    report.AddChangedFCO(specIter, oldChangedFCO2, newChangedFCO2, changedPropVector2);

    // add some errors
//    report.GetErrorQueue()->AddError(eError(_T("this is a general error")));
 //   report.GetErrorQueue()->AddError(eError(_T("this too is a general error")));
        

    cFCOSpecStopPointSet *pStopPts2     = new cFCOSpecStopPointSet;
    cFCOSpecImpl*       pSpec2              = new cFCOSpecImpl( fcoNameSpec2.AsString(), NULL, pStopPts2);
    cFCOSpecAttr*   pAttr2              = new cFCOSpecAttr;

    fcoNameMakeMe = fcoNameSpec2.AsString() + _T("/added_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  addedFCO5       = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *addedFCO5 );
    
    fcoNameMakeMe = fcoNameSpec2.AsString() + _T("/removed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  removedFCO5     = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *removedFCO5 );

    pSpec2->SetStartPoint( fcoNameSpec2 );    
    pAttr2->SetName( fcoNameSpec2.AsString() );
    pAttr2->SetSeverity(64);
    pStopPts2->Add( cFCOName( fcoNameSpec2.AsString() + _T("/End2") ) );
    report.AddSpec(0x00020001, pSpec2, pAttr2, &specIter); // TODO:bam -- use cFS::Genre
    pAttr2->Release();
    
    specIter.GetAddedSet()->Insert(addedFCO5);
    specIter.GetRemovedSet()->Insert(removedFCO5);

    

    // make changed FCO3
    cFCOPropVector changedPropVector3;
    
    fcoNameMakeMe = fcoNameSpec2.AsString() + _T("/changed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  oldChangedFCO3  = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *oldChangedFCO3 );
    (static_cast<cFSPropSet*> (oldChangedFCO3->GetPropSet()))->SetBlockSize(313222);
    
    cFSObject*  newChangedFCO3  = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc2->VisitFSObject( *newChangedFCO3 );
    (static_cast<cFSPropSet*> (newChangedFCO3->GetPropSet()))->SetBlockSize(22213145);

    changedPropVector3.AddItem(cFSPropSet::PROP_BLOCK_SIZE);
    report.AddChangedFCO(specIter, oldChangedFCO3, newChangedFCO3, changedPropVector3);

    // make changed FCO4
    cFCOPropVector changedPropVector4;
    
    fcoNameMakeMe = fcoNameSpec2.AsString() + _T("/changed_fileXXXXXX");
    MakeFile( fcoNameMakeMe );
    cFSObject*  oldChangedFCO4  = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc->VisitFSObject( *oldChangedFCO4 );
    (static_cast<cFSPropSet*> (oldChangedFCO4->GetPropSet()))->SetSize(9104498);
    (static_cast<cFSPropSet*> (oldChangedFCO4->GetPropSet()))->SetMode( S_IREAD | S_IWRITE );    
    
    cFSObject*  newChangedFCO4  = new cFSObject( cFCOName(fcoNameMakeMe) );
    pPropCalc2->VisitFSObject( *newChangedFCO4 );
    (static_cast<cFSPropSet*> (newChangedFCO4->GetPropSet()))->SetSize(66);
    (static_cast<cFSPropSet*> (newChangedFCO4->GetPropSet()))->SetMode( S_IREAD | S_IWRITE | S_IEXEC );

    changedPropVector4.AddItem(cFSPropSet::PROP_SIZE);
    changedPropVector4.AddItem(cFSPropSet::PROP_MODE);
    report.AddChangedFCO(specIter, oldChangedFCO4, newChangedFCO4, changedPropVector4);

    specIter.SeekBegin();
    specIter.Next();
    //specIter.GetErrorQueue()->AddError(2, "this is an \"/etc2\" spec error",NULL);

    d.TraceDebug(_T("\n======================================================\nStart PrintTextReport...\n======================================================\n\n\n"));
        
    TSTRING tstrEmpty( _T("") );
    cFCOReportHeader rhi;
    cFCOReportUtil::FinalizeReport( report );
    cTextReportViewer trv(rhi, report);
    trv.DisplayReportAndHaveUserUpdateIt( _T("") );

            // test writing of USID
            cFileArchive outFile;
            outFile.OpenReadWrite(_T("tmp.twr"));
            cSerializerImpl outSer(outFile, cSerializerImpl::S_WRITE);

            //TraceReport(report, d);
            outSer.Init();
            outSer.WriteObject(&report);
            outSer.Finit();

            outFile.Close();

            cFileArchive inFile;
            inFile.OpenRead(_T("tmp.twr"));
            cSerializerImpl inSer(inFile, cSerializerImpl::S_READ);

            cFCOReport inReport;

            inSer.Init();
            inSer.ReadObject(&inReport);
            inSer.Finit();

            d.TraceDebug("Read in serialized report:\n");
            //TraceReport(inReport, d);
            trv.PrintTextReport(TSTRING( TwTestPath("test2.txt" ) ) );

            //TODO: this does not work any more
            //trv.LaunchEditorOnFile( TSTRING( TEMP_DIR _T("/test2.txt") ), _T("") );


    // look at results
    trv.PrintTextReport(fcoNameTempFile );
    //TODO: this does not work any more
    //cTextReportViewer::LaunchEditorOnFile( fcoNameTempFile, _T("") );

    
    iFSServices* pFSServices = iFSServices::GetInstance();
    TEST( pFSServices );
    pFSServices->FileDelete( addedFCO->GetName().AsString() );
    pFSServices->FileDelete( addedFCO2->GetName().AsString() );
    pFSServices->FileDelete( addedFCO3->GetName().AsString() );
    pFSServices->FileDelete( addedFCO5->GetName().AsString() );
    pFSServices->FileDelete( removedFCO->GetName().AsString() );
    pFSServices->FileDelete( removedFCO2->GetName().AsString() );
    pFSServices->FileDelete( removedFCO5->GetName().AsString() );
    pFSServices->FileDelete( newChangedFCO->GetName().AsString() );
    pFSServices->FileDelete( newChangedFCO2->GetName().AsString() );
    pFSServices->FileDelete( newChangedFCO3->GetName().AsString() );
    pFSServices->FileDelete( newChangedFCO4->GetName().AsString() );
    pFSServices->FileDelete( fcoNameTempFile );

    // don't remove TEMP_DIR since other people may be using it
    rmdir( fcoNameSpec1.AsString().c_str() );
    rmdir( fcoNameSpec2.AsString().c_str() );

    pSpec->Release();
    pSpec2->Release();
    addedFCO->Release();
    addedFCO2->Release();
    addedFCO3->Release();
    addedFCO5->Release();
    removedFCO->Release();
    removedFCO2->Release();
    removedFCO5->Release();
    delete pPropCalc;
    delete pPropCalc2;
    oldChangedFCO->Release();
    newChangedFCO->Release();
    oldChangedFCO2->Release();
    newChangedFCO2->Release();
    oldChangedFCO3->Release();
    newChangedFCO3->Release();
    oldChangedFCO4->Release();
    newChangedFCO4->Release();
#endif

    return;
}

void MakeFile(TSTRING& strNameMakeMe)
{
    try
    {
        iFSServices* pFSServices = iFSServices::GetInstance();
        TEST(pFSServices);
        pFSServices->MakeTempFilename(strNameMakeMe);

        std::string strA;
        for (TSTRING::iterator i = strNameMakeMe.begin(); i != strNameMakeMe.end(); ++i)
        {
            char ach[6];
            TEST(MB_CUR_MAX <= 6);

            int n = wctomb(ach, *i);
            TEST(n != -1);

            for (int j = 0; j < n; j++)
                strA += ach[j];
        }

        TOFSTREAM file(strA.c_str());
        TEST(file);
        file.close();
    }
    catch (const eFSServices& e)
    {
        TEST(false);
    }
    catch (...)
    {
        TEST(false);
    }
}

void MakeDir(const TCHAR* const lpszDirName)
{
    TEST(0 == tw_mkdir(lpszDirName, 0777))
}

//#endif //FIXED_TRV_TEST_SUITE

void RegisterSuite_TextReportViewer()
{
    RegisterTest("TextReportViewer", "Basic", TestTextReportViewer);
}
