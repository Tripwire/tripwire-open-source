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
// siggenmain.cpp
#include "stdsiggen.h"


#include "core/types.h"
#include "core/tchar.h"
#include "core/debug.h"
#include "core/error.h"

#include "siggencmdline.h"
#include "core/cmdlineparser.h"
#include "core/errorbucketimpl.h"
#include "core/usernotifystdout.h"
#include "core/epoch.h"
#include "fs/fsstrings.h"
#include "tw/twstrings.h"
#include "tw/twutil.h" // for cTWUtil::PrintErrorMsg
#include "siggenstrings.h"

#include "siggen.h"

#include <memory>
#include <iostream>
#include <exception>

#include "core/unixfsservices.h"
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////
// terminate and unexpected handlers
// TODO: move these to a common library
///////////////////////////////////////////////////////////////////////////////
void tw_terminate_handler()
{
    fputs("### Internal Error.\n### Terminate Handler called.\n### Exiting...\n", stderr);
#if HAVE__EXIT
    _exit(1);
#else
    exit(1);
#endif    
}

#if USE_UNEXPECTED
void tw_unexpected_handler()
{
    fputs("### Internal Error.\n### Unexpected Exception Handler called.\n### Exiting...\n", stderr);
#if HAVE__EXIT
    _exit(1);
#else
    exit(1);
#endif
}
#endif

static void SiggenInit()
{
    TSS_Dependency(cSiggen);

    static cUserNotifyStdout unStdout;
    static cErrorTracer      et;
    static cErrorReporter    er;

    //
    // initialize iUserNotify
    //
    iUserNotify::SetInstance(&unStdout);
    iUserNotify::GetInstance()->SetVerboseLevel(iUserNotify::V_NORMAL);

    //
    // set up the file system services
    //
    static cUnixFSServices fss;
    iFSServices::SetInstance(&fss);

    //
    // set up an error bucket that will spit things to stderr
    //
    et.SetChild(&er);
}

int __cdecl _tmain(int argc, const TCHAR** argv)
{
    int ret = 0;

    if (CheckEpoch())
        return 1;

    try
    {
        // set unexpected and terminate handlers
        // Note: we do this before Init() in case it attempts to call these handlers
        // TODO: move this into the Init() routine
        EXCEPTION_NAMESPACE set_terminate(tw_terminate_handler);
#if USE_UNEXPECTED
        EXCEPTION_NAMESPACE set_unexpected(tw_unexpected_handler);
#endif
        //cTWInit twInit( argv[0] );
        SiggenInit();

        cDebug::SetDebugLevel(cDebug::D_DETAIL);
        cSiggenCmdLine siggen;

        // first, process the command line
        if (argc < 2)
        {
            TCOUT << TSS_GetString(cSiggen, siggen::STR_SIGGEN_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;

            ret = 1;
            goto exit;
        }

        //
        // Display the version info...
        // this is quick and dirty ... just the way I like it :-) -- mdb
        //
        if (_tcscmp(argv[1], _T("--version")) == 0)
        {
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION_LONG) << std::endl;
            ret = 0;
            goto exit;
        }


        cCmdLineParser cmdLine;
        siggen.InitCmdLineParser(cmdLine);
        try
        {
            cmdLine.Parse(argc, argv);
        }
        catch (eError& e)
        {
            cTWUtil::PrintErrorMsg(e);
            TCERR << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;

            ret = 1;
            goto exit;
        }

        cCmdLineIter iter(cmdLine);
        if (iter.SeekToArg(cSiggenCmdLine::HELP))
        {
            TCOUT << TSS_GetString(cSiggen, siggen::STR_SIGGEN_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            TCOUT << TSS_GetString(cSiggen, siggen::STR_SIGGEN_USAGE) << std::endl;
            ret = 1;
            goto exit;
        }

        if (!siggen.Init(cmdLine))
        {
            TCOUT << TSS_GetString(cSiggen, siggen::STR_SIGGEN_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            TCOUT << TSS_GetString(cSiggen, siggen::STR_SIGGEN_USAGE) << std::endl;
            ret = 1;
            goto exit;
        }
        ret = siggen.Execute();

    } //end try block
    catch (eError& error)
    {
        cErrorReporter::PrintErrorMsg(error);
        ASSERT(false);
    }

exit:

    return ret;
} //end MAIN
