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
// fcospecattr.h
//
// cFCOSpecAttr -- class that holds integrity-check related fco spec attributes
#ifndef __FCOSPECATTR_H
#define __FCOSPECATTR_H

#ifndef __SREFCOUNTOBJ_H
#include "core/srefcountobj.h"
#endif
#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __TYPES_H
#include "types.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif
///////////////////////////////////////////////////////////////////////////////
// cFCOSpecAttr
///////////////////////////////////////////////////////////////////////////////
class cFCOSpecAttr : public iSerRefCountObj
{
public:
    cFCOSpecAttr();
    virtual ~cFCOSpecAttr();

    ///////////////////////
    // the attributes
    ///////////////////////
    const TSTRING& GetName() const;
    void           SetName(const TSTRING& name);

    int32_t GetSeverity() const;
    void    SetSeverity(int32_t s);

    int  GetNumEmail() const;
    void AddEmail(const TSTRING& str);
    // adds an email address for report notification. This class makes no attempt
    // to catch and prune identical entries in the email list.

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    void TraceContents(int dl = -1) const;

    bool operator==(const cFCOSpecAttr& rhs) const;

    DECLARE_SERREFCOUNT()
private:
    cFCOSpecAttr(const cFCOSpecAttr& rhs);   // not impl
    void operator=(const cFCOSpecAttr& rhs); // not impl

    std::list<TSTRING> mEmailAddrs; // the email addresses of people to be notified
    TSTRING            mName;       // the name of the spec
    int32_t            mSeverity;   // the severity level

    friend class cFCOSpecAttrEmailIter;
};

///////////////////////////////////////////////////////////////////////////////
// cFCOSpecAttrEmailIter
///////////////////////////////////////////////////////////////////////////////
class cFCOSpecAttrEmailIter
{
public:
    explicit cFCOSpecAttrEmailIter(const cFCOSpecAttr& attr);
    ~cFCOSpecAttrEmailIter();

    void SeekBegin() const;
    bool Done() const;
    bool IsEmpty() const;
    void Next() const;

    const TSTRING& EmailAddress() const;

private:
    const std::list<TSTRING>&                  mSet;
    mutable std::list<TSTRING>::const_iterator mIter;
};


//#############################################################################
// inline implementation
inline cFCOSpecAttr::cFCOSpecAttr() : mName(_T("Unnamed")), mSeverity(0)
{
}
inline cFCOSpecAttr::~cFCOSpecAttr()
{
}
inline const TSTRING& cFCOSpecAttr::GetName() const
{
    return mName;
}
inline void cFCOSpecAttr::SetName(const TSTRING& name)
{
    mName = name;
}
inline int32_t cFCOSpecAttr::GetSeverity() const
{
    return mSeverity;
}
inline void cFCOSpecAttr::SetSeverity(int32_t s)
{
    mSeverity = s;
}

inline void cFCOSpecAttr::AddEmail(const TSTRING& str)
{
    mEmailAddrs.push_back(str);
}
inline int cFCOSpecAttr::GetNumEmail() const
{
    return mEmailAddrs.size();
}

///////////////
// iterator
///////////////
inline cFCOSpecAttrEmailIter::cFCOSpecAttrEmailIter(const cFCOSpecAttr& attr) : mSet(attr.mEmailAddrs)
{
    mIter = mSet.begin();
}

inline cFCOSpecAttrEmailIter::~cFCOSpecAttrEmailIter()
{
}
inline void cFCOSpecAttrEmailIter::SeekBegin() const
{
    mIter = mSet.begin();
}
inline bool cFCOSpecAttrEmailIter::Done() const
{
    return (mIter == mSet.end());
}
inline bool cFCOSpecAttrEmailIter::IsEmpty() const
{
    return mSet.empty();
}
inline void cFCOSpecAttrEmailIter::Next() const
{
    ++mIter;
}
inline const TSTRING& cFCOSpecAttrEmailIter::EmailAddress() const
{
    ASSERT(!Done());
    return *mIter;
}


#endif //__FCOSPECATTR_H
