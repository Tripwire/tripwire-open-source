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
// fsobject.h -- FCO-derived classes for file systems
//
// cFSObject -- base class for files and directories
//
#ifndef __FSOBJECT_H
#define __FSOBJECT_H

#ifndef __FCO_H
#include "fco/fco.h"
#endif

#ifndef __FSPROPSET_H
#include "fspropset.h"
#endif

#ifndef __FCOSETIMPL_H
#include "fco/fcosetimpl.h"
#endif

#ifndef __FCONAME_H
#include "fco/fconame.h"
#endif

///////////////////////////////////////////////////////////////////
// cFSObject -- base class for files and directory FCOs
class cFSObject : public iFCO
{
    DECLARE_SERREFCOUNT()

public:
    explicit cFSObject(const cFCOName& name);

    virtual void            SetName(const cFCOName& name);
    virtual const cFCOName& GetName() const;

    virtual const iFCOPropSet* GetPropSet() const;
    virtual iFCOPropSet*       GetPropSet();
    virtual uint32_t           GetCaps() const;

    virtual iFCO* Clone() const;
    virtual void  AcceptVisitor(iFCOVisitor* pVisitor);

    const cFSPropSet& GetFSPropSet() const;
    cFSPropSet&       GetFSPropSet();
    // returns a reference to the FS property set

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    virtual void TraceContents(int dl = -1) const;

#ifdef DEBUG
    static void TraceStats();
    // this TRACEs statistics on FSObject usage that is pertinent to performance or mem leakage
    // concerns.
#endif

    cFSObject();
    virtual ~cFSObject();
    // only destroy by calling Release(); Also prevents creation on the
    // stack.

private:
    cFSPropSet mPropSet;
    cFCOName   mName;
};

//////////////////////////////////////////////////////
// inline implementation
//////////////////////////////////////////////////////
inline const cFSPropSet& cFSObject::GetFSPropSet() const
{
    return mPropSet;
}

inline cFSPropSet& cFSObject::GetFSPropSet()
{
    return mPropSet;
}


#endif
