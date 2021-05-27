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
// usernotify.h
//
// iUserNotify -- interface for notifying the user of events
#ifndef __USERNOTIFY_H
#define __USERNOTIFY_H

#ifndef __DEBUG_H
#include "debug.h"
#endif
#ifndef va_start
#include <cstdarg>
#endif

class iUserNotify
{
public:
    // singleton interface; caller is responsible for deleting pointer;
    static iUserNotify* GetInstance();
    static void         SetInstance(iUserNotify* pInst);

    virtual void Notify(int verboseLevel, const TCHAR* format, ...);
    // notify the user that an event has occured. The outcome of this operation is
    // dependant on the type of object that is implementing this interface (for example,
    // a console application would want an iUserNotify that prints things to stdout)
    // If the current verbosity level is less than verboseLevel, nothing will happen.
    // All output should be sent through the cDisplayEncoder beforehand
    // TODO:BAM -- enforce this somehow?

    virtual void SetVerboseLevel(int level);
    virtual int  GetVerboseLevel() const;
    // get/set the current verbosity level. Notify()s that occur whose verbosity level
    // is greater than the current level will not be processed.

    // a convenience enumeration; no one is bound by law to use these
    enum VerboseLevel
    {
        V_SILENT  = 0,
        V_NORMAL  = 1,
        V_VERBOSE = 2
    };
    //
    // convenience methods for notifying at these three levels...
    //
    void NotifySilent(const TCHAR* format, ...);
    void NotifyNormal(const TCHAR* format, ...);
    void NotifyVerbose(const TCHAR* format, ...);


    iUserNotify(int verboseLevel = 0);
    virtual ~iUserNotify();

protected:
    virtual void HandleNotify(int level, const TCHAR* format, va_list& args) = 0;
    // this is implemented in derived classes to implement the specific type of
    // notification desired

    int mVerboseLevel;

private:
    static iUserNotify* mpInstance;
};


//-----------------------------------------------------------------------------
// notify macros
//
// use these as an alternative to iUserNotify::GetInstance()->Notify( XXX )
// becuase the call will not be made if it is not the appropriate verbosity
// level. This is useful when you don't want the "..." part of Notify() to be
// evaluated (for example, if it is an expensive operation like cFCOName::AsString()
//
//-----------------------------------------------------------------------------

#define TW_NOTIFY_SILENT                                                        \
    if (iUserNotify::GetInstance()->GetVerboseLevel() >= iUserNotify::V_SILENT) \
    iUserNotify::GetInstance()->NotifySilent

#define TW_NOTIFY_NORMAL                                                        \
    if (iUserNotify::GetInstance()->GetVerboseLevel() >= iUserNotify::V_NORMAL) \
    iUserNotify::GetInstance()->NotifyNormal

#define TW_NOTIFY_VERBOSE                                                        \
    if (iUserNotify::GetInstance()->GetVerboseLevel() >= iUserNotify::V_VERBOSE) \
    iUserNotify::GetInstance()->NotifyVerbose

//#############################################################################
// inline implementation
//#############################################################################
inline iUserNotify* iUserNotify::GetInstance()
{
    ASSERT(mpInstance != 0);
    return mpInstance;
}

inline void iUserNotify::SetInstance(iUserNotify* pInst)
{
    mpInstance = pInst;
}


#endif //__USERNOTIFY_H
