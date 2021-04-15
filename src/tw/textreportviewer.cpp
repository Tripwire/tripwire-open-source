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
// textreportviewer.cpp -- implementation for textreportviewer class
//

// TODO:BAM put strings into resources
// TODO:BAM add debug strings

//=========================================================================
// DIRECTIVES
//=========================================================================


//=========================================================================
// INCLUDES
//=========================================================================

#include "stdtw.h"
#include "textreportviewer.h"
#include "core/errorbucketimpl.h"
#include "core/fsservices.h"
#include "core/timeconvert.h"
#include "core/msystem.h"
#include "core/twlocale.h"
#include "core/errortable.h"
#include "core/debug.h"
#include "core/errorutil.h"
#include "core/displayutil.h"
#include "core/fixedfilebuf.h"
#include "fco/fcospec.h"
#include "fco/iterproxy.h"
#include "fco/fconame.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"
#include "fco/fconametranslator.h"
#include "fco/fcoundefprop.h"
#include "fco/genreswitcher.h"
#include "fco/twfactory.h"
#include "fco/fcopropdisplayer.h"
#include "fco/fcospecattr.h"
#include "fco/fcoundefprop.h"
#include "core/stringutil.h"
#include "util/fileutil.h"
#include "headerinfo.h"
#include "fcoreport.h"
#include "twstrings.h"
#include "core/displayencoder.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <fstream>
#include <iomanip>
#include <time.h>
#include <fcntl.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

using namespace std;

//=========================================================================
// GLOBALS
//=========================================================================

static const TCHAR* g_sz79Dashes =
    _T("-------------------------------------------------------------------------------");
static const TCHAR* g_sz40Dashes = _T("----------------------------------------");
static const TCHAR* g_sz79Equals =
    _T("===============================================================================");

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

#if !ARCHAIC_STL
static void OpenOutputFile(fixed_basic_ofstream<TCHAR>& out, const TSTRING& strFile); // throw( eTextReportViewer )
#else
static void OpenOutputFile(ofstream& out, const TSTRING& strFile); // throw( eTextReportViewer )
#endif

static void OpenInputFile(std::ifstream& out, const TSTRING& strFile);                // throw( eTextReportViewer )
static bool PrintableProp(const iFCO* pfcoOld, const iFCO* pfcoNew, int j);

static bool SpecEmpty(const cFCOReportSpecIter& ri);

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

cTextReportViewer::cTextReportViewer(const cFCOReportHeader& h, const cFCOReport& r)
{
    Init(h, const_cast<cFCOReport&>(r)); // TODO:BAM -- hack to allow creation
                                         // for updating (non-const report) and non-updating
                                         // (const report) in one constructor format
}

cTextReportViewer::cTextReportViewer(const cFCOReportHeader& h, cFCOReport& r)
{
    Init(h, r);
}

int cTextReportViewer::Init(const cFCOReportHeader& h, cFCOReport& r)
{
    mpHeader             = &h;
    mpReport             = &r;
    mpOut                = NULL;
    mpIn                 = NULL;
    mpCurPD              = NULL;
    mpCurNT              = NULL;
    mfUpdate             = false;
    mMaxSeverityViolated = 0;
    mNumberViolations    = 0;
    mErrorNum            = 1;
    mReportingLevel      = FULL_REPORT;
    mfGotNumbers         = false;
    mCurrentChar[0]      = '\0';
    mCurrentCharSize     = 0;
    mFilterFCOs          = false;
    return 0;
}

cTextReportViewer::~cTextReportViewer()
{
    if (mfUpdate)
    {
        for (GenreList::iterator i = mFCOsRemoveFromReport.begin(); i != mFCOsRemoveFromReport.end(); ++i)
        {
            ASSERT(i->second != 0);
            i->second->clear();
            delete i->second;
        }
    }
}


int cTextReportViewer::GetMaxSeverityViolated()
{
    if (!mfGotNumbers)
        GetReportNumbers();

    return mMaxSeverityViolated;
}

int cTextReportViewer::GetNumberViolations()
{
    if (!mfGotNumbers)
        GetReportNumbers();

    return mNumberViolations;
}

void cTextReportViewer::InitOStream()
{
    // align left
    (*mpOut).flags(((*mpOut).flags() & ~std::ios::adjustfield) | std::ios::left);
}

void cTextReportViewer::DisplayReportAndHaveUserUpdateIt(const TSTRING& edName,
                                                         ReportingLevel level) //throw (eFSServices)
{
    if (!CanUpdate())
    {
        // We should not be calling this for classes derived from cTextReport that can't update
        THROW_INTERNAL("textreportviewer.cpp");
    }

    ASSERT(!edName.empty());

    //
    // get temp filename in which to store user-readable report
    //
    TSTRING      strTempFile;
    iFSServices* pFSServices = iFSServices::GetInstance();
    ASSERT(pFSServices != 0);

    pFSServices->GetTempDirName(strTempFile);
    strTempFile += _T("twtempXXXXXX");
    pFSServices->MakeTempFilename(strTempFile);

    // this try/catch ensures the temp file was removed...
    try
    {
        // we are updating...
        mfUpdate = true;

        //
        // write report to the temp file.  put checks beside each FCO entry.
        //
        PrintTextReport(strTempFile, level);

        //
        // find an appropriate editor and launch it on the temp file.  the user
        // can uncheck any item she doesn't want put back into the database.
        //
        LaunchEditorOnFile(strTempFile, edName);

        //
        // read report back in from the temp file and remove any unchecked entries from
        // the report
        //
        ReadTextReport(strTempFile);

        //
        // remove unchecked elements from remove
        //
        RemoveFCOsFromReport();
    }
    catch (...)
    {
        // delete the temp file, then rethrow
        pFSServices->FileDelete(strTempFile);
        throw;
    }
    //
    // delete temp file
    //
    pFSServices->FileDelete(strTempFile);
}

///////////////////////////////////////////////////////////////////////////////
// PrintTextReport
//
// prints a text version of the passed in report to the passed in stream; the
// version that takes a file name is just a convenience function that calls the
// ostream version. '-' can be passed to signify stdout.
///////////////////////////////////////////////////////////////////////////////
void cTextReportViewer::PrintTextReport(const TSTRING& strFilename, ReportingLevel level) //throw (eTextReportViewer);
{
    mReportingLevel = level;

    if (strFilename.compare(_T("-")) == 0)
    {
        mpOut = &TCOUT;
        OutputTextReport();
    }
    else
    {
#if !ARCHAIC_STL      
        fixed_basic_ofstream<TCHAR> out;
#else
	ofstream out;
#endif
        OpenOutputFile(out, strFilename);
        mpOut = &out;
        OutputTextReport();
        out.close();
    }
}

///////////////////////////////////////////////////////////////////////////////
// PrintTextReport
///////////////////////////////////////////////////////////////////////////////
void cTextReportViewer::PrintTextReport(TOSTREAM& ostr, ReportingLevel level)
{
    mReportingLevel = level;
    mpOut           = &ostr;
    OutputTextReport();
}


void cTextReportViewer::GetReportNumbers()
{
    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        // We need to at least get the max severity in this function
        cFCOReportSpecIter ri(genreIter);
        for (ri.SeekBegin(); !ri.Done(); ri.Next())
        {
            if (!IgnoreThisSpec(ri.GetAttr()))
            {
                int nAdded   = ri.GetAddedSet()->Size();
                int nRemoved = ri.GetRemovedSet()->Size();
                int nChanged = ri.GetNumChanged();

                if (nAdded || nRemoved || nChanged)
                {
                    // update the max severity
                    if (mMaxSeverityViolated < ri.GetAttr()->GetSeverity())
                        mMaxSeverityViolated = ri.GetAttr()->GetSeverity();

                    mNumberViolations += (nAdded + nRemoved + nChanged);
                }
            }
        }
    }

    mfGotNumbers = true;
}

