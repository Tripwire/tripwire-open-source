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
// twlimits.h
//
// this file contains classes that contain informations regarding
// "limits" that need to be shared between files.  The classes should only
// contain minimal functionality in support of these limits.
//
//

#ifndef __TWLIMITS_H
#define __TWLIMITS_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================


///////////////////////////////////////////////
// cInterpretInt
///////////////////////////////////////////////
//
// override GetMin() and GetMax() to have different
// limits
//
///////////////////////////////////////////////
class cInterpretInt
{
public:
    //
    // basic functionality
    //
    bool InterpretInt(const TSTRING& str, int* pi);
    // converts the string into an int value, returned in *pi.
    // returns true if successfully converted value.
    // *pi is undefined if function returns false.

private:
    //
    // private enums
    //
    enum Limits
    {
        LIMIT_MIN = INT_MIN,
        LIMIT_MAX = INT_MAX
    };

    //
    // private functionality
    //
    virtual int GetMax()
    {
        return LIMIT_MAX;
    };
    virtual int GetMin()
    {
        return LIMIT_MIN;
    };

public:
    virtual ~cInterpretInt()
    {
    }
};

///////////////////////////////////////////////
// cSeverityLimits
///////////////////////////////////////////////
class cSeverityLimits : public cInterpretInt
{
private:
    enum Limits
    {
        LIMIT_MIN = 0,
        LIMIT_MAX = 1000000
    };

    virtual int GetMax()
    {
        return LIMIT_MAX;
    };
    virtual int GetMin()
    {
        return LIMIT_MIN;
    };

public:
    virtual ~cSeverityLimits()
    {
    }
};

///////////////////////////////////////////////
// cRecurseDepthLimits
///////////////////////////////////////////////
class cRecurseDepthLimits : public cInterpretInt
{
private:
    enum Limits
    {
        LIMIT_MIN = -1,
        LIMIT_MAX = INT_MAX
    };

    virtual int GetMax()
    {
        return LIMIT_MAX;
    };
    virtual int GetMin()
    {
        return LIMIT_MIN;
    };

public:
    virtual ~cRecurseDepthLimits()
    {
    }
};

#endif //__TWLIMITS_H
