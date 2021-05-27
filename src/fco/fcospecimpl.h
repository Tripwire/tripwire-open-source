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
// fcospecimpl.h
//
// class cFCOSpecImpl -- an implementation of iFCOSpec for file systems
#ifndef __FCOSPECIMPL_H
#define __FCOSPECIMPL_H

#ifndef __FCOSPEC_H
#include "fcospec.h"
#endif
#ifndef __FCOPROPVECTOR_H
#include "fcopropvector.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif

#include <set>

class cErrorBucket;
class iFCOSpecHelper;
///////////////////////////////////////////////////////////////////////
// NOTE: it is interesting that there is nothing about this class that
//      is specific to files; it could potentially be used with other FCOs,
//      so maybe its name should be changed to cFCOSpecImpl or something
//      like that -- mdb
///////////////////////////////////////////////////////////////////////
class cFCOSpecImpl : public iFCOSpec
{
    DECLARE_SERREFCOUNT()
public:
    // TODO -- remove the iFCODataSource from the ctor; I am doing nothing with it right now and
    //      didn't change it simply because I don't want to touch 1000 files updateing the creation
    //      parameters... 28 jan mdb
    //
    cFCOSpecImpl(const TSTRING& name, /* const iFCODataSource* */ void* pSrc = 0, iFCOSpecHelper* pHelper = NULL);
    // the spec will delegate fco creation to pSrc as needed. It is OK to pass NULL as the data
    // source, but it must be set before this spec can be used.
    // the spec will delete whatever helper it contains when it is destroyed
    cFCOSpecImpl(const cFCOSpecImpl& rhs);
    cFCOSpecImpl();
    cFCOSpecImpl& operator=(const cFCOSpecImpl& rhs);

    // from iFCOSpec
    virtual bool                  SpecContainsFCO(const cFCOName& name) const;
    virtual const TSTRING&        GetName() const;
    virtual void                  SetName(const TSTRING& name);
    virtual iFCOSpec*             Clone() const;
    virtual const cFCOName&       GetStartPoint() const;
    virtual void                  SetStartPoint(const cFCOName& name); // throw (eError);
    virtual const cFCOPropVector& GetPropVector(const iFCOSpecMask* pFCOSpecMask) const;
    virtual bool                  SetPropVector(const iFCOSpecMask* pMask, const cFCOPropVector& vector);
    virtual const iFCOSpecMask*   GetSpecMask(const iFCO* pFCO) const;
    virtual void                  TraceContents(int dl = -1) const;
    virtual bool                  ShouldStopDescent(const cFCOName& name) const;

    virtual void                  SetHelper(iFCOSpecHelper* pHelper);
    virtual const iFCOSpecHelper* GetHelper() const;
    // helper manipulation; note that the spec will delete whatever helper it contains when it is destroyed,
    // but it will not delete the current helper when SetHelper() is called.

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)


    virtual ~cFCOSpecImpl();
    // only destroy with Release(). Don't create on the stack.
private:
    TSTRING         mName;
    cFCOPropVector  mPropVector;
    iFCOSpecHelper* mpHelper;
};

//#############################################################################
// inline impl
//#############################################################################
inline void cFCOSpecImpl::SetHelper(iFCOSpecHelper* pHelper)
{
    mpHelper = pHelper;
}

inline const iFCOSpecHelper* cFCOSpecImpl::GetHelper() const
{
    ASSERT(mpHelper != 0);
    return mpHelper;
}


#endif //__FCOSPECIMPL_H
