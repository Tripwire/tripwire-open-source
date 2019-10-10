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
// twinit.cpp
//
// cTWInit -- initializes global objects for tw executables
//

#include "stdtw.h"

#include "twinit.h"
#include "fco/genreswitcher.h"
#include "core/fsservices.h"
#include "core/usernotifystdout.h"
#include "core/errorbucketimpl.h"
#include "core/timeconvert.h"
#include "fco/fconame.h"
#include "tw/systeminfo.h"
#include "core/twlocale.h"
#include "core/codeconvert.h"

//////////////////
// the headers for objects we will need to register in RegisterAllSerializable() below.
#include "core/serializerimpl.h"
#include "fs/fsobject.h"
#include "fco/fcospecimpl.h"
#include "fco/fconame.h"
#include "tw/fcoreport.h"
#include "fco/fcosetimpl.h"
#include "fco/fcospechelper.h"
#include "fco/fcospecattr.h"
#include "fs/fspropdisplayer.h"
#include "tw/fcodatabasefile.h"
#include "core/fileheader.h"
#include "core/serstring.h"
#include "tw/headerinfo.h"
#include "fs/fspropset.h"
/////////////////

// Used for package initialization
//
#include "core/package.h"
#include "tw/tw.h"

#include "fco/fconame.h"

#include "fs/fs.h" // object initialization
#include "core/unixfsservices.h"
#include "core/tw_signal.h" // to ignore SIGPIPE

#if IS_AROS
#include <errno.h>

#if HAVE_PROTO_EXEC_H
#include <proto/exec.h>
#endif

#if HAVE_PROTO_BSDSOCKET_H
#include <proto/bsdsocket.h>
#endif

#if HAVE_BSDSOCKET_SOCKETBASETAGS_H
#include <bsdsocket/socketbasetags.h>
#endif

static bool aros_socketbase_init();
#endif

//=============================================================================
// cTWInit_i
//=============================================================================
class cTWInit_i
{
public:
    cTWInit_i();
    ~cTWInit_i();
    // both shouldn't throw exceptions

    iFSServices*      pFSServices;
    cUserNotifyStdout unStdout;
    cErrorTracer      et;
    cErrorReporter    er;
};

cTWInit_i::cTWInit_i() : pFSServices(NULL)
{
    // package initialization
    //

    TSS_Dependency(cFS);
}

cTWInit_i::~cTWInit_i()
{
    delete pFSServices;
}

///////////////////////////////////////////////////////////////////////////////
// RegisterAllSerializable() -- Register all serializable objects
///////////////////////////////////////////////////////////////////////////////
static void RegisterAllSerializable()
{
    // Reference count objects
    cSerializerImpl::RegisterSerializableRefCt(CLASS_TYPE(cFSObject), cFSObject::Create);
    cSerializerImpl::RegisterSerializableRefCt(CLASS_TYPE(cFCOSpecImpl), cFCOSpecImpl::Create);
    cSerializerImpl::RegisterSerializableRefCt(CLASS_TYPE(cFCOSpecAttr), cFCOSpecAttr::Create);

    // Non-reference count objects
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCODatabaseFile), cFCODatabaseFile::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cErrorQueue), cErrorQueue::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCOName), cFCOName::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCOReport), cFCOReport::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCOSetImpl), cFCOSetImpl::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCOSpecStopPointSet), cFCOSpecStopPointSet::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCOSpecNoChildren), cFCOSpecNoChildren::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cSerializableNString), cSerializableNString::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cSerializableWString), cSerializableWString::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cHeaderInfo), cHeaderInfo::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCODbHeader), cFCODbHeader::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFCOReportHeader), cFCOReportHeader::Create);
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFSPropSet), cFSPropSet::Create);

    // prop displayers
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cFSPropDisplayer), cFSPropDisplayer::Create);
}

