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
// tw_signal.h -- a wrapper around signal(), needed because of linux
//      build issues
//
#ifndef __TW_SIGNAL_H
#define __TW_SIGNAL_H

#include <signal.h>

#ifdef HAVE_SIGNUM_H
#include <signum.h> // the signal number constants
#endif
#ifdef HAVE_BITS_SIGNUM_H
#include <bits/signum.h>
#endif

extern "C"
{
    typedef void (*tw_sighandler_t)(int);
}

tw_sighandler_t tw_signal(int sig, tw_sighandler_t pFunc);
int             tw_raise(int sig);

///////////////////////////////////////////////////////////////////////////////
// tw_sigign -- wrapper around tw_signal(XXX, SIG_IGN)
//
tw_sighandler_t tw_sigign(int sig);

///////////////////////////////////////////////////////////////////////////////
// tw_HandleSignal  --wrapper around signal( XXX, <our handler function> )
//
tw_sighandler_t tw_HandleSignal(int sig);


#endif //__TW_SIGNAL_H
