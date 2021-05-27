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
// tripwiremain.cpp

#include "stdtripwire.h"


#include "core/debug.h"
#include "core/error.h"
#include "tw/twinit.h"
#include "tw/twutil.h"
#include "twcmdline.h"
#include "core/cmdlineparser.h"
#include "tw/configfile.h"
#include "core/errorbucketimpl.h"
#include "core/usernotifystdout.h"
#include "core/epoch.h"
#include <memory> // for auto_ptr / unique_ptr
#include <iostream>
#include <exception>

#include "tw/twstrings.h"
#include "tripwirestrings.h"
#include "tripwire.h" // package initialization

#include "fco/fcogenre.h"
#include "fco/genreswitcher.h"

#include "core/unixfsservices.h"
#include <unistd.h>

static TSTRING util_GetWholeCmdLine(int argc, const TCHAR* argv[]);

///////////////////////////////////////////////////////////////////////////////
// global new() and delete() overrides -- this is done to do performance testing
///////////////////////////////////////////////////////////////////////////////
/*
#ifdef DEBUG
#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif
static int32_t  gCurAlloc=0,
                gMaxAlloc=0;
void* operator new(size_t size)
{
    gCurAlloc += size;
    if(gCurAlloc > gMaxAlloc)
        gMaxAlloc = gCurAlloc;
    return malloc(size);
}
void operator delete(void* addr)
{
    gCurAlloc -= 4;
    free (addr);
}
#endif //_DEBUG
*/

///////////////////////////////////////////////////////////////////////////////
// terminate and unexpected handlers
// TODO: move these to a common library
///////////////////////////////////////////////////////////////////////////////
void tw_terminate_handler()
{
    fputs("### Internal Error.\n### Terminate Handler called.\n### Exiting...\n", stderr);
#if HAVE__EXIT    
    _exit(8);
#else
    exit(8);
#endif
}

#if USE_UNEXPECTED
void tw_unexpected_handler()
{
    fputs("### Internal Error.\n### Unexpected Exception Handler called.\n### Exiting...\n", stderr);

#if HAVE__EXIT 
    _exit(8);
#else
    exit(8);
#endif
}
#endif

///////////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////////
int __cdecl _tmain(int argc, const TCHAR* argv[], const TCHAR* envp[])
{

    if (CheckEpoch())
        return 8;

    int ret = 0;

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
        // Initialization
        //
        twInit.Init(argv[0]);
        TSS_Dependency(cTripwire);

        // set up the debug output
        cDebug::SetDebugLevel(cDebug::D_DEBUG /*D_DETAIL*/ /*D_NEVER*/);


        // first, get the right mode...
        TW_UNIQUE_PTR<iTWMode> pMode(cTWCmdLine::GetMode(argc, argv));
        if (!pMode.get())
        {
            // no valid mode passed; GetMode will display an appropriate string (include usage statement)
            ret = 8;
            goto exit;
        }

        // if version was requested, output version string and exit
        if (pMode.get()->GetModeID() == cTWCmdLine::MODE_VERSION)
        {
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION_LONG) << std::endl;
            ret = 0;
            goto exit;
        }

        // process the command line
        cCmdLineParser cmdLine;
        pMode->InitCmdLineParser(cmdLine);
        try
        {
            cmdLine.Parse(argc, argv);
        }
        catch (eError& e)
        {
            cTWUtil::PrintErrorMsg(e);
            TCERR << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;

            ret = 8;
            goto exit;
        }

        TSTRING commandLine = util_GetWholeCmdLine(argc, argv);

        // erase the command line
        // TODO: it might be a good idea to move this to cTWUtil
        int i;
        for (i = 1; i < argc; ++i)
            memset((char*)argv[i], 0, strlen(argv[i]) * sizeof(TCHAR));

        cCmdLineIter iter(cmdLine);
        if (iter.SeekToArg(cTWCmdLine::HELP))
        {
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            //
            //Since --help was passed, exit after emitting a mode-specific usage statement.
            TCOUT << pMode->GetModeUsage();
            ret = 8;
            goto exit;
        }

        if (iter.SeekToArg(cTWCmdLine::VERBOSE))
        {
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
        }

        // open up the config file, possibly using the passed in path
        cConfigFile    config;
        TSTRING        strConfigFile;
        cErrorReporter errorReporter;

        if (pMode->GetModeID() != cTWCmdLine::MODE_HELP)
        {
            try
            {
                //open cfg file
                cTWUtil::OpenConfigFile(config, cmdLine, cTWCmdLine::CFG_FILE, errorReporter, strConfigFile);
            }
            catch (eError& error)
            {
                TSTRING extra;
                extra += TSS_GetString(cTW, tw::STR_NEWLINE);
                extra += TSS_GetString(cTW, tw::STR_ERR_TWCFG_CANT_READ);

                cTWUtil::PrintErrorMsg(error, extra);
                ret = 8;
                goto exit;
            }
        }

        // ok, now we can initialize the mode object and have it execute
        pMode->SetCmdLine(commandLine);

        pMode->SetConfigFile(strConfigFile);

        if (!pMode->Init(config, cmdLine))
        {
            TCERR << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
            ret = 8;
            goto exit;
        }

        ret = pMode->Execute(&twInit.errorQueue);

    } //end try block

    catch (eError& error)
    {
        cTWUtil::PrintErrorMsg(error);
        ASSERT(false);
        ret = 8;
    }

    catch (std::bad_alloc& e)
    {
        // Note: We use fputs rather than TCERR as it will probably require the
        // least amount of memory to do its thing.  If we ran out of memory we
        // need to do everything we can to get this string out to the user.
        fputs("*** Fatal exception: Out of memory\n", stderr);
        fputs("*** Exiting...\n", stderr);
        ret = 8;
    }

    catch (std::exception& e)
    {
        TCERR << _T("*** Fatal exception: ");
        std::cerr << e.what() << std::endl;
        TCERR << _T("*** Exiting...\n");
        ret = 8;
    }
    catch (...)
    {
        TCERR << _T("*** Fatal exception occurred.\n");
        TCERR << _T("*** Exiting...\n");
        ret = 8;
    }

exit:


    // print out the max memory usage...
    /*
#ifdef DEBUG
    TCOUT << _T("Maximum memory footprint = ") << gMaxAlloc << std::endl;
#endif
*/


    return ret;

} //end MAIN

static TSTRING util_GetWholeCmdLine(int argc, const TCHAR* argv[])
{
    TSTRING tstrRet;

    bool wipeNextItem = false;

    for (int i = 0; i < argc; i++)
    {
        if (wipeNextItem)
        {
            tstrRet += _T("***** ");
            wipeNextItem = false;
        }
        else
        {
            tstrRet += argv[i];
            tstrRet += _T(" ");

            // Passwords passed on the command line are not saved
            if (_tcsncmp(argv[i], _T("-P"), 2) == 0 || _tcsncmp(argv[i], _T("-Q"), 2) == 0 ||
                _tcscmp(argv[i], _T("--local-passphrase")) == 0 || _tcscmp(argv[i], _T("--site-passphrase")) == 0)
            {
                wipeNextItem = true;
            }
        }
    }

    return (tstrRet);
}
