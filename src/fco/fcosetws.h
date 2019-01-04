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
// fcosetws.h
//
// cFCOSetWS -- derived from cFCOSetImpl, this set is associated with a spec and
//      for each insert, ASSERTs that the fco is contained in the spec.
//      ws stands for either With Spec, With Sanity checking, or Warm Salamander
#ifndef __FCOSETWS_H
#define __FCOSETWS_H

#ifndef __FCOSETIMPL_H
#include "fcosetimpl.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif
#ifndef __FCOSPEC_H
#include "fcospec.h"
#endif


class cFCOSetWS : public cFCOSetImpl
{
public:
    cFCOSetWS(const iFCOSpec* pSpec);
    // all FCOs that are inserted will be ASSERTed to be contained in this spec.
    // the class will AddRef() the spec, so the sanity of this class can be subverted
    // by changing the contents of the spec after this set is created
    cFCOSetWS(const cFCOSetWS& rhs);
    virtual ~cFCOSetWS();
    void operator=(const cFCOSetWS& rhs);

    void SetSpec(const iFCOSpec* pSpec);
    // this may only be called if the set is empty (useful if you want to construct a
    // set but you don't have the spec yet. The spec must be set before Insert() is called.

    virtual void Insert(iFCO* pFCO);

private:
    const iFCOSpec* mpSpec;
};

//#############################################################################
// inline implementation
inline cFCOSetWS::cFCOSetWS(const iFCOSpec* pSpec) : cFCOSetImpl(), mpSpec(pSpec)
{
    if (mpSpec)
        mpSpec->AddRef();
}
inline cFCOSetWS::cFCOSetWS(const cFCOSetWS& rhs) : cFCOSetImpl(rhs), mpSpec(rhs.mpSpec)
{
    if (mpSpec)
        mpSpec->AddRef();
}

inline cFCOSetWS::~cFCOSetWS()
{
    if (mpSpec)
        mpSpec->Release();
}

inline void cFCOSetWS::operator=(const cFCOSetWS& rhs)
{
    mpSpec = rhs.mpSpec;
    if (mpSpec)
        mpSpec->AddRef();
    cFCOSetImpl::operator=(rhs);
}

inline void cFCOSetWS::SetSpec(const iFCOSpec* pSpec)
{
    if (mpSpec)
        mpSpec->Release();
    mpSpec = pSpec;
    if (mpSpec)
        mpSpec->AddRef();
}

inline void cFCOSetWS::Insert(iFCO* pFCO)
{
    // here is the only real work this class does!
#ifdef DEBUG
    // TODO -- note that this doesn't do any checking if the spec is NULL. I
    // am not sure if this is the right thing to do or not.
    if (mpSpec)
    {
        ASSERT(mpSpec->SpecContainsFCO(pFCO->GetName()));

        if (!mpSpec->SpecContainsFCO(pFCO->GetName()))
        {
            cDebug d("cFCOSetWS::Insert");
            d.TraceError("*** Insert of FCO %s not appropriate for spec %s\n",
                         pFCO->GetName().AsString().c_str(),
                         mpSpec->GetName().c_str());
            d.TraceError("*** Spec contents:\n");
            mpSpec->TraceContents(cDebug::D_ERROR);
        }
    }
#endif // DEBUG

    cFCOSetImpl::Insert(pFCO);
}


#endif //__FCOSETWS_H