void cTextReportViewer::OutputTextReport() //throw (eTextReportViewer);
{
    ASSERT(mpOut != 0);

    InitOStream();

    //
    // count up total violations, max severity, etc.
    //
    GetReportNumbers();

    //
    // TODO: move this
    //
    if (SINGLE_LINE == mReportingLevel)
    {
        (*mpOut) << SingleLineReport() << endl;
    }
    else if (PARSEABLE == mReportingLevel)
    {
        OutputParseableReport();
    }
    else if (SUMMARY_ONLY <= mReportingLevel)
    {
        //
        // output header, just like it sez....
        //
        OutputReportHeader();

        //
        // output summary info
        //
        OutputRulesSummary();

        //
        // print names of rule-breakers
        //
        // don't print out object summary if in CONCISE_REPORT because
        // the detailed report will look a lot like the summary.
        //
        if (CONCISE_REPORT != mReportingLevel)
        {
            OutputObjectSummary();
        }

        if (CONCISE_REPORT <= mReportingLevel)
        {
            //
            // print details of rule-breakers
            //
            OutputObjectDetails();
        }

        //
        // output any errors
        //
        PrintErrors();

        //
        // output footer
        //
        OutputFooter();
    }
}

void cTextReportViewer::SetUpForNewGenre(const cFCOReportGenreIter& genreIter)
{
    //
    // set up prop displayer and name translator
    //
    mpCurPD = genreIter.GetGenreHeader().GetPropDisplayer();
    mpCurNT = cGenreSwitcher::GetInstance()->GetFactoryForGenre(genreIter.GetGenre())->GetNameTranslator();
}

void cTextReportViewer::PrintErrors()
{
    //
    // output header
    //
    OutputSectionDelimiter(tw::STR_ERROR_REPORT);

    //
    // output any errors from the report's general error queue
    //
    bool            fAtLeastOneGeneralError = false;
    cErrorQueueIter eqIter(*(mpReport->GetErrorQueue()));
    for (eqIter.SeekBegin(); !eqIter.Done(); eqIter.Next())
    {
        ReportError(eqIter);
        fAtLeastOneGeneralError = true;
    }

    // separate general errors from genre errors
    if (fAtLeastOneGeneralError)
        (*mpOut) << endl;

    //
    // cycle through genres
    //
    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        //
        // set up prop displayer and name translator
        //
        SetUpForNewGenre(genreIter);

        //
        // report errors for genre
        //
        bool               fFirstErrorInGenre = true;
        cFCOReportSpecIter rsi(genreIter);
        for (rsi.SeekBegin(); !rsi.Done(); rsi.Next())
        {
            cErrorQueueIter eqIter2(*(rsi.GetErrorQueue()));
            for (eqIter2.SeekBegin(); !eqIter2.Done(); eqIter2.Next())
            {
                if (fFirstErrorInGenre)
                {
                    OutputGenreDelimiter(genreIter.GetGenre());
                    fFirstErrorInGenre = false;
                }

                ReportError(eqIter2);
            }
        }

        // separate genre errors
        if (!fFirstErrorInGenre)
            (*mpOut) << endl;
    }

    //
    // if there were no errors, tell user
    //
    if (1 == mErrorNum)
    {
        (*mpOut) << TSS_GetString(cTW, tw::STR_REPORT_NO_ERRORS) << endl << endl;
    }
}

void cTextReportViewer::OutputSectionDelimiter(int nString)
{
    (*mpOut) << g_sz79Equals << endl;
    (*mpOut) << TSS_GetString(cTW, nString) << endl;
    (*mpOut) << g_sz79Equals << endl << endl;
}

void cTextReportViewer::OutputGenreDelimiter(cGenre::Genre g, bool fDenoteBallotSection)
{
    (*mpOut) << g_sz79Dashes << endl;

    // a # denotes the ballot section
    if (fDenoteBallotSection)
        (*mpOut) << _T("# ");
    else
        (*mpOut) << _T("  ");

    (*mpOut) << TSS_GetString(cTW, tw::STR_SECTION);
    (*mpOut) << _T(": ") << cGenreSwitcher::GetInstance()->GenreToString(g, true) << endl;
    (*mpOut) << g_sz79Dashes << endl << endl;
}

void cTextReportViewer::OutputNumDetails(int nString, int nObjects)
{
    (*mpOut) << _T("  ") << g_sz40Dashes << endl;
    (*mpOut) << _T("  ") << TSS_GetString(cTW, nString) << nObjects << endl;
    (*mpOut) << _T("  ") << g_sz40Dashes << endl << endl;
}

void cTextReportViewer::OutputObjectSummary()
{
    //
    // check if we should output summary
    //
    if (!WantOutputObjectSummary())
        return;

    //
    // output summary header
    //
    OutputSectionDelimiter(tw::STR_OBJECT_SUMMARY);

    //
    // cycle through genres
    //
    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        //
        // set up prop displayer and name translator
        //
        SetUpForNewGenre(genreIter);

        //
        // output genre delimiter
        //
        OutputGenreDelimiter(genreIter.GetGenre(), true);

        //
        // output object summary
        //
        OutputObjectSummary(genreIter);
    }
}

void cTextReportViewer::OutputObjectSummary(const cFCOReportGenreIter& genreIter)
{
    // iterate over all report elements (keyed by a spec), and save
    // references to all added, removed, and changed FCOs.  We do this
    // because we want to group the FCOs by added, removed, and changed,
    // not by spec.


    //
    // if we're updating, we need to save a list of names
    //
    FCOList* pFCONameList = NULL;
    if (mfUpdate)
        pFCONameList = new FCOList;

    cFCOReportSpecIter ri(genreIter);
    bool               violationReported = false;
    for (ri.SeekBegin(); !ri.Done(); ri.Next())
    {
        // skip this spec if it's not one we care about for this report
        if (IgnoreThisSpec(ri.GetAttr()))
            continue;

        //
        // output a spec header if it was violated
        //
        if (!SpecEmpty(ri))
        {
            violationReported = true;

            OutputSpecHeader(ri);
            (*mpOut) << endl;

            if (mfUpdate)
            {
                (*mpOut) << TSS_GetString(cTW, tw::STR_ADD_X) << endl;
                (*mpOut) << endl;
            }

            //
            // output added set
            //
            OutputAddedSummary(ri, pFCONameList);

            //
            // output removed set
            //
            OutputRemovedSummary(ri, pFCONameList);

            //
            // output changed set
            //
            OutputChangedSummary(ri, pFCONameList);
        }
    }

    if (violationReported == false)
    {
        // There are no violations for this section
        (*mpOut) << TSS_GetString(cTW, tw::STR_NO_VIOLATIONS_IN_SECTION) << endl << endl;
    }

    //
    // insert FCO list into master list
    //
    if (mfUpdate)
        mFCOsRemoveFromReport.insert(cTextReportViewer::GenreList::value_type(genreIter.GetGenre(), pFCONameList));
}

void cTextReportViewer::OutputObjectDetails()
{
    //
    // check if we should output details
    //
    if (!WantOutputObjectDetails())
        return;

    //
    // output detail header
    //
    OutputSectionDelimiter(tw::STR_OBJECT_DETAIL);

    //
    // cycle through genres
    //
    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        //
        // set up prop displayer and name translator
        //
        SetUpForNewGenre(genreIter);

        //
        // output genre delimiter
        //
        OutputGenreDelimiter(genreIter.GetGenre());

        //
        // output details
        //
        OutputObjectDetails(genreIter);
    }
}

void cTextReportViewer::OutputObjectDetails(const cFCOReportGenreIter& genreIter)
{
    //
    // Now enumerate all changes
    //
    cFCOReportSpecIter ri(genreIter);
    bool               violationReported = false;

    for (ri.SeekBegin(); !ri.Done(); ri.Next())
    {
        // skip this spec if it's not one we care about for this report
        if (IgnoreThisSpec(ri.GetAttr()))
            continue;

        //
        // output spec section header if there are changes
        //
        if (!SpecEmpty(ri))
        {
            violationReported = true;

            OutputSpecHeader(ri);

            //
            // output added set
            //
            OutputAddedDetails(ri);

            //
            // output removed set
            //
            OutputRemovedDetails(ri);

            //
            // output changed set
            //
            OutputChangedDetails(ri);
        }
    } // enum change details

    if (violationReported == false)
    {
        // There are no violations for this section
        (*mpOut) << TSS_GetString(cTW, tw::STR_NO_VIOLATIONS_IN_SECTION) << endl << endl;
    }
}

