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
// twprintmain.cpp
//

#include "stdtwprint.h"

#include "twprintstrings.h"
#include "twprintcmdline.h"

#include "core/error.h"
#include "core/cmdlineparser.h"
#include "core/errorbucketimpl.h"
#include "core/usernotifystdout.h"
#include "core/epoch.h"

#include "core/debug.h"

#include "tw/twinit.h"
#include "tw/configfile.h"
#include "tw/twutil.h"
#include "tw/twstrings.h"

#include "twprint.h" // package initialization

#include "core/unixfsservices.h"
#include <unistd.h>

#include <memory> // for auto_ptr
#include <iostream>
#include <exception>

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

// Exception specifications removed as a misfeature in C++17
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

int __cdecl _tmain(int argc, const TCHAR* argv[])
{
    int ret = 0;


    if (CheckEpoch())
        return 1;

    cTWInit twInit;

    try
    {
        // set unexpected and terminate handlers
        // Note: we do this before Init() in case it attempts to call these handlers
        // TODO: move this into the Init() routine

        EXCEPTION_NAMESPACE set_terminate(tw_terminate_handler);
#if USE_UNEXPECTED
        EXCEPTION_NAMESPACE set_unexpected(tw_unexpected_handler);
#endif

        twInit.Init(argv[0]);
        TSS_Dependency(cTWPrint);
        // init twprint strings

        cDebug::SetDebugLevel(cDebug::D_DEBUG);

        // **** let's try a new way of doing things!
        // first, process the command line
        if (argc < 2)
        {
            TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_VERSION) << std::endl;
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

        // Next, set the mode... exit with error if now valid mode specified.
        TW_UNIQUE_PTR<iTWMode> pMode(cTWPrintCmdLine::GetMode(argc, argv));
        if (!pMode.get())
        {
            // no valid mode passed; GetMode will display an appropriate string (include usage statement)
            ret = 1;
            goto exit;
        }

        cCmdLineParser cmdLine;
        // Call InitCmdLineParser for this mode:
        pMode->InitCmdLineParser(cmdLine);

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

        // erase the command line
        // TODO: it might be a good idea to move this to cTWUtil
        int i;
        for (i = 1; i < argc; ++i)
            memset((char*)argv[i], 0, strlen(argv[i]) * sizeof(char));

        cCmdLineIter iter(cmdLine);
        if (iter.SeekToArg(cTWPrintCmdLine::HELP))
        {
            TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            // Output a specific usage statement for this mode.
            TCOUT << pMode->GetModeUsage();
            ret = 1;
            goto exit;
        }

        // open up the config file, possibly using the passed in path
        cConfigFile    config;
        cErrorReporter errorReporter;

        if (pMode->GetModeID() != cTWPrintCmdLine::MODE_HELP)
        {
            try
            {
                //open cfg file
                TSTRING cfgPath;
                cTWUtil::OpenConfigFile(config, cmdLine, cTWPrintCmdLine::CFG_FILE, errorReporter, cfgPath);
                pMode->SetConfigFile(cfgPath);
            }
            catch (eError& error)
            {
                cTWUtil::PrintErrorMsg(error);
                TCERR << TSS_GetString(cTW, tw::STR_ERR_TWCFG_CANT_READ) << std::endl;
                ret = 1;
                goto exit;
            }
        }

        // ok, now we can initialize the mode object and have it execute
        if (!pMode->Init(config, cmdLine))
        {
            // TODO -- Init should spit out the error msg...
            // I don't think this error message is necessary
            //TCERR << _T("Problem initializing twprint!") << std::endl;
            TCERR << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
            ret = 1;
            goto exit;
        }
        ret = pMode->Execute(&twInit.errorQueue);

    } //end try block

    catch (eError& error)
    {
        cTWUtil::PrintErrorMsg(error);
        ASSERT(false);
        ret = 1;
    }

    catch (std::bad_alloc& e)
    {
        TCERR << _T("*** Fatal exception: Out of memory ");
        TCERR << _T("*** Exiting...\n");
        ret = 1;
    }

    catch (std::exception& e)
    {
        TCERR << _T("*** Fatal exception: ");
        std::cerr << e.what() << std::endl;
        TCERR << _T("*** Exiting...\n");
        ret = 1;
    }

    /*
    catch (...)
    {
        TCERR << _T("*** Fatal exception occurred.\n");
        TCERR << _T("*** Exiting...\n");
    }
    */

exit:


    return ret;

} //end MAIN
