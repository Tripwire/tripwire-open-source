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
// fcodatasource.h
//
// iFCODataSource -- interface to an object that can create FCOs given a cFCOName
#ifndef __FCODATASOURCE_H
#define __FCODATASOURCE_H

class iFCO;
class cFCOName;
class cFCOErrorQueue;

class iFCODataSource
{
public:
    enum
    {
        CHILDREN_NONE = 0,
        CHILDREN_ALL  = -1
    };

    virtual ~iFCODataSource() = 0;

    // NOTE -- in the act of creating fcos, there are certain properties that we can obtain "for free" just
    //      by the act of querying the system we are generating fcos for (for example, a stat() call is required
    //      to create file system fcos, which also gives us many of the fsfco's property values). Therefore,
    //      as an optimization, fcos returned from this interface might have some properties filled out. If that
    //      is not desireable, you should call iFCO::GetPropSet()->Clear() to invalidate all the properties.

    virtual iFCO* CreateFCO(const cFCOName& name, int recurseDepth, cFCOErrorQueue* pQueue = 0) const = 0;
    // creates the FCO specified by "name", returns NULL if the object doesn't exist
    // on the system the data source is bound to. recurseDepth is how deep the child
    // tree should be built; 0 means get no children, and -1 means all the children
    // All errors that occur will be reported to the passed in error bucket; it is ok to pass
    // NULL if you don't care about errors.

    virtual bool GetChildren(iFCO* pFCO, int recurseDepth, cFCOErrorQueue* pQueue = 0) const = 0;
    // given a pointer to an FCO, this will populate it with children for as many levels
    // as specified by recursedepth. The passed in FCO should have no children; it is
    // undefined what will happen to existing children after this function returns.
    // All errors that occur will be reported to the passed in error bucket; it is ok to pass
    // NULL if you don't care about errors.

    // cErrorBucket error numbers...
    // the data source owns all error numbers from 100-199
    enum ErrorNum
    {
        ERR_READDIR_FAIL     = 100,
        ERR_GETFILETYPE_FAIL = 101,
        ERR_NUM_ERRORS
    };
};

#endif
