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
// textdbviewer.cpp -- implementation for dbviewer class
//

//=========================================================================
// DIRECTIVES
//=========================================================================


//=========================================================================
// INCLUDES
//=========================================================================
#include "stdtw.h"

#include <fstream>
#include <iomanip>

#include "core/tchar.h"
#include "textdbviewer.h"
#include "core/debug.h"
#include "fcodatabasefile.h"
#include "dbdatasource.h"
#include "fco/fcospec.h"
#include "fco/iterproxy.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"
#include "core/fsservices.h"
#include "fco/fcospecattr.h"
#include "core/timeconvert.h"
#include "fco/fcopropdisplayer.h"
#include "headerinfo.h"
#include "fco/genreswitcher.h"
#include "core/twlocale.h"
#include "fco/twfactory.h"
#include "fco/fconametranslator.h"
#include "fco/fcoundefprop.h"
#include "tw/twstrings.h"
#include "core/displayencoder.h"

//=========================================================================
// USING NAMESPACE
//=========================================================================

using namespace std;

//=========================================================================
// GLOBALS
//=========================================================================

static const TCHAR* g_sz79Dashes =
    _T("-------------------------------------------------------------------------------");
static const TCHAR* g_sz79Equals =
    _T("===============================================================================");

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================
static void InitOStream(TOSTREAM* pOut);
static void DisplayFCOProps(const iFCO* const pFCO, const iFCOPropDisplayer* pPD, TOSTREAM* pOut);

static void OutputDatabaseHeader(const cFCODbHeader& dbHeader, TOSTREAM* pOut);
// Currently we don't have a rule summary.  See function definition below
//static void OutputRulesSummary    ( const cFCODatabaseFileIter& genreIter, const iFCONameTranslator* pNT, TOSTREAM* pOut );


static void OutputSectionDelimiter(int nString, TOSTREAM* pOut);
static void OutputGenreDelimiter(cGenre::Genre g, TOSTREAM* pOut);

static void OutputObjectSummary(cFCODatabaseFile& rd, TOSTREAM* pOut, int margin);
static void OutputObjectDetail(cFCODatabaseFile& rd, TOSTREAM* pOut);

static void
            OutputObjectSummary(cFCODatabaseFileIter& genreIter, const iFCONameTranslator* pNT, TOSTREAM* pOut, int margin);
static void OutputObjectDetail(cFCODatabaseFileIter& genreIter, const iFCONameTranslator* pNT, TOSTREAM* pOut);


static void OutputIterChildren(cDbDataSourceIter         dbIter,
                               const iFCOPropDisplayer*  pPD,
                               const iFCONameTranslator* pNT,
                               TOSTREAM*                 pOut,
                               bool                      fDetails);
static void OutputIterPeers(cDbDataSourceIter         dbIter,
                            const iFCOPropDisplayer*  pPD,
                            const iFCONameTranslator* pNT,
                            TOSTREAM*                 pOut,
                            bool                      fDetails);
//static void OutputIter          ( cDbDataSourceIter& dbIter, const iFCOPropDisplayer* pPD, TOSTREAM* pOut, bool fDetails );
static void PrintFCOShort(
    const iFCO* pFCO, const iFCOPropDisplayer* pPD, const iFCONameTranslator* pNT, TOSTREAM* pOut, int margin);
//static void OutputSpecHeader( const cFCODbSpecIter& dbi, TOSTREAM* pOut );
//static void OutputDetailHeader( const cFCODbSpecIter& dbi, int nObjects, TOSTREAM* pOut );

static TSTRING util_Encode(const TSTRING& sIn)
{
    static cDisplayEncoder e;
    TSTRING                sOut = sIn;
    e.Encode(sOut);
    return sOut;
}

//=========================================================================
// METHOD CODE
//=========================================================================