void cTextReportViewer::OutputFooter()
{
    (*mpOut) << g_sz79Dashes << endl;
    (*mpOut) << TSS_GetString(cTW, tw::STR_END_OF_REPORT) << endl << endl;
    (*mpOut) << TSS_GetString(cTW, tw::STR_COPYRIGHT) << endl;
}


bool cTextReportViewer::LaunchEditorOnFile(const TSTRING& strFilename,
                                           const TSTRING& edName) //throw (eTextReportViewer)
{
    bool    fRanViewer = false;
    TSTRING editor     = edName;

    // make sure we can read from this file
    cFileUtil::TestFileReadable(strFilename);

#if USES_MSYSTEM
    // editor is going to need terminal type, so tell msystem to include
    // it in environment when it makes its system call.
    le_set("TERM");
    le_set("DISPLAY"); // DISPLAY and HOME needed so we can launch X apps. X apps apparently check
    le_set("HOME");    // a .xpermissions file in the users home dir
    le_set("LANG");    // LANG allowed through in case any apps need
    int systemRet = msystem((char*)((editor + _T(' ') + strFilename).c_str()));
    le_unset("LANG");
    le_unset("HOME");
    le_unset("DISPLAY");
    le_unset("TERM");
#else
    int systemRet = system((char*)((editor + _T(' ') + strFilename).c_str()));
#endif

    if (0 == systemRet)
    {
        fRanViewer = true;
    }
    else
    {
        //ASSERT( false );
        throw eTextReportViewerEditorLaunch(edName);
    }

    return (fRanViewer);
}


void cTextReportViewer::ReportError(const cErrorQueueIter& eqIter)
{
    const int nWidth = 5;

    // output error number
    TOSTRINGSTREAM ostr;
    ostr << mErrorNum << _T(".");
    tss_mkstr(numStr, ostr);
    
    (*mpOut).width(nWidth);
    (*mpOut) << numStr;

    // output general error
    (*mpOut) << cErrorTable::GetInstance()->Get(eqIter.GetError().GetID()) << endl;

    // output specific error
    (*mpOut).width(nWidth);
    (*mpOut) << _T(""); // this is here so the next output is 10 chars over
    (*mpOut) << cDisplayUtil::FormatMultiLineString(
                    cDisplayEncoder::EncodeInlineAllowWS(eqIter.GetError().GetMsg()), nWidth, 0)
             << endl;


    mErrorNum++;
}

void cTextReportViewer::PrintBallotLine(const iFCO& FCO)
{
    ASSERT(mpOut != 0);

    //
    // output "[X] <name of FCO>"
    //

    if (mfUpdate) //only have Xs for updating
    {
        (*mpOut) << _T("[x] "); // TODO: should this be in resources?
    }

    TSTRING strEncoded = mpCurNT->ToStringDisplay(FCO.GetName(), true);
    (*mpOut) << strEncoded.c_str();

    (*mpOut) << endl;
}


// looks for "\n[" and leaves the file pointer at the next character
// could be a problem if the ballot box is the first line in the file, then "\n["
// wouldn't be encountered, only "[", so make sure that the ballots aren't
// on the first line
// returns false if finds "\n#" -- start of new genre
int cTextReportViewer::FindNextLineToken()
{
    // loop over chars until (1) EOF is hit, or (2) we find "\n[" or '\n#'
    for (GetChar(); !mpIn->eof(); GetChar())
    {
        if (mCurrentChar[0] == '\n')
        {
            // we'll only compare to single byte chars
            // so it's ok to use PeekChar() instead of our GetChar()
            if (PeekChar() == '[')
            {
                return TOKEN_BALLOT_BOX;
            }
            else if (PeekChar() == '#') // start of new genre
            {
                return TOKEN_GENRE;
            }
        }
    }

    return (TOKEN_EOF);
}

void cTextReportViewer::ReadTextReport(const TSTRING& strFilename) //throw (eTextReportViewer)
{
    std::ifstream in;
    OpenInputFile(in, strFilename);
    mpIn = &in;

    //
    // for each genre and ballot box in text report file
    //
    bool     fMoreTokens = true;
    FCOList* pCurList    = NULL;
    while (fMoreTokens)
    {
        switch (FindNextLineToken())
        {
        case TOKEN_GENRE:
            GetGenreInfo(&pCurList);
            break;
        case TOKEN_BALLOT_BOX:
            GetBallotInfo(pCurList);
            break;
        case TOKEN_EOF:
            fMoreTokens = false;
            break;
        default:
            ASSERT(false);
            break;
        }
    }

    //
    // close the file
    //
    in.close();
}


void cTextReportViewer::GetGenreInfo(FCOList** ppCurList)
{
    //
    // identify genre
    //
    TSTRING strGenre = GetGenre();
    ASSERT(!strGenre.empty());
    cGenre::Genre g = cGenreSwitcher::GetInstance()->StringToGenre(strGenre.c_str());
    if (cGenre::GENRE_INVALID == g)
    {
        throw eTextReportViewerReportCorrupt("Invalid Genre"); // TODO: ERR_UKNOWN_GENRE
    }
    cGenreSwitcher::GetInstance()->SelectGenre(g);

    //
    // get list of fcos in report for this genre
    //
    GenreList::iterator curIter = mFCOsRemoveFromReport.find(g);
    if (curIter == mFCOsRemoveFromReport.end())
        throw eTextReportViewerReportCorrupt("No files found in report"); // TODO: ERR_UKNOWN_GENRE
    *ppCurList = curIter->second;

    //
    // get the prop displayer
    //
    mpCurPD = cGenreSwitcher::GetInstance()->GetFactoryForGenre(g)->CreatePropDisplayer();
    mpCurNT = cGenreSwitcher::GetInstance()->GetFactoryForGenre(g)->GetNameTranslator();
}

void cTextReportViewer::GetBallotInfo(FCOList* pCurList)
{
    if (!pCurList)
        throw eTextReportViewerReportCorrupt("No ballot list found");

    // if the box is checked, then the user elected to leave the item
    // in the report, so we do nothing.  If the box isn't checked, then
    // we must remove the item from the report.
    if (IsChecked())
    {
        //
        // look at the ballot line and get the name of the FCO
        //
        cFCOName fcoName;
        GetFCONameFromBallotLine(fcoName);

        ///////////////////////////////////////
        // remove this FCO from list
        cTextReportViewer::FCOList::const_iterator iter;
        iter = pCurList->find(fcoName);
        if (iter == pCurList->end())
        {
            throw eTextReportViewerReportCorrupt("Unknown file name in report"); // TODO: ERR_UKNOWN_NAME
        }

        pCurList->erase(fcoName);
        ///////////////////////////////////////
    }
}

// reads input up to EOL or EOF
TSTRING cTextReportViewer::GetGenre()
{
    std::string strGenre;

    EatSection();

    while (true)
    {
        char chFirst = PeekChar();
        if (PeekIsEOF() || chFirst == '\r' || chFirst == '\n')
            break;

        // eat up char
        GetChar();
        AppendChar(strGenre);
    }

    return cStringUtil::StrToTstr(strGenre);
}


bool cTextReportViewer::PeekIsEOF()
{
#if !ARCHAIC_STL  
    return (mpIn->peek() == char_traits<char>::eof());
#else
    return (mpIn->peek() == EOF);    
#endif    
}

// if the next character in the stream is ('X' or 'x'), it eats the x, else it returns false
// if the next character after the x is ']', it eats that, else it returns false.
bool cTextReportViewer::IsChecked()
{
    bool fIsChecked = false;
    char chFirst;

    GetChar();            // eat up '['
    chFirst = PeekChar(); // look for 'X'

    if ((!PeekIsEOF()) && (chFirst == 'X' || chFirst == 'x')) // TODO: X in resources?
    {
        mpIn->get(chFirst); // eat up X

        char chSecond = PeekChar();
        if ((!PeekIsEOF()) && (chSecond == ']'))
        {
            fIsChecked = true;
            mpIn->get(chSecond); // eat up ']'
        }
    }

    return (fIsChecked);
}