///////////////////////////////////////////////////////////////////////////////
// SetExeDir
///////////////////////////////////////////////////////////////////////////////
static bool SetExeDir(const TSTRING& strArgv0)
{
    ASSERT(!strArgv0.empty());

    // record executable directory
    TSTRING strFullPath;
    if (iFSServices::GetInstance()->GetExecutableFilename(strFullPath, strArgv0) && !strFullPath.empty())
    {
#if USES_DEVICE_PATH
        strFullPath = cDevicePath::AsPosix(strFullPath);
#endif
        cSystemInfo::SetExePath(strFullPath);

        TSTRING::size_type s = strFullPath.find_last_of(_T('/'));
        // paths will always come back from GetExecutableFilename delimited with a forward slash
        ASSERT(TSTRING::npos != s);
        if (TSTRING::npos != s)
            strFullPath.resize(s);
        cSystemInfo::SetExeDir(strFullPath);
    }
    else // error
    {
        ASSERT(false);
        cSystemInfo::SetExeDir(_T(""));
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cTWInit::cTWInit()
{
    mpData = new cTWInit_i();
}

///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cTWInit::~cTWInit()
{
    delete mpData;

    //
    // clear out the fco name tbl
    // this helps the mem leak quest...
    //
    cFCOName::ClearNameTable();
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
void cTWInit::Init(const TSTRING& strArgv0)
{
    // package initialization...
    //
    TSS_Dependency(cTW); // initialize all packages on program startup (exes
                         // should call this function (cTWInit::Init) on startup

    // we require 8-bit bytes for some functionality
    //ASSERT(sizeof(byte) == sizeof(uint8_t));

    //
    // set debug level
    // NOTE:BAM -- this is also being set in cCore::cCore() because
    // things in that constructor need to have a debug output as well.
    // so this code here is redundant.
    //
    cDebug::SetDebugLevel(cDebug::D_DEBUG);
    cDebug::AddOutTarget(cDebug::OUT_STDOUT);

    // ------------------------------------------------------------
    // BEG:RAD -- Does not belong here, moved to cCore::cCore!
    // ------------------------------------------------------------
    //
    // initialize locale
    //
    // cTWLocale::InitGlobalLocale();
    // ------------------------------------------------------------
    // END:RAD
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // BEG:RAD -- Does not belong here, moved to cCore::cCore!
    // ------------------------------------------------------------
    // initialize code converter ( inits itself on creation,
    // but initializing it here lets us see any errors immediately )
    //
    //  iCodeConverter::GetInstance();
    // ------------------------------------------------------------
    // END:RAD
    // ------------------------------------------------------------

#if IS_AROS
    aros_socketbase_init();
#endif
    //
    // set up the file system services
    //
    mpData->pFSServices = new cUnixFSServices;
    ASSERT(mpData->pFSServices != 0);
    iFSServices::SetInstance(mpData->pFSServices);

    //
    // save directory in which this executable resides
    //
    SetExeDir(strArgv0);

    //
    // save current time
    //
    cSystemInfo::SetExeStartTime(cTimeUtil::GetTime());

    // assure that a default genre has been set and...
    ASSERT(cGenre::GENRE_INVALID != cGenreSwitcher::GetInstance()->GetDefaultGenre());
    // ...switch to it
    cGenreSwitcher::GetInstance()->SelectGenre(cGenreSwitcher::GetInstance()->GetDefaultGenre());

    // initialize iUserNotify
    iUserNotify::SetInstance(&mpData->unStdout);
    iUserNotify::GetInstance()->SetVerboseLevel(iUserNotify::V_NORMAL);

    // register serializable objects
    RegisterAllSerializable();

    // set up an error bucket that will spit things to stderr
    mpData->et.SetChild(&mpData->er);
    errorQueue.SetChild(&mpData->et);

#if SUPPORTS_POSIX_SIGNALS
    // ignore SIGPIPE
    tw_sigign(SIGPIPE); //TODO: somebody add comment here!

    // We'll install handlers for each signal that would cause a core dump.
    // the disposition will still be to exit, just without producing a core file.
    // I'll emit an error statement that indicates the interrupt before the exit
    // call.
    tw_HandleSignal(SIGBUS);  // Handle Bus error signals without dumping core...
    tw_HandleSignal(SIGSEGV); // Segmentation fault
    tw_HandleSignal(SIGQUIT); // Quit signal - issued from terminal (CTRL-\)
    tw_HandleSignal(SIGILL);  // Illegal instruction - probably won't be an issue.
#    ifndef DEBUG
    tw_HandleSignal(SIGTRAP); // We don't want to mess up the debugger in debug builds...
#    endif
    tw_HandleSignal(SIGABRT); // Supposedly we can only get this signal by calling abort()
                              // ourselves, but to be on the safe side...
#    ifdef SIGEMT
    tw_HandleSignal(SIGEMT); // Emulator trap.
#    endif
#    ifdef SIGSYS
    tw_HandleSignal(SIGSYS); // Bad system call.
#    endif
    tw_HandleSignal(SIGFPE); // Floating point exception.

#    ifdef SIGXCPU
    tw_HandleSignal(SIGXCPU); // CPU time exceeded.  Might very well be an issue for us.
#    endif

#    ifdef SIGXFSZ
    tw_HandleSignal(SIGXFSZ); // File size limit exceeded.
#    endif

#endif
}

#if IS_AROS
struct Library* SocketBase = 0;

bool aros_socketbase_init()
{
    if (!(SocketBase = OpenLibrary("bsdsocket.library", 4)))
    {
        printf("Failed to load socket library");
        return 0;
    }

    if (SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))),
                       (IPTR)&errno,
                       SBTM_SETVAL(SBTC_HERRNOLONGPTR),
                       (IPTR)&errno,
                       TAG_DONE))
    {
        printf("Failed to init socket library");
        return 0;
    }
    return 1;
}
#endif
