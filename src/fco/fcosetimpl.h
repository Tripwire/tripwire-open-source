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
// fcosetimpl.h
//
// class cFCOSetImpl        -- concrete implementation of an FCO set
// class cFCOIterImpl       -- the iterator over the set
#ifndef __FCOSETIMPL_H
#define __FCOSETIMPL_H

#ifndef __FCO_H
#include "fco.h"
#endif
#include "fconame.h"

#ifndef __FCONAME_H
#include "fconame.h"
#endif
class cFCOIterImpl;

class cFCOSetImpl : public iFCOSet
{
    friend class cFCOIterImpl;
    DECLARE_TYPEDSERIALIZABLE()

public:
    cFCOSetImpl();
    cFCOSetImpl(const cFCOSetImpl& rhs);
    virtual ~cFCOSetImpl();
    cFCOSetImpl& operator=(const cFCOSetImpl& rhs);

    virtual const iFCOIter* Lookup(const cFCOName& name) const;
    virtual iFCOIter*       Lookup(const cFCOName& name);
    virtual const iFCOIter* GetIter() const;
    virtual iFCOIter*       GetIter();
    virtual void            Insert(iFCO* pFCO);
    virtual void            Clear();
    virtual bool            IsEmpty() const;
    virtual int             Size() const
    {
        return mFCOSet.size();
    };
    virtual void TraceContents(int dl = -1) const;

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
private:
    void ReturnIter(const cFCOIterImpl* pIter) const;
    // returns the iterator to its owner; the reciprocal action
    // to Lookup() or GetIter(); called by the iterator when it is destroyed

    // class we store in the set below; it is a hack that allows us to
    // look up iFCOs using cFCONames in a std::set
    class cFCONode
    {
    public:
        iFCO*           mpFCO;
        const cFCOName* mpFCOName;

        cFCONode() : mpFCO(0), mpFCOName(0)
        {
        }
        //TODO: make the iFCO* constructor explicit
        cFCONode(iFCO* pFCO) : mpFCO(pFCO), mpFCOName(&pFCO->GetName())
        {
        }
        explicit cFCONode(const cFCOName& name) : mpFCO(0), mpFCOName(&name)
        {
        }
        cFCONode(const cFCONode& rhs) : mpFCO(rhs.mpFCO), mpFCOName(rhs.mpFCOName)
        {
        }
        bool operator<(const cFCONode& rhs) const
        {
            if (mpFCOName)
                return (*mpFCOName < *rhs.mpFCOName);
            else
                return false;
        }
        bool operator==(const cFCONode& rhs) const
        {
            if (mpFCOName)
                return (*mpFCOName == *rhs.mpFCOName);
            else
                return false;
        }
    };

    std::set<cFCONode> mFCOSet;
    // this is what actually stores the iFCOs.
};

class cFCOIterImpl : public iFCOIter
{
    friend class cFCOSetImpl;

public:
    explicit cFCOIterImpl(cFCOSetImpl* pSet);
    explicit cFCOIterImpl(const cFCOSetImpl* pSet);

    virtual void        SeekBegin() const;
    virtual bool        Done() const;
    virtual bool        IsEmpty() const;
    virtual void        Next() const;
    virtual const iFCO* FCO() const;
    virtual iFCO*       FCO();
    virtual bool        SeekToFCO(const cFCOName& name) const;

    virtual void Remove();
    virtual void Remove() const;
    virtual void DestroyIter() const;

private:
    virtual ~cFCOIterImpl();

    cFCOSetImpl*                                      mpSet;
    mutable std::set<cFCOSetImpl::cFCONode>::iterator mIter;
    // the definition of a const iterator is not that its position cannot change,
    // but that it cannot modify the set it is iterating over, hence the "mutable"
};


#endif //__FCOSETIMPL_H