// FCO names are '\"' delimited and begin after the first '\"' after the ballot box
// NOTE: will not eat up a return character
void cTextReportViewer::GetFCONameFromBallotLine(cFCOName& fcoName) //throw (eTextReportViewer)
{
    char        chIn;
    std::string strFCOName;

#define TW_IS_QUOTE(ch) (ch == '\"')
#define TW_IS_EOL(ch) (ch == '\n' || ch == '\r')
#define TW_IS_BACKSLASH(ch) (ch == '\\')

    //
    // go past any character that's not '\"' or EOL
    //  if we hit EOL, then there was no fconame present, so error
    //
    for (chIn = PeekChar(); !PeekIsEOF(); chIn = PeekChar()) // while not EOF
    {
        if (TW_IS_QUOTE(chIn))
        {
            mpIn->get(chIn);    // eat up all chars but EOL
            strFCOName += chIn; // add quote to string

            // we've found our quote, break and get name
            break;
        }
        else if (TW_IS_EOL(chIn))
        {
            // if EOL, there was no name!
            throw eTextReportViewerReportCorrupt("Ballot item without a name");
        }
        else
        {
            GetChar(); // eat up all chars but EOL
        }
    }

    //
    // get FCO name (chars up until the next '<EOL>')
    //
    for (chIn = PeekChar(); !PeekIsEOF(); chIn = PeekChar()) // while not EOF
    {
        if (TW_IS_EOL(chIn)) // if EOL, there was no ending quote!
        {
            // found end of fconame
            break;
        }
        else
        {
            // add char to fco name
            GetChar();
            AppendChar(strFCOName);
        }
    }

    // make sure we've got a name
    if (strFCOName.empty() || !mpCurNT->DisplayStringToFCOName(cStringUtil::StrToTstr(strFCOName), fcoName))
    {
        std::string msg = "Invalid object name: " + strFCOName;
        throw eTextReportViewerReportCorrupt(
            msg); // TODO -- it might be nice to be able to specify what line of the report got corrupted
    }
}

void cTextReportViewer::RemoveFCOsFromReport() //throw (eTextReportViewer)
{
    // TODO: remove fconames from fcosToRemove when we find them in report,
    //       so we don't compare that fconame again.  Watch out for messing
    //       with iters in the for, loops, though!  That's why I didn't do
    //       it yet... (and I figure most users would only un-X a few items,
    //       in which case it is currently faster anyways...)


    //
    // iterate over all specs in report, then in each of the added,
    // removed, and changed sets, lookup the FCOName and, if found,
    // delete it
    //
    int nFCOsRemoved = 0;

    cFCOReportGenreIter rgi(*mpReport);
    for (rgi.SeekBegin(); !rgi.Done(); rgi.Next())
    {
        cTextReportViewer::GenreList::iterator currentList = mFCOsRemoveFromReport.find(rgi.GetGenre());
        ASSERT(currentList != mFCOsRemoveFromReport.end());
        cTextReportViewer::FCOList* pFCOList = currentList->second;

        cFCOReportSpecIter rsi(rgi);
        for (rsi.SeekBegin(); !rsi.Done(); rsi.Next())
        {
            // search added
            const iFCOSet* pAddedSet = rsi.GetAddedSet();
            ASSERT(pAddedSet != 0);

            const iFCOIter*                      fcoIter = NULL;
            cTextReportViewer::FCOList::iterator iter;
            //
            // see if any FCOs to remove are in this set
            //
            for (iter = pFCOList->begin(); iter != pFCOList->end(); ++iter)
            {
                fcoIter = pAddedSet->Lookup(*iter);
                if (fcoIter)
                {
                    fcoIter->Remove();
                    nFCOsRemoved++;

                    fcoIter->DestroyIter();
                }
            }

            // search removed
            // get removed set
            const iFCOSet* pRemovedSet = rsi.GetRemovedSet();
            ASSERT(pRemovedSet != 0);

            //
            // see if any FCOs to remove are in this set
            //
            for (iter = pFCOList->begin(); iter != pFCOList->end(); ++iter)
            {
                fcoIter = pRemovedSet->Lookup(*iter);
                if (fcoIter)
                {
                    fcoIter->Remove();
                    nFCOsRemoved++;

                    fcoIter->DestroyIter();
                }
            }


            for (iter = pFCOList->begin(); iter != pFCOList->end(); ++iter)
            {
                // search changed
                // get changed set iterator
                cFCOReportChangeIter changedIter(rsi);

                // iterate over all changed fcos
                for (changedIter.SeekBegin(); !changedIter.Done(); changedIter.Next())
                {
                    cFCOName name = changedIter.GetOld()->GetName();

                    if (name.IsEqual(*iter))
                    {
                        changedIter.Remove();
                        nFCOsRemoved++;
                    }
                }
            }
        }
    }

    //
    // count FCOs to remove
    //
    int                                          nFCOsToRemove = 0;
    cTextReportViewer::GenreList::const_iterator iter          = mFCOsRemoveFromReport.begin();
    for (; iter != mFCOsRemoveFromReport.end(); ++iter)
        nFCOsToRemove += iter->second->size();

    if (nFCOsToRemove != nFCOsRemoved)
    {
        // TODO -- maybe have a different enumeration for this?
        throw eTextReportViewerReportCorrupt("Mismatch in objects to remove");
    }
}

void cTextReportViewer::OutputReportHeader()
{
    if (!WantOutputReportHeader())
        return;

    const int headerColumnWidth = 30;

    (*mpOut) << TSS_GetString(cTW, tw::STR_REPORT_TITLE) << endl << endl;


    (*mpOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_R_GENERATED_BY)
             << util_Encode(mpHeader->GetCreator()) << endl;

    // TODO: ( start / end / elapsed ) time
    TSTRING tstrDummy;
    int64_t i64CreateTime = mpHeader->GetCreationTime();
    (*mpOut).width(headerColumnWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_R_CREATED_ON) << cTWLocale::FormatTime(i64CreateTime, tstrDummy).c_str()
             << endl;

    (*mpOut).width(headerColumnWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_DB_LAST_UPDATE);

    int64_t i64LastDBUTime = mpHeader->GetLastDBUpdateTime();
    if (i64LastDBUTime == 0)
    {
        (*mpOut) << TSS_GetString(cTW, tw::STR_NEVER) << endl << endl;
    }
    else
    {
        (*mpOut) << cTWLocale::FormatTime(i64LastDBUTime, tstrDummy).c_str() << endl << endl;
    }

    OutputSectionDelimiter(tw::STR_R_SUMMARY);

    (*mpOut).width(headerColumnWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_HOST_NAME) << mpHeader->GetSystemName().c_str() << endl;
    (*mpOut).width(headerColumnWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_HOST_IP) << mpHeader->GetIPAddress() << endl;

    (*mpOut).width(headerColumnWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_HOST_ID);

    if (!mpHeader->GetHostID().empty())
        (*mpOut) << mpHeader->GetHostID() << endl;
    else
        (*mpOut) << TSS_GetString(cTW, tw::STR_NONE) << endl;

    (*mpOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_POLICY_FILE_USED)
             << util_Encode(mpHeader->GetPolicyFilename()) << endl;

    (*mpOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_CONFIG_FILE_USED)
             << util_Encode(mpHeader->GetConfigFilename()) << endl;

    (*mpOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_DB_FILE_USED)
             << util_Encode(mpHeader->GetDBFilename()) << endl;

    (*mpOut) << setw(headerColumnWidth) << TSS_GetString(cTW, tw::STR_CMD_LINE_USED)
             << util_Encode(mpHeader->GetCommandLineParams()) << endl
             << endl;
}