void cTextDBViewer::PrintDB(cFCODatabaseFile& rd, const TSTRING& strFilename, DbVerbosity verbosity)
{
    TOSTREAM* pOut;
    TOFSTREAM fileOut;
    bool      fIsFile = false;

    if (strFilename == _T("-"))
    {
        pOut = &TCOUT;
    }
    else
    {
        //Gonna have to insert a lame hack here, since ostr.open DEMANDS a const char*!!
        fileOut.open(strFilename.c_str());

        if (fileOut.is_open())
        {
            pOut    = &fileOut;
            fIsFile = true;
        }
        else
        {
            ASSERT(false);
            throw eArchiveOpen(strFilename, iFSServices::GetInstance()->GetErrString());
        }
    }

    InitOStream(pOut);

    //
    // output header, just like it sez....
    //
    OutputDatabaseHeader(rd.GetHeader(), pOut);


    if (verbosity > SUMMARY)
    {
        //TODO: OutputRulesSummary() would go here

        OutputObjectSummary(rd, pOut, DETAILS_MARGIN);

        if (verbosity == VERBOSE)
        {
            OutputObjectDetail(rd, pOut);
        }
    }

    // we're done
    (*pOut) << g_sz79Dashes << endl;
    (*pOut) << TSS_GetString(cTW, tw::STR_END_OF_DB) << endl << endl;
    (*pOut) << TSS_GetString(cTW, tw::STR_COPYRIGHT) << endl;

    if (fIsFile)
        static_cast<TOFSTREAM*>(pOut)->close();

    return;
}


///////////////////////////////////////////////////////////////////////////////
// OutputFCO
///////////////////////////////////////////////////////////////////////////////
void cTextDBViewer::OutputFCO(cDbDataSourceIter&        dbIter,
                              const iFCOPropDisplayer*  pPD,
                              const iFCONameTranslator* pNT,
                              TOSTREAM*                 pOut,
                              bool                      fDetails)
{
    InitOStream(pOut);

    if (dbIter.HasFCOData())
    {
        iFCO* pFCO = dbIter.CreateFCO();

        if (fDetails)
        {
            (*pOut) << TSS_GetString(cTW, tw::STR_OBJECT_NAME) << _T(" ")
                    << pNT->ToStringDisplay(pFCO->GetName()).c_str() << endl
                    << endl;
            DisplayFCOProps(pFCO, pPD, pOut);
            (*pOut) << endl << endl;
        }
        else
            PrintFCOShort(pFCO, pPD, pNT, pOut, DETAILS_MARGIN);

        pFCO->Release();
    }
}


//=========================================================================
//  UTIL FUNCTION CODE
//=========================================================================

void InitOStream(TOSTREAM* pOut)
{
    // align left
    (*pOut).flags(((*pOut).flags() & ~std::ios::adjustfield) | std::ios::left);
}

///////////////////////////////////////////////////////////////////////////////
// PrintFCOShort
///////////////////////////////////////////////////////////////////////////////
static void
PrintFCOShort(const iFCO* pFCO, const iFCOPropDisplayer* pPD, const iFCONameTranslator* pNT, TOSTREAM* pOut, int margin)
{
    ASSERT(pOut != 0);

    TSTRING strDetails;

    //
    // output "<name of FCO>"\n\t<details of FCO>"
    //
    (*pOut) << _T(" ");
    (*pOut) << pNT->ToStringDisplay(pFCO->GetName()).c_str();
    (*pOut) << endl;
    (*pOut) << setw(margin) << _T("");
    pPD->GetDetails(pFCO, strDetails);
    (*pOut) << strDetails;
    (*pOut) << _T("\n");
}


static void OutputIterChildren(cDbDataSourceIter         dbIter,
                               const iFCOPropDisplayer*  pPD,
                               const iFCONameTranslator* pNT,
                               TOSTREAM*                 pOut,
                               bool                      fDetails)
{
    dbIter.Descend();
    OutputIterPeers(dbIter, pPD, pNT, pOut, fDetails);
}

