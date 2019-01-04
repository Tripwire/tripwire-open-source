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
// fsdatasourceiter.h
//
#ifndef __FSDATASOURCEITER_H
#define __FSDATASOURCEITER_H

//=========================================================================
// INCLUDES
//=========================================================================
#include "fco/fcodatasourceiterimpl.h"
#include "core/fileerror.h"
#include "core/fsservices.h"

TSS_FILE_EXCEPTION(eFSDataSourceIter, eFileError)
TSS_FILE_EXCEPTION(eFSDataSourceIterReadDir, eFSDataSourceIter)


//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

//-----------------------------------------------------------------------------
// *** Important Note: Not crossing file system boundaries works by noting the
//      device number of the object created by SeekToFCO( XXX, false ), since it
//      is assumed the only time you will pass false is when you are seeking to
//      the start point of a spec.
//          -- 20 Jan 99 mdb
//-----------------------------------------------------------------------------
class cFSDataSourceIter : public cFCODataSourceIterImpl
{
public:
    cFSDataSourceIter();
    cFSDataSourceIter(const cFSDataSourceIter& rhs);
    virtual ~cFSDataSourceIter();

    cFSDataSourceIter& operator=(const cFSDataSourceIter& rhs);

    virtual iFCODataSourceIter* CreateCopy() const;

    static void SetFileSystemCrossing(bool crossFS);
    // Call this to set the property where cFSDataSourceIter does not automatically recurse
    // across file system boundaries.  Currently this is by default is set to false.

    //void TraceContents(int dl = -1) const;
private:
    uint64_t mDev; // the device number of the last node reached through SeekTo()
                 // if this is zero, then it is assumed to be uninitialized

    //-------------------------------------------------------------------------
    // helper methods
    //-------------------------------------------------------------------------

    virtual void GetChildrenNames(const TSTRING& strParentName, std::vector<TSTRING>& vChildrenNames);

    virtual iFCO* CreateObject(const cFCOName& name, bool bCreatingPeers);
    virtual bool  InitializeTypeInfo(iFCO* pFCO);

    void AddIterationError(const eError& e);
    bool DoStat(const TSTRING& name, cFSStatArgs& statArgs);
};

#endif //__FSDATASOURCEITER_H