void cTextReportViewer::OutputRulesSummary()
{
    //
    // check if we should output summary
    //
    if (!WantOutputRulesSummary())
        return;

    //
    // output summary header
    //
    OutputSectionDelimiter(tw::STR_RULE_SUMMARY);

    //
    // cycle through genres
    //
    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        //
        // set up prop displayer and name translator
        //
        SetUpForNewGenre(genreIter);

        //
        // output genre delimiter
        //
        OutputGenreDelimiter(genreIter.GetGenre());

        //
        // output rules summary
        //
        OutputRulesSummary(genreIter);
    }
}

void cTextReportViewer::OutputRulesSummary(const cFCOReportGenreIter& genreIter)
{
    // output spec stats header
    // 2 + specNameWidth + severityWidth + 3*numObjectsWidth should be < 80
    const int specNameWidth   = 32;
    const int severityWidth   = 18;
    const int numObjectsWidth = 9;
    (*mpOut) << _T("  "); // indent
    (*mpOut).width(specNameWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_RULE_NAME);
    (*mpOut).width(severityWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_SEVERITY_LEVEL);
    (*mpOut).width(numObjectsWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_ADDED);
    (*mpOut).width(numObjectsWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_REMOVED);
    (*mpOut).width(numObjectsWidth);
    (*mpOut) << TSS_GetString(cTW, tw::STR_CHANGED);
    (*mpOut) << endl;

    (*mpOut) << _T("  ");
    (*mpOut) << setw(specNameWidth) << _T("---------");
    (*mpOut) << setw(severityWidth) << _T("--------------");
    (*mpOut) << setw(numObjectsWidth) << _T("-----");
    (*mpOut) << setw(numObjectsWidth) << _T("-------");
    (*mpOut) << setw(numObjectsWidth) << _T("--------");
    (*mpOut) << endl;

    // merge rules of duplicate names and severities
    ObjectSummaryList summaryList;
    CollateRulesSummary(genreIter, summaryList);

    // Output the information in the list
    int                         nTotalObjectsChanged = 0;
    ObjectSummaryList::iterator si;
    for (si = summaryList.begin(); si != summaryList.end(); ++si)
    {
        // highlight violated rules
        if (si->mAddedObjects + si->mRemovedObjects + si->mChangedObjects > 0)
            (*mpOut) << _T("* ");
        else
            (*mpOut) << _T("  "); // indent

        TSTRING strSpecName = util_Encode(si->mSpecName);
        (*mpOut) << setw(specNameWidth) << strSpecName;

        // if the name is too long, put it on its own line
        if (strSpecName.length() >= (unsigned int)specNameWidth)
        {
            (*mpOut) << endl;
            (*mpOut) << _T("  "); // indent
            (*mpOut).width(specNameWidth);
            (*mpOut) << _T(""); // output space holder
        }

        (*mpOut).width(severityWidth);
        (*mpOut) << si->mSeverity;
        (*mpOut).width(numObjectsWidth);
        (*mpOut) << si->mAddedObjects;
        (*mpOut).width(numObjectsWidth);
        (*mpOut) << si->mRemovedObjects;
        (*mpOut).width(numObjectsWidth);
        (*mpOut) << si->mChangedObjects;
        (*mpOut) << endl;

        // output start point on next line
        // TODO: I am not sure if I like this in the new collated reports, dmb Sept 16 1999
        if (!si->mStartPoint.empty())
            (*mpOut) << _T("  (") << si->mStartPoint << _T(")") << endl;

        nTotalObjectsChanged += si->mAddedObjects + si->mRemovedObjects + si->mChangedObjects;
    }

    (*mpOut) << endl;
    (*mpOut) << TSS_GetString(cTW, tw::STR_OBJECTS_SCANNED) << _T(" ") << genreIter.GetGenreHeader().GetObjectsScanned()
             << endl;
    (*mpOut) << TSS_GetString(cTW, tw::STR_TOTAL_VIOLATIONS) << _T(" ") << nTotalObjectsChanged << endl;
    (*mpOut) << endl;
}


void cTextReportViewer::CollateRulesSummary(const cFCOReportGenreIter& genreIter, ObjectSummaryList& summaryList)
{
    summaryList.clear();

    cFCOReportSpecIter ri(genreIter);
    for (ri.SeekBegin(); !ri.Done(); ri.Next())
    {
        // skip this spec if it's not one we care about for this report
        if (IgnoreThisSpec(ri.GetAttr()))
            continue;

        RuleSummaryLine newLine;

        newLine.mSpecName       = util_Encode(ri.GetSpec()->GetName());
        newLine.mSeverity       = ri.GetAttr()->GetSeverity();
        newLine.mAddedObjects   = ri.GetAddedSet()->Size();
        newLine.mRemovedObjects = ri.GetRemovedSet()->Size();
        newLine.mChangedObjects = ri.GetNumChanged();
        newLine.mStartPoint     = mpCurNT->ToStringDisplay(ri.GetSpec()->GetStartPoint());

        // If there is already a summary line for this rule with equivalent severity, merge this one into it
        ObjectSummaryList::iterator si;
        for (si = summaryList.begin();; ++si)
        {
            if (si == summaryList.end())
            {
                summaryList.push_back(newLine);
                break;
            }

            if (si->mSpecName == newLine.mSpecName && si->mSeverity == newLine.mSeverity)
            {
                si->mAddedObjects += newLine.mAddedObjects;
                si->mRemovedObjects += newLine.mRemovedObjects;
                si->mChangedObjects += newLine.mChangedObjects;

                // if one of the start points is a subset of the other, then we take the shorter one.
                // otherwise we set the startpoint to empty.
		// TODO this logic is not aware of strings as paths and may do the wrong thing in some cases.
		// 
                if (newLine.mStartPoint.length() <= si->mStartPoint.length())
		{
#if !ARCHAIC_STL
                    if (newLine.mStartPoint.compare(0, newLine.mStartPoint.length(), si->mStartPoint) == 0)
#else
		    if (_tcsncmp(newLine.mStartPoint.c_str(), si->mStartPoint.c_str(), newLine.mStartPoint.length()) == 0)
#endif		      
		    {
		        si->mStartPoint = newLine.mStartPoint;
                    }
		    else
		    {
                        si->mStartPoint.erase();
		    }
		}
#if !ARCHAIC_STL		
                else if (si->mStartPoint.compare(0, si->mStartPoint.length(), newLine.mStartPoint) == 0)
#else
                else if (_tcsncmp(si->mStartPoint.c_str(), newLine.mStartPoint.c_str(), si->mStartPoint.length()) == 0)
#endif		  
		{
                    ;
		}
                else
		{
                    si->mStartPoint.erase();
                }
		
                break;
            }
        }
    }
}


void cTextReportViewer::OutputSpecHeader(const cFCOReportSpecIter& ri)
{
    if (!WantOutputSpecHeader())
        return;

    (*mpOut) << g_sz79Dashes << endl;

    (*mpOut) << TSS_GetString(cTW, tw::STR_RULE_NAME) << _T(": ") << util_Encode(ri.GetSpec()->GetName());
    (*mpOut) << _T( " (" ) << mpCurNT->ToStringDisplay(ri.GetSpec()->GetStartPoint()) << _T( ")" ) << endl;
    (*mpOut) << TSS_GetString(cTW, tw::STR_SEVERITY_LEVEL) << _T(": ") << ri.GetAttr()->GetSeverity() << endl;

    (*mpOut) << g_sz79Dashes << endl;
}

