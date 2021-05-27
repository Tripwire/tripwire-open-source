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
// fcoundefprop.h -- the undefined FCO property
//
#ifndef __FCOUNDEFPROP_H
#define __FCOUNDEFPROP_H

#ifndef __FCOPROP_H
#include "fcoprop.h"
#endif

//=============================================================================
// cFCOUndefinedProp
//
// This class exists only as a singleton.  iFCOPropSet derived classes will return a
// pointer to the singleton for properties that are undefined.
//=============================================================================

class cFCOUndefinedProp : public iFCOProp
{
public:
    DECLARE_TYPEDSERIALIZABLE() // type information

    static cFCOUndefinedProp* GetInstance();
    // access to the singleton

    // from iFCOProp
    virtual TSTRING   AsString() const;
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;
    virtual void      Copy(const iFCOProp* rhs);

private:
    friend struct cFCOUndefinedPropProxy;

    cFCOUndefinedProp();
    virtual ~cFCOUndefinedProp();
    // don't new or construct these on the stack

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eInternal)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eInternal)
        // These functions should never be called, and they will throw and eInternal if they are.
};


#endif // __FCOUNDEFPROP_H
