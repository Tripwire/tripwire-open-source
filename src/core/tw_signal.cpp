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
// tw_signal.cpp
#include "stdcore.h"
#include "tw_signal.h"
#include "corestrings.h"
#include <signal.h>
#include <stdlib.h>

static void util_SignalHandler(int sig);

static void tw_psignal(int sig, const TCHAR* s);

tw_sighandler_t tw_signal(int sig, tw_sighandler_t pFunc)
{
    return signal(sig, pFunc);
}

int tw_raise(int sig)
{
    return raise(sig);
}

tw_sighandler_t tw_sigign(int sig)
{
    return signal(sig, SIG_IGN);
}

//////////////////////////////////////////////////////////////////////
// tw_HandleSignal -- Take a given signal and install a handler for
//      it, which will then exit with the supplied exit value
tw_sighandler_t tw_HandleSignal(int sig)
{
    return signal(sig, util_SignalHandler);
}

void util_SignalHandler(int sig)
{
    //If we're on unix, let's print out a nice error message telling
    //the user which signal we've recieved.
    tw_psignal(sig, (TSS_GetString(cCore, core::STR_SIGNAL).c_str()));
    exit(8);
}

/* For the morbidly curious, here's a thread where a POSIX standards committee
   wrings its hands about how to define NSIG: http://austingroupbugs.net/view.php?id=741#c1834 */
#ifndef NSIG
#    ifdef SIGMAX
#        define NSIG (SIGMAX + 1)
#    else
#        define NSIG 32
#    endif
#endif

void tw_psignal(int sig, const TCHAR* str)
{
    const TCHAR* siglist[] =    { _T("Unknown Signal"),
                                  _T("Hangup"),
                                  _T("Interrupt"),
                                  _T("Quit"),
                                  _T("Illegal Instruction"),
                                  _T("Trace/Breakpoint Trap"),
                                  _T("Abort"),
                                  _T("Emulation Trap"),
                                  _T("Arithmetic Exception"),
                                  _T("Killed"),
                                  _T("Bus Error"),
                                  _T("Segmentation Fault"),
                                  _T("Bad System Call"),
                                  _T("Broken Pipe"),
                                  _T("Alarm Clock"),
                                  _T("Terminated"),
                                  _T("User Signal 1"),
                                  _T("User Signal 2"),
                                  _T("Child Status Changed"),
                                  _T("Power-Fail/Restart"),
                                  _T("Virtual Timer Expired"),
                                  _T("Profiling Timer Expired"),
                                  _T("Pollable Event"),
                                  _T("Window Size Change"),
                                  _T("Stopped (signal)"),
                                  _T("Stopped (user)"),
                                  _T("Continued"),
                                  _T("Stopped (tty input)"),
                                  _T("Stopped (tty output)"),
                                  _T("Urgent Socket Condition"),
                                  _T("File Lock Lost")};

    _ftprintf(stderr, _T("%s %s\n"), str, sig < NSIG ? siglist[sig] : siglist[0]);
}