static void OutputIterPeers(cDbDataSourceIter         dbIter,
                            const iFCOPropDisplayer*  pPD,
                            const iFCONameTranslator* pNT,
                            TOSTREAM*                 pOut,
                            bool                      fDetails)
{
    for (dbIter.SeekBegin(); !dbIter.Done(); dbIter.Next())
    {
        // TODO: this obviously needs to be expanded upon.
        cTextDBViewer::OutputFCO(dbIter, pPD, pNT, pOut, fDetails);

        if (dbIter.CanDescend())
            OutputIterChildren(dbIter, pPD, pNT, pOut, fDetails);
    }
}


static void OutputDatabaseHeader(const cFCODbHeader& dbHeader, TOSTREAM* pOut)
{
    const int headerColumnWidth = 30;
    (*pOut) << TSS_GetString(cTW, tw::STR_DBPRINT_TITLE) << endl << endl;

    (*pOut).width(headerColumnWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_DB_GENERATED_BY) << util_Encode(dbHeader.GetCreator()) << endl;

    TSTRING tstrDummy;
    int64_t i64CreateTime = dbHeader.GetCreationTime();
    (*pOut).width(headerColumnWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_DB_CREATED_ON) << cTWLocale::FormatTime(i64CreateTime, tstrDummy).c_str()
            << endl;

    (*pOut).width(headerColumnWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_DB_LAST_UPDATE);

    int64_t i64LastDBUTime = dbHeader.GetLastDBUpdateTime();
    if (i64LastDBUTime == 0)
    {
        (*pOut) << TSS_GetString(cTW, tw::STR_NEVER) << endl << endl;
    }
    else
    {
        (*pOut) << cTWLocale::FormatTime(i64LastDBUTime, tstrDummy).c_str() << endl << endl;
    }

    OutputSectionDelimiter(tw::STR_DB_SUMMARY, pOut);

    (*pOut).width(headerColumnWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_HOST_NAME) << dbHeader.GetSystemName().c_str() << endl;
    (*pOut).width(headerColumnWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_HOST_IP) << dbHeader.GetIPAddress() << endl;


    (*pOut).width(headerColumnWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_HOST_ID);

    if (!dbHeader.GetHostID().empty())
        (*pOut) << dbHeader.GetHostID() << endl;
    else
        (*pOut) << TSS_GetString(cTW, tw::STR_NONE) << endl;


    (*pOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_POLICY_FILE_USED)
            << util_Encode(dbHeader.GetPolicyFilename()) << endl;

    (*pOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_CONFIG_FILE_USED)
            << util_Encode(dbHeader.GetConfigFilename()) << endl;

    (*pOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_DB_FILE_USED)
            << util_Encode(dbHeader.GetDBFilename()) << endl;

    (*pOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_CMD_LINE_USED)
            << util_Encode(dbHeader.GetCommandLineParams()) << endl
            << endl;
}