void cTextReportViewer::DisplayChangedProps(const iFCO* const     pfcoOld,
                                            const iFCO* const     pfcoNew,
                                            const cFCOPropVector* pv)
{
    ASSERT(mpOut != 0);
    ASSERT(pfcoOld || pfcoNew); // have to have at least one to display

    const iFCO* pfcoValid = pfcoOld ? pfcoOld : pfcoNew; // we need to use one of the pfco's for some Get functions

    // marginWidth + attrNameWidth + ( 2 * attrValueWidth ) should be < 80
    const int attrNameWidth  = 21;
    const int attrValueWidth = 28;
    const int marginWidth    = 2; // for "  " or "* "
    ASSERT((attrNameWidth + (2 * attrValueWidth) + marginWidth) < 80);

    // output header
    (*mpOut) << setw(marginWidth) << _T("");
    (*mpOut) << setw(attrNameWidth) << TSS_GetString(cTW, tw::STR_ATTRIBUTES);
    (*mpOut) << setw(attrValueWidth) << TSS_GetString(cTW, tw::STR_EXPECTED);
    (*mpOut) << setw(attrValueWidth) << TSS_GetString(cTW, tw::STR_OBSERVED);
    (*mpOut) << endl;

    (*mpOut) << setw(marginWidth) << _T("");
    (*mpOut) << setw(attrNameWidth) << _T("-------------");
    (*mpOut) << setw(attrValueWidth) << _T("-----------");
    (*mpOut) << setw(attrValueWidth) << _T("-----------");
    (*mpOut) << endl;

    int iNumProps = pfcoValid->GetPropSet()->GetNumProps(); // TODO: what if new + old have different props
    for (int j = 0; j < iNumProps; j++)
    {
        // if not a full report, only print properties that have changed
        if (FULL_REPORT != mReportingLevel)
        {
            if (!pv || !pv->ContainsItem(j))
                continue;
        }

        // ignore certain combinations of undef and non-existent props
        if (!PrintableProp(pfcoOld, pfcoNew, j))
            continue;

        // highlight changes and set width for prop name
        if (!mpCurPD->IsMultiLineProp(j))
        {
            if (!pv || pv->ContainsItem(j))
                (*mpOut) << setw(marginWidth) << _T("*");
            else
                (*mpOut) << setw(marginWidth) << _T("");
        }
        else // for multiline props we will highlight the observed prop if it has changed
        {
            (*mpOut) << setw(marginWidth) << _T("");
        }


        //
        // output prop name
        //
        {
            TSTRING strNewName;
            strNewName = pfcoValid->GetPropSet()->GetPropName(j);

            if (mpCurPD->IsMultiLineProp(j))
            {
                strNewName += _T(" ");
                strNewName += TSS_GetString(cTW, tw::STR_EXPECTED);
            }

            //strNewName += _T(':') ;

            (*mpOut) << setw(attrNameWidth) << strNewName;
        }

        //
        // output old value
        //
        TSTRING strOldValue;
        (*mpOut).width(attrValueWidth);
        if (pfcoOld && pfcoOld->GetPropSet()->GetValidVector().ContainsItem(j))
            strOldValue = mpCurPD->PropAsString(pfcoOld, j, (marginWidth + attrNameWidth), 0).c_str();
        else
            strOldValue = _T("---");
        (*mpOut) << strOldValue;


        //
        // output name again, if necessary (if multiline prop)
        //
        if (mpCurPD->IsMultiLineProp(j))
        {
            TSTRING strNewName;
            strNewName = pfcoValid->GetPropSet()->GetPropName(j);
            strNewName += _T(" ");
            strNewName += TSS_GetString(cTW, tw::STR_OBSERVED);
            //strNewName += _T(':') ;

            (*mpOut) << endl;

            if (!pv || pv->ContainsItem(j))
                (*mpOut) << setw(marginWidth) << _T("*");
            else
                (*mpOut) << setw(marginWidth) << _T("");

            (*mpOut) << setw(attrNameWidth) << strNewName;
        }
        else
        {
            // if old value would run into this value, we need to put this value
            // on the next line (but in the same column).  don't output
            // value but position "cursor" in the correct place
            if (strOldValue.length() >= (unsigned int)attrValueWidth)
            {
                (*mpOut) << endl;
                (*mpOut) << setw(marginWidth) << _T("");
                (*mpOut) << setw(attrNameWidth) << _T("");
                (*mpOut) << setw(attrValueWidth) << _T("");
            }
        }

        //
        // output new value
        //
        (*mpOut).width(attrValueWidth);
        if (pfcoNew && pfcoNew->GetPropSet()->GetValidVector().ContainsItem(j))
            (*mpOut) << mpCurPD->PropAsString(pfcoNew, j, (marginWidth + attrNameWidth), 0).c_str();
        else
            (*mpOut) << _T("---");

        (*mpOut) << endl;
    }
}

void cTextReportViewer::OutputAddedSummary(const cFCOReportSpecIter& ri, FCOList* pFCONameList)
{
    if (!ri.GetAddedSet()->IsEmpty())
    {
        (*mpOut) << TSS_GetString(cTW, tw::STR_ADDED) << _T(":") << endl;

        // iterate over all fcos
        const cIterProxy<iFCOIter> pSetIterAdded = ri.GetAddedSet()->GetIter();
        for (pSetIterAdded->SeekBegin(); !pSetIterAdded->Done(); pSetIterAdded->Next())
        {
            const iFCO* fco = pSetIterAdded->FCO();
            if (!IgnoreThisFCO(fco))
            {
                PrintBallotLine(*fco);

                // if we're updating, save a list of FCO names
                if (mfUpdate && pFCONameList)
                    pFCONameList->insert(fco->GetName());
            }
        }
        (*mpOut) << endl;
    }
}

void cTextReportViewer::OutputRemovedSummary(const cFCOReportSpecIter& ri, FCOList* pFCONameList)
{
    if (!ri.GetRemovedSet()->IsEmpty())
    {
        (*mpOut) << TSS_GetString(cTW, tw::STR_REMOVED) << _T(":") << endl;

        // iterate over all fcos
        const cIterProxy<iFCOIter> pSetIterRemoved = ri.GetRemovedSet()->GetIter();
        for (pSetIterRemoved->SeekBegin(); !pSetIterRemoved->Done(); pSetIterRemoved->Next())
        {
            const iFCO* fco = pSetIterRemoved->FCO();
            if (!IgnoreThisFCO(fco))
            {
                PrintBallotLine(*fco);

                // if we're updating, save a list of FCO names
                if (mfUpdate && pFCONameList)
                    pFCONameList->insert(fco->GetName());
            }
        }
        (*mpOut) << endl;
    }
}

void cTextReportViewer::OutputChangedSummary(const cFCOReportSpecIter& ri, FCOList* pFCONameList)
{
    if (ri.GetNumChanged() > 0)
    {
        (*mpOut) << TSS_GetString(cTW, tw::STR_CHANGED) << _T(":") << endl;

        // iterate over all changed fcos
        cFCOReportChangeIter changedIter(ri);
        for (changedIter.SeekBegin(); !changedIter.Done(); changedIter.Next())
        {
            const iFCO* fco = changedIter.GetNew();
            if (!IgnoreThisFCO(fco))
            {
                PrintBallotLine(*fco);

                // if we're updating, save a list of FCO names
                if (mfUpdate && pFCONameList)
                    pFCONameList->insert(fco->GetName());
            }
        }

        (*mpOut) << endl;
    }
}

void cTextReportViewer::OutputAddedDetails(const cFCOReportSpecIter& ri)
{
    if (!ri.GetAddedSet()->IsEmpty())
    {
        OutputNumDetails(tw::STR_ADDED_FILES, ri.GetAddedSet()->Size());

        // iterate over all fcos
        const cIterProxy<iFCOIter> pSetIterAdded = ri.GetAddedSet()->GetIter();
        ASSERT(pSetIterAdded != 0);
        for (pSetIterAdded->SeekBegin(); !pSetIterAdded->Done(); pSetIterAdded->Next())
        {
            const iFCO* fco = pSetIterAdded->FCO();
            if (!IgnoreThisFCO(fco))
            {
                (*mpOut) << TSS_GetString(cTW, tw::STR_ADDED_FILE_NAME) << _T(" ")
                         << mpCurNT->ToStringDisplay(fco->GetName()).c_str() << endl;

                if (FULL_REPORT == mReportingLevel)
                {
                    (*mpOut) << endl;
                    DisplayChangedProps(NULL, fco, NULL);
                    (*mpOut) << endl << endl;
                }
            }
        }

        (*mpOut) << endl;
    }
}

