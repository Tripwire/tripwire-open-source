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
// textreportviewer.h -- definitions for textreportviewer class
//

#ifndef __TEXTREPORTVIEWER_H
#define __TEXTREPORTVIEWER_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __ERROR_H
#include "core/error.h"
#endif
#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class cFCOReport;
class cFCOReportSpecIter;
class cFCOName;
class cFCOReportHeader;
class cFCOReportGenreIter;
class iFCOPropDisplayer;
class cFCOPropVector;
class iFCO;
class cFCOSpecAttr;
class iFCONameTranslator;
class cErrorQueueIter;
//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

TSS_EXCEPTION(eTextReportViewer, eError);
TSS_EXCEPTION(eTextReportViewerFileOpen, eTextReportViewer);
TSS_EXCEPTION(eTextReportViewerEditorLaunch, eTextReportViewer);
TSS_EXCEPTION(eTextReportViewerReportCorrupt, eTextReportViewer);


///////////////////////////////////////////////////////////////////////////////
// cTextReportViewer
//
class cTextReportViewer
{
public:
    //
    // ctors and dtors
    //
    cTextReportViewer(const cFCOReportHeader& h, cFCOReport& r);
    cTextReportViewer(const cFCOReportHeader& h, const cFCOReport& r);
    virtual ~cTextReportViewer();
    int Init(const cFCOReportHeader& h, cFCOReport& r);

    enum ReportingLevel
    {
        SINGLE_LINE,
        PARSEABLE,
        SUMMARY_ONLY,
        CONCISE_REPORT,
        FULL_REPORT
    };

    //
    // basic functionality
    //
    void DisplayReportAndHaveUserUpdateIt(
        const TSTRING& edName, ReportingLevel level = FULL_REPORT); //throw (eFSServices, eTextReportViewer, eInternal);
    // outputs the given report to a temp file, opens an editor, has
    // the user view changes to the database, and, by selecting FCO entries,
    // chooses which changes to write to the database.  Unchecked entries
    // are removed from the report
    // edName is the name of the editor to use to update the report

    virtual void PrintTextReport(const TSTRING& strFilename,
                                 ReportingLevel level = FULL_REPORT); //throw (eTextReportViewer);
    // if strFilename is "-", will print to TCOUT
    virtual void PrintTextReport(TOSTREAM& ostr, ReportingLevel level = FULL_REPORT); //throw (eTextReportViewer);
        // prints the report to the specified ostream

    TSTRING SingleLineReport();

    int GetMaxSeverityViolated();
    int GetNumberViolations();

    void SetObjects(const std::set<std::string>& objects);

protected:
    //
    // don't let C++ create these functions
    //
    cTextReportViewer();
    cTextReportViewer(const cTextReportViewer&);
    cTextReportViewer& operator=(const cTextReportViewer&);

    //
    // private enums
    //
    enum
    {
        DETAILS_MARGIN = 4
    };

    enum
    {
        TOKEN_GENRE,
        TOKEN_BALLOT_BOX,
        TOKEN_EOF
    };

    //
    // private structures
    //
    struct RuleSummaryLine
    {
        TSTRING mSpecName;
        int     mSeverity;
        int     mAddedObjects;
        int     mRemovedObjects;
        int     mChangedObjects;
        TSTRING mStartPoint;

        RuleSummaryLine() : mSeverity(0), mAddedObjects(0), mRemovedObjects(0), mChangedObjects(0)
        {
        }
        RuleSummaryLine(const RuleSummaryLine& rhs)
        {
            *this = rhs;
        }
        RuleSummaryLine& operator=(const RuleSummaryLine& rhs)
        {
            mSpecName       = rhs.mSpecName;
            mSeverity       = rhs.mSeverity;
            mAddedObjects   = rhs.mAddedObjects;
            mRemovedObjects = rhs.mRemovedObjects;
            mChangedObjects = rhs.mChangedObjects;
            mStartPoint     = rhs.mStartPoint;

            return *this;
        }
    };

    //
    // private typedefs
    //
    typedef std::set<cFCOName>                FCOList;
    typedef std::map<cGenre::Genre, FCOList*> GenreList;
    typedef std::list<RuleSummaryLine>        ObjectSummaryList;

    //
    // private util functions
    //
    bool LaunchEditorOnFile(const TSTRING& strFilename,
                            const TSTRING& edName); //throw (eFSServices, eTextReportViewer);
    void OutputTextReport();                        //throw (eTextReportViewer);