#if 0 
// The rules summary is not working due to changes in the databasse infrastructure.  Until 
// we hear different from the product manager, it is out of here.
static void OutputRulesSummary(const cFCODatabaseFileIter& genreIter, const iFCONameTranslator* pNT, TOSTREAM* pOut )
{
    const cHierDatabase& db = genreIter.GetDb();  

    // these should add up to 79
    const int nameWidth = 50;
    const int numObjectsWidth = 10; 

    (*pOut) << g_sz79Dashes << endl;
    (*pOut) << TSS_GetString( cTW, tw::STR_RULE_SUMMARY ) << endl;
    (*pOut) << g_sz79Dashes << endl << endl;

    (*pOut).width(nameWidth);
    (*pOut) << TSS_GetString( cTW, tw::STR_RULE_NAME );
    (*pOut).width(numObjectsWidth);
    (*pOut) << TSS_GetString( cTW, tw::STR_OBJECTS ) << endl;

    (*pOut) << setw( nameWidth ) << _T("---------");
    (*pOut) << setw( numObjectsWidth ) << _T("-------");
    (*pOut) << endl;

    // output spec stats
    cFCOSpecListCanonicalIter specIter( genreIter.GetSpecList() );

    // iterate over all database elements 
    int nTotalObjects = 0;
    for( specIter.SeekBegin(); !specIter.Done(); specIter.Next() )
    {
        (*pOut) << setw( nameWidth ) << specIter.Spec()->GetName();

        // if the name is too long, put it on its own line
        if( specIter.Spec()->GetName().length() >= nameWidth )
        {
            (*pOut) << endl;

            // output space holder
            (*pOut).width(nameWidth);
            (*pOut) << _T("");
        }

        (*pOut).width(numObjectsWidth);
        (*pOut) << _T("TODO");
        //(*pOut) << specIter.GetFCOSet()->Size(); TODO: Do this right
        (*pOut) << endl;
    }
    
    (*pOut) << endl;

    // NOTE 4 Jan 99 mdb -- this used to use dbHeader.GetTotalObjectsScanned() but it was not always consistent with the 
    //      total number of objects in the database. TODO -- if we are really not going to use this number, then we should 
    //      not store it in the database at all.
    //
    (*pOut) << TSS_GetString( cTW, tw::STR_TOTAL_NUM_FILES );
    //(*pOut) << _T(" ") << nTotalObjects << endl;
    (*pOut) << _T("TODO") << endl;
    (*pOut) << endl;
}
#endif


void OutputSectionDelimiter(int nString, TOSTREAM* pOut)
{
    (*pOut) << g_sz79Equals << endl;
    (*pOut) << TSS_GetString(cTW, nString) << endl;
    (*pOut) << g_sz79Equals << endl << endl;
}

void OutputGenreDelimiter(cGenre::Genre g, TOSTREAM* pOut)
{
    (*pOut) << g_sz79Dashes << endl;
    (*pOut) << _T("# ") << TSS_GetString(cTW, tw::STR_SECTION);
    (*pOut) << _T(": ") << cGenreSwitcher::GetInstance()->GenreToString(g, true) << endl;
    (*pOut) << g_sz79Dashes << endl << endl;
}

void OutputObjectSummary(cFCODatabaseFile& rd, TOSTREAM* pOut, int margin)
{
    OutputSectionDelimiter(tw::STR_OBJECT_SUMMARY, pOut);

    cFCODatabaseFileIter genreIter(rd);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        cGenreSwitcher::GetInstance()->SelectGenre((cGenre::Genre)genreIter.GetGenre());

        iFCONameTranslator* pNT = iTWFactory::GetInstance()->GetNameTranslator();

        OutputGenreDelimiter((cGenre::Genre)genreIter.GetGenre(), pOut);

        // Currently we don't have a rule summary.  See function definition below
        //OutputRulesSummary( genreIter, pNT, pOut);

        OutputObjectSummary(genreIter, pNT, pOut, margin);
    }
}

void OutputObjectSummary(cFCODatabaseFileIter& genreIter, const iFCONameTranslator* pNT, TOSTREAM* pOut, int margin)
{
    const cDbDataSourceIter dbIter(&genreIter.GetDb());
    ASSERT(dbIter.AtRoot());

    (*pOut) << setw(margin) << _T("");
    TSTRING strBuf;
    (*pOut) << genreIter.GetGenreHeader().GetPropDisplayer()->GetDetailsHeader(strBuf, margin).c_str() << endl;

    OutputIterPeers(dbIter, genreIter.GetGenreHeader().GetPropDisplayer(), pNT, pOut, false);
    (*pOut) << endl;
}