void cTextReportViewer::OutputRemovedDetails(const cFCOReportSpecIter& ri)
{
    if (!ri.GetRemovedSet()->IsEmpty())
    {
        OutputNumDetails(tw::STR_REMOVED_FILES, ri.GetRemovedSet()->Size());

        // iterate over all fcos
        const cIterProxy<iFCOIter> pSetIterRemoved = ri.GetRemovedSet()->GetIter();
        ASSERT(pSetIterRemoved != 0);
        for (pSetIterRemoved->SeekBegin(); !pSetIterRemoved->Done(); pSetIterRemoved->Next())
        {
            const iFCO* fco = pSetIterRemoved->FCO();
            if (!IgnoreThisFCO(fco))
            {
                (*mpOut) << TSS_GetString(cTW, tw::STR_REMOVED_FILE_NAME) << _T(" ")
                         << mpCurNT->ToStringDisplay(fco->GetName()).c_str() << endl;

                if (FULL_REPORT == mReportingLevel)
                {
                    (*mpOut) << endl;
                    DisplayChangedProps(fco, NULL, NULL);
                    (*mpOut) << endl << endl;
                }
            }
        }
        (*mpOut) << endl;
    }
}

void cTextReportViewer::OutputChangedDetails(const cFCOReportSpecIter& ri)
{
    if (ri.GetNumChanged() > 0)
    {
        OutputNumDetails(tw::STR_CHANGED_FILES, ri.GetNumChanged());

        // iterate over all changed fcos
        cFCOReportChangeIter changedIter(ri);
        for (changedIter.SeekBegin(); !changedIter.Done(); changedIter.Next())
        {
            const iFCO* fco = changedIter.GetOld();
            if (!IgnoreThisFCO(fco))
            {
                (*mpOut) << TSS_GetString(cTW, tw::STR_CHANGED_FILE_NAME) << _T(" ")
                         << mpCurNT->ToStringDisplay(fco->GetName()).c_str() << endl
                         << endl;
                DisplayChangedProps(fco, changedIter.GetNew(), &changedIter.GetChangeVector());
                (*mpOut) << endl << endl;
            }
        }
        (*mpOut) << endl;
    }
}

void cTextReportViewer::SetObjects(const std::set<std::string>& objects)
{
    if (!objects.empty())
    {
        mObjects = objects;
        mFilterFCOs = true;
    }
}

// overridables:
// These function allows derived classes to tailor reports
bool cTextReportViewer::IgnoreThisSpec(const cFCOSpecAttr* attr)
{
    return false;
}

bool cTextReportViewer::IgnoreThisFCO(const iFCO* fco)
{
    if (!mFilterFCOs)
    {
        return false;
    }
    return (mObjects.find(fco->GetName().AsString()) == mObjects.end());
}

bool cTextReportViewer::WantOutputReportHeader()
{
    return true;
}

bool cTextReportViewer::WantOutputRulesSummary()
{
    return true;
}

bool cTextReportViewer::WantOutputSpecHeader()
{
    return true;
}

bool cTextReportViewer::WantOutputObjectSummary()
{
    return true;
}

bool cTextReportViewer::WantOutputObjectDetails()
{
    return true;
}

bool cTextReportViewer::CanUpdate()
{
    return true;
}


//=========================================================================
// UTIL FUNCTION CODE
//=========================================================================

void OpenInputFile(std::ifstream& in, const TSTRING& strFile) // throw( eTextReportViewer )
{
    in.open(cStringUtil::TstrToStr(strFile).c_str());

    if (!in.is_open())
    {
        throw eTextReportViewerFileOpen(strFile);
    }
}

#if !ARCHAIC_STL
void OpenOutputFile(fixed_basic_ofstream<TCHAR>& out, const TSTRING& strFile) // throw( eTextReportViewer )
#else
void OpenOutputFile(ofstream& out, const TSTRING& strFile) // throw( eTextReportViewer )
#endif  
{
    std::string narrowFilename = cStringUtil::TstrToStr(strFile);

    // we explicitly create the file so that we control the
    // permissions on the new file.
    unlink(narrowFilename.c_str());
    int fd = open(narrowFilename.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_EXCL, 00600);
    if (fd == -1 || close(fd) != 0)
    {
        throw eTextReportViewerFileOpen(strFile);
    }

    out.open(narrowFilename.c_str());

    if (!out.is_open())
        throw eTextReportViewerFileOpen(strFile);
}

bool SpecEmpty(const cFCOReportSpecIter& ri)
{
    return (ri.GetAddedSet()->IsEmpty() && ri.GetRemovedSet()->IsEmpty() && 0 == ri.GetNumChanged());
}

bool PrintableProp(const iFCO* pfcoOld, const iFCO* pfcoNew, int j)
{
    bool fOldExists = pfcoOld && pfcoOld->GetPropSet()->GetValidVector().ContainsItem(j);
    bool fNewExists = pfcoNew && pfcoNew->GetPropSet()->GetValidVector().ContainsItem(j);

    //
    // don't output line if neither FCO has the prop
    //
    if (!fOldExists && !fNewExists)
    {
        return false;
    }

    //
    // don't output anything if both props are undefined
    //
    if (fOldExists && fNewExists &&
        pfcoOld->GetPropSet()->GetPropAt(j)->GetType() == cFCOUndefinedProp::GetInstance()->GetType() &&
        pfcoNew->GetPropSet()->GetPropAt(j)->GetType() == cFCOUndefinedProp::GetInstance()->GetType())
    {
        return false;
    }

    //
    // don't output anything if old is undef and new doesn't exist
    //
    if (fOldExists && !fNewExists &&
        pfcoOld->GetPropSet()->GetPropAt(j)->GetType() == cFCOUndefinedProp::GetInstance()->GetType())
    {
        return false;
    }

    //
    // don't output anything if new is undef and old doesn't exist
    //
    if (!fOldExists && fNewExists &&
        pfcoNew->GetPropSet()->GetPropAt(j)->GetType() == cFCOUndefinedProp::GetInstance()->GetType())
    {
        return false;
    }

    return true;
}

void cTextReportViewer::EatSection()
{
    GetChar(); // get '#'

    // eat up " section: "
    int nb = cStringUtil::TstrToStr(TSS_GetString(cTW, tw::STR_SECTION)).length();

    nb += 3; // one for each space surrounding section, and one for colon

    // eat up nb bytes
    for (int i = 0; !mpIn->eof() && i < nb; i++)
        mpIn->get();
}

//=========================================================================
//
// Methods of class cEmailReportViewer
//
//=========================================================================

cEmailReportViewer::cEmailReportViewer(const cFCOReportHeader& h,
                                       const cFCOReport&       r,
                                       const TSTRING&          address,
                                       bool                    bForceFullReport /*= false */)
    : cTextReportViewer(h, r), mbForceFullReport(bForceFullReport)
{
    mAddress = address;
}

bool cEmailReportViewer::IgnoreThisSpec(const cFCOSpecAttr* attr)
{
    // IgnoreThisSpec - returns false if the email address is specified
    // for the given spec attributes.

    if (mbForceFullReport)
        return false;

    // loop through all email addresses for this spec
    cFCOSpecAttrEmailIter emailIter(*attr);
    for (emailIter.SeekBegin(); !emailIter.Done(); emailIter.Next())
    {
        if (emailIter.EmailAddress() == mAddress)
            return false; // Don't ignore it. It applies to me.
    }

    return true; // Ignore it. It doesn't have my name on it.
}

bool cEmailReportViewer::WantOutputObjectSummary()
{
    // This area is not useful unless you're doing the ballot box thing,
    // which is not applicable to emailed reports.
    return false;
}

bool cEmailReportViewer::CanUpdate()
{
    // this derived class does not allow "DisplayReportAndHaveUserUpdateIt()"
    return false;
}