    void PrintGenre(const cFCOReportGenreIter& genreIter); //throw (eTextReportViewer);
    void OutputGenreDelimiter(cGenre::Genre g, bool fDenoteBallotSection = false);
    void OutputSectionDelimiter(int nString);
    void OutputNumDetails(int nString, int nObjects);

    void PrintBallotLine(const iFCO& FCO);
    void DisplayChangedProps(const iFCO* const pfcoOld, const iFCO* const pfcoNew, const cFCOPropVector* pv);
    void SetUpForNewGenre(const cFCOReportGenreIter& genreIter); // call this after changing genres

    void OutputReportHeader();
    void OutputRulesSummary();
    void OutputRulesSummary(const cFCOReportGenreIter& genreIter);
    void CollateRulesSummary(const cFCOReportGenreIter& genreIter, ObjectSummaryList& summaryList);
    void OutputSpecHeader(const cFCOReportSpecIter& ri);
    void OutputFooter();

    void PrintErrors();
    void ReportError(const cErrorQueueIter& eqIter);

    void OutputObjectSummary();
    void OutputObjectSummary(const cFCOReportGenreIter& genreIter);
    void OutputAddedSummary(const cFCOReportSpecIter& ri, FCOList* pFCONameList);
    void OutputRemovedSummary(const cFCOReportSpecIter& ri, FCOList* pFCONameList);
    void OutputChangedSummary(const cFCOReportSpecIter& ri, FCOList* pFCONameList);

    void OutputObjectDetails();
    void OutputObjectDetails(const cFCOReportGenreIter& genreIter);
    void OutputAddedDetails(const cFCOReportSpecIter& ri);
    void OutputRemovedDetails(const cFCOReportSpecIter& ri);
    void OutputChangedDetails(const cFCOReportSpecIter& ri);

    bool    IsChecked();
    TSTRING GetGenre();
    void    InitOStream();
    void    GetReportNumbers();

    void ReadTextReport(const TSTRING& strFilename);

    void GetChar();
    char PeekChar();
    bool PeekIsEOF();
    void AppendChar(std::string& str);

    void EatSection();
    int  FindNextLineToken();
    void GetGenreInfo(FCOList** ppCurList);
    void GetBallotInfo(FCOList* pCurList);
    void GetFCONameFromBallotLine(cFCOName& fcoName); //throw (eTextReportViewer)
    void RemoveFCOsFromReport();                      //throw (eTextReportViewer)

    void OutputParseableReport();

    //
    // overridables:
    //

    virtual bool IgnoreThisSpec(const cFCOSpecAttr* attr);
    virtual bool IgnoreThisFCO(const iFCO* fco);
    virtual bool WantOutputReportHeader();
    virtual bool WantOutputRulesSummary();
    virtual bool WantOutputSpecHeader();
    virtual bool WantOutputObjectSummary();
    virtual bool WantOutputObjectDetails();
    // These function allows derived classes to tailor the report's apearance

    virtual bool CanUpdate();
    // override this to return false if DisplayReportAndHaveUserUpdateIt should not be allowed

    //
    // data members
    //
    TOSTREAM*               mpOut;
    std::ifstream*          mpIn;
    bool                    mfUpdate;
    const cFCOReportHeader* mpHeader;
    const cFCOReport*       mpReport;
    int                     mMaxSeverityViolated;
    int                     mNumberViolations;
    int                     mErrorNum;
    GenreList               mFCOsRemoveFromReport; // collection of filenames to remove from report.
    ReportingLevel          mReportingLevel;
    iFCOPropDisplayer*      mpCurPD;
    iFCONameTranslator*     mpCurNT;
    bool                    mfGotNumbers;
    char                    mCurrentChar[6];
    size_t                  mCurrentCharSize;
    bool                    mFilterFCOs;
    std::set<std::string>   mObjects;
};


///////////////////////////////////////////////////////////////////////////////
//
// cEmailReportViewer
//
// this class is like cTextReportViewer, except that it only generates a report
// for the rules associated with the specified email address in the policy file.
//
class cEmailReportViewer : public cTextReportViewer
{
public:
    // constructor must be provided an email recipient's address
    cEmailReportViewer(const cFCOReportHeader& h,
                       const cFCOReport&       r,
                       const TSTRING&          address,
                       bool                    bForceFullReport = false);


protected:
    // overrides
    virtual bool IgnoreThisSpec(const cFCOSpecAttr* attr);
    virtual bool WantOutputObjectSummary();
    virtual bool CanUpdate();

    // data members
    TSTRING mAddress;
    bool    mbForceFullReport;
};


#endif // __TEXTREPORTVIEWER_H