void OutputObjectDetail(cFCODatabaseFile& rd, TOSTREAM* pOut)
{
    OutputSectionDelimiter(tw::STR_OBJECT_DETAIL, pOut);

    cFCODatabaseFileIter genreIter(rd);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        cGenreSwitcher::GetInstance()->SelectGenre((cGenre::Genre)genreIter.GetGenre());
        iFCONameTranslator* pNT = iTWFactory::GetInstance()->GetNameTranslator();

        OutputGenreDelimiter((cGenre::Genre)genreIter.GetGenre(), pOut);

        OutputObjectDetail(genreIter, pNT, pOut);
    }
}


void OutputObjectDetail(cFCODatabaseFileIter& genreIter, const iFCONameTranslator* pNT, TOSTREAM* pOut)
{
    const cDbDataSourceIter dbIterDetails(&genreIter.GetDb());

    OutputIterPeers(dbIterDetails, genreIter.GetGenreHeader().GetPropDisplayer(), pNT, pOut, true);
    (*pOut) << endl;
}


/*
static void OutputDetailHeader( const cFCODbSpecIter &dbi, int nObjects, TOSTREAM* pOut )
{
    (*pOut) << g_sz79Dashes << _T( "\n" );
    (*pOut) << TSS_GetString( cTW, tw::STR_FILE_DETAIL ) << endl;
    
    TOSTRINGSTREAM ostr2;
    ostr2 << TSS_GetString( cTW, tw::STR_RULE_NAME ) << _T(": ") << dbi.GetSpec()->GetName().c_str();
    ostr2 << _T( " (" ) << dbi.GetSpec()->GetStartPoint().AsString().c_str() << _T( ")" ) << endl;
    (*pOut) << ostr2.str().c_str();

    (*pOut) << TSS_GetString( cTW, tw::STR_TOTAL_NUM_FILES ) << _T(" ") << nObjects << endl;

    (*pOut) << g_sz79Dashes << endl << endl;
}
*/

/*
static void OutputSpecHeader( const cFCODbSpecIter &dbi, TOSTREAM* pOut )
{
    (*pOut) << g_sz79Dashes << _T( "\n" );
    
    TOSTRINGSTREAM ostr2;
    ostr2 << TSS_GetString( cTW, tw::STR_RULE_NAME ) << _T(": ") << dbi.GetSpec()->GetName().c_str();
    ostr2 << _T( " (" ) << dbi.GetSpec()->GetStartPoint().AsString().c_str() << _T( ")" ) << endl;
    (*pOut) << ostr2.str().c_str();

    (*pOut) << g_sz79Dashes << _T( "\n" );
}
*/


void DisplayFCOProps(const iFCO* const pFCO, const iFCOPropDisplayer* pPD, TOSTREAM* pOut)
{
    ASSERT(pOut != 0);
    ASSERT(pFCO != 0);

    const int attrNameWidth  = 24;
    const int attrValueWidth = 28;

    // output header
    // TODO: make these constants into enums
    (*pOut).width(attrNameWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_ATTRIBUTES);
    (*pOut).width(attrValueWidth);
    (*pOut) << TSS_GetString(cTW, tw::STR_ATTR_VALUE);
    (*pOut) << endl;

    (*pOut).width(attrNameWidth);
    (*pOut) << _T("-------------");
    (*pOut).width(attrValueWidth);
    (*pOut) << _T("-----------");
    (*pOut) << endl;

    int iNumProps = pFCO->GetPropSet()->GetNumProps();
    for (int j = 0; j < iNumProps; j++)
    {
        // output if prop is in FCO, and prop is not cFCOUndefinedProp
        if (pFCO->GetPropSet()->GetValidVector().ContainsItem(j) &&
            pFCO->GetPropSet()->GetPropAt(j)->GetType() != cFCOUndefinedProp::GetInstance()->GetType())
        {
            TSTRING strProp;

            // output property name
            (*pOut).width(attrNameWidth);
            (*pOut) << pFCO->GetPropSet()->GetPropName(j);
            (*pOut).width(attrValueWidth);
            (*pOut) << pPD->PropAsString(pFCO, j, attrNameWidth, 0);
            (*pOut) << _T("\n");
        }
    }
}