//=========================================================================
// 6/16/99 -- new reporting stuff
//=========================================================================
TSTRING cTextReportViewer::SingleLineReport()
{
    TOSTRINGSTREAM sstrReport;
    ASSERT(mpReport != 0);
    ASSERT(mpHeader != 0);

    //
    // get current time
    //
    tm* ptm = cTimeUtil::TimeToDateLocal(mpHeader->GetCreationTime());
    if (NULL == ptm)
        ThrowAndAssert(ePoly());

    //
    // format time: YYYYMMDDHHMMSS
    //
    TCHAR szDate[256];
    _tcsftime(szDate, countof(szDate), _T("%Y%m%d%H%M%S"), ptm);

    sstrReport << TSS_GetString(cTW, tw::STR_TRIPWIRE_REPORT_SHORT);
    sstrReport << _T(" ");
    sstrReport << mpHeader->GetSystemName();
    sstrReport << _T(" ");
    sstrReport << szDate;

    //
    // get report stats
    //
    int nAddedTotal         = 0;
    int nRemovedTotal       = 0;
    int nChangedTotal       = 0;
    int nViolations         = 0;
    int maxSeverityViolated = 0;

    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        // We need to at least get the max severity in this function
        cFCOReportSpecIter ri(genreIter);
        for (ri.SeekBegin(); !ri.Done(); ri.Next())
        {
            int nAdded   = ri.GetAddedSet()->Size();
            int nRemoved = ri.GetRemovedSet()->Size();
            int nChanged = ri.GetNumChanged();

            nAddedTotal += nAdded;
            nRemovedTotal += nRemoved;
            nChangedTotal += nChanged;

            if (nAdded || nRemoved || nChanged)
            {
                // update the max severity
                if (maxSeverityViolated < ri.GetAttr()->GetSeverity())
                    maxSeverityViolated = ri.GetAttr()->GetSeverity();

                nViolations += (nAdded + nRemoved + nChanged);
            }
        }
    }

    sstrReport << _T(" ") << TSS_GetString(cTW, tw::STR_VIOLATIONS_SHORT) << _T(":") << nViolations;
    sstrReport << _T(" ") << TSS_GetString(cTW, tw::STR_MAX_SEV_SHORT) << _T(":") << maxSeverityViolated;
    sstrReport << _T(" ") << TSS_GetString(cTW, tw::STR_ADDED_SHORT) << _T(":") << nAddedTotal;
    sstrReport << _T(" ") << TSS_GetString(cTW, tw::STR_REMOVED_SHORT) << _T(":") << nRemovedTotal;
    sstrReport << _T(" ") << TSS_GetString(cTW, tw::STR_CHANGED_SHORT) << _T(":") << nChangedTotal;

    tss_return_stream(sstrReport, out);
}


void cTextReportViewer::OutputParseableReport()
{
    // iterate over all genres in a report
    cFCOReportGenreIter genreIter(*mpReport);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        SetUpForNewGenre(genreIter);

        // iterate over all specs in a genre
        cFCOReportSpecIter ri(genreIter);
        for (ri.SeekBegin(); !ri.Done(); ri.Next())
        {
            // iterate over all added fcos
            const cIterProxy<iFCOIter> pSetIterAdded = ri.GetAddedSet()->GetIter();
            for (pSetIterAdded->SeekBegin(); !pSetIterAdded->Done(); pSetIterAdded->Next())
            {
                if (!IgnoreThisFCO(pSetIterAdded->FCO()))
                {
                    (*mpOut) << TSS_GetString(cTW, tw::STR_ADDED) << _T(":\t");
                    (*mpOut) << mpCurNT->ToStringDisplay(pSetIterAdded->FCO()->GetName(), true) << endl;
                }
            }

            // iterate over all removed fcos
            const cIterProxy<iFCOIter> pSetIterRemoved = ri.GetRemovedSet()->GetIter();
            for (pSetIterRemoved->SeekBegin(); !pSetIterRemoved->Done(); pSetIterRemoved->Next())
            {
                if (!IgnoreThisFCO(pSetIterRemoved->FCO()))
                {
                    (*mpOut) << TSS_GetString(cTW, tw::STR_REMOVED) << _T(":\t");
                    (*mpOut) << mpCurNT->ToStringDisplay(pSetIterRemoved->FCO()->GetName(), true) << endl;
                }
            }

            // iterate over all changed fcos
            cFCOReportChangeIter changedIter(ri);
            for (changedIter.SeekBegin(); !changedIter.Done(); changedIter.Next())
            {
                if (!IgnoreThisFCO(changedIter.GetNew()))
                {
                    (*mpOut) << TSS_GetString(cTW, tw::STR_CHANGED) << _T(":\t");
                    (*mpOut) << mpCurNT->ToStringDisplay(changedIter.GetNew()->GetName(), true) << endl;
                }
            }
        }
    }
}

char cTextReportViewer::PeekChar()
{
#if !ARCHAIC_STL    
    return char_traits<char>::to_char_type(mpIn->peek());
#else
    return (char)mpIn->peek();
#endif    
}


// TODO:BAM -- man is this function sloppy!!!
// The only reason that this function would fail would be that the text report
// was corrupted, like um, binarily corrupted. (that is, anything a text editor
// would put in here would be readable)
void cTextReportViewer::GetChar()
{
    cDebug d("cTextReportViewer::GetChar");
    ASSERT(sizeof(mCurrentChar) >= MB_CUR_MAX); // mCurrentChar holds a mb character for current locale

    // initialize mCurrentChar
    mCurrentCharSize = 0;
    for (uint32_t i = 0; i < sizeof(mCurrentChar); i++)
        mCurrentChar[i] = 0;

#if !ARCHAIC_STL    
    static const std::istream::char_type eof = std::char_traits<char>::to_char_type(std::char_traits<char>::eof());
#endif
    
    std::streampos pos = mpIn->tellg();

    for (size_t nch = 0; nch < (size_t)MB_CUR_MAX; nch++)
    {
        if (mpIn->eof() || PeekIsEOF())
        {
            // should be first byte we read
            if (nch != 0)
                throw eTextReportViewerReportCorrupt("Expected EOF");

            if (PeekIsEOF())
            {
                // get the eof char so that mpIn->eof() will return true
                mpIn->get();
            }

            d.TraceDebug(_T("Found EOF\n"));

#if !ARCHAIC_STL	    
            mCurrentChar[0]  = eof;
#else
            mCurrentChar[0]  = EOF;
#endif	    
            mCurrentCharSize = 1;

            return;
        }
        else
        {
            if (!mpIn->good())
            {
                d.TraceDebug(_T("Input stream error.\n"));
                throw eTextReportViewerReportCorrupt("Input stream error");
            }

            // get character from input stream
#if !ARCHAIC_STL	    
            std::istream::char_type ch = std::char_traits<char>::to_char_type(mpIn->get());
#else
	    char ch = mpIn->get();
#endif	    
            // add character to mb buffer
            mCurrentChar[nch] = ch;
            mCurrentCharSize++;

            // have we found a complete mb character yet?
            if (0 <= mbtowc(NULL, mCurrentChar, mCurrentCharSize))
            {
                // completed a valid mb charactter
                return;
            }
            else
            {
                // incomplete mb char -- keep reading in bytes
            }
        }
    }

    mpIn->seekg(pos);

#if !ARCHAIC_STL    
    std::istream::char_type c = std::char_traits<char>::to_char_type(mpIn->get());
#else
    char c = mpIn->get();
#endif
    
    if ((unsigned char)c > 0x7f)
    {
        mCurrentChar[0]  = c;
        mCurrentChar[1]  = 0;
        mCurrentCharSize = 1;
        return;
    }

    // sequence was not a valid mb character
    // (searched MB_CUR_MAX chars and didn't find a complete mb character)
    d.TraceDebug(_T("Invalid mb char found!\n"));
#ifdef DEBUG
    for (int j = 0; j < MB_CUR_MAX; j++)
        d.TraceDebug(_T("%u\n"), (size_t)(unsigned char)mCurrentChar[j]);
#endif
    ASSERT(false);
    throw eTextReportViewerReportCorrupt("Invalid multibyte sequence");
}

void cTextReportViewer::AppendChar(std::string& str)
{
    for (size_t s = 0; s < mCurrentCharSize; s++)
        str += mCurrentChar[s];
}
