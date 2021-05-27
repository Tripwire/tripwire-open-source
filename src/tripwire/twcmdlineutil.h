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
// twcmdlineutil.h
//
// cTWCmdLineUtil -- helper functions that twcmdline.cpp uses...
#ifndef __TWCMDLINEUTIL_H
#define __TWCMDLINEUTIL_H

#include "mailmessage.h"
#include "core/error.h"

class cFCOSpecList;
class cGenreSpecListVector;
class cErrorQueue;
class cFCOReport;
class cFCOReportHeader;
class cTWModeCommon;
class cErrorBucket;
class cFCODatabaseFile;

class cTWCmdLineUtil
{
public:
    static void ParsePolicyFile(cGenreSpecListVector& genreSpecList,
                                const TSTRING&        fileName,
                                TSTRING&              strSiteKeyFile,
                                cErrorQueue*          pQueue); // throw (eError)
    // this parses the given (encrypted) policy file and puts the results into the passed in specList. If an error occurs,
    // pQueue will be filled up and an exception will be thrown.
    // Note: If the policy file is not encrypted, an warning message will be printed to standard error.
    static void ParseTextPolicyFile(cGenreSpecListVector& genreSpecList,
                                    const TSTRING&        fileName,
                                    TSTRING&              strSiteKeyFile,
                                    cErrorQueue*          pQueue); // throw (eError)
    // same as ParsePolicyFile above, but it expects a plain text policy file as input. Used during policy update
    // mode

    static void TrimPropsFromSpecs(cFCOSpecList& specList, const TSTRING& propsToIgnore); // throw (eError)
        // converts propsToIgnore into a property vector and removes all the properties in the
        // vector from all specs in specList; throws eError() the string is in an invalid format.
    static void TrimSpecsByName(cFCOSpecList& specList, const TSTRING specName); // throw (eError)
        // removes all specs from the list whose name doesn't match specName. If every name is
        // removed from the spec list, an error msg is printed and eError is thrown
    static void TrimSpecsBySeverity(cFCOSpecList& specList, int severity); // throw (eError)
        // removes all specs from the list whose severity is less than the passed in value.
        // If every name is removed from the spec list, an error msg is printed and eError is thrown
    static void TrimSpecsBySeverityName(cFCOSpecList& specList, const TSTRING& severity); // throe (eError)
        // converts the severity name to a number and calls TrimSpecsBySeverity(). Prints an error
        // message and throws eError() if it falis (can't convert name to int)
    static bool ReportContainsFCO(const cFCOReport& report);
    // returns true if the report contains at least one FCO in it.

    static int GetICReturnValue(const cFCOReport& report);
    // determines the appropriate return value for an integrity check; the following
    // values are ORed together to produce the final return value:
    // 1 = files were added
    // 2 = files were removed
    // 4 = files were changed

    static bool EmailReport(const cFCOReportHeader& header, const cFCOReport& report, const cTWModeCommon* modeCommon);
    // email all interested parties the contents of this report, by opening a pipe to
    // the given email program (sendmail or some other MTA) and writing the report in
    // its text form. This only works in unix.
    // if an error occurs, then a message is displayed and false is returned

    static bool SendEmailTestMessage(const TSTRING& mAddress, const cTWModeCommon* modeCommon);
    // email a test message to the specified address

    static void VerifyPolicy(cGenreSpecListVector& genreSpecList, cFCODatabaseFile& dbFile); //throw (eError)
        // makes sure that the given policy is appropriate for the given database. It is appropriate if it is
        // _exactly_ the same, including equal numbers and types of genres.
    static void VerifySpecs(const cFCOSpecList& parsedList, const cFCOSpecList& dbList); //throw (eError)
        // determines whether the policy in the policy file (parsedList) is equal to the database's current
        // policy (dbList). If it is not, an exception is thrown
};

#endif
