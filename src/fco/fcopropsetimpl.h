//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
// fspropsetimpl.h -- file system objects' property set objects
//
// cPropSetImpl --  base class for all prop sets
//                  contains common implementation

#ifndef __FSPROPSETIMPL_H
#define __FSPROPSETIMPL_H

#ifndef __FCOPROPSET_H
#include "fco/fcopropset.h"
#endif
#ifndef __PROPSET_H
#include "fco/propset.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// cPropSetImpl
///////////////////////////////////////////////////////////////////////////////
class cPropSetImpl : public iFCOPropSet
{
public:
    cFSPropSet();
    cFSPropSet(const cFSPropSet& rhs);
    virtual ~cFSPropSet();

    const cFSPropSet& operator=(const cFSPropSet& rhs);

    virtual const cFCOPropVector&   GetValidVector()                const ;
    virtual int                     GetNumProps ()                  const ;
    virtual int                     GetPropIndex(const TCHAR* name) const;
    virtual        TSTRING          GetPropName (int index)         const;
    virtual const iFCOProp*         GetPropAt(int index)            const;
    virtual       iFCOProp*         GetPropAt(int index)            ;
    virtual void                    InvalidateProp(int index);
    virtual void                    InvalidateProps(const cFCOPropVector& propsToInvalidate) ;
    virtual void                    InvalidateAll();
    virtual void                    CopyProps(const iFCOPropSet* pSrc, const cFCOPropVector& propsToCopy);

private:
    cFCOPropVector      mValidProps;    // all the properties that have been evaluated
    cFCOPropVector      mUndefinedProps;    // properties that have been measured but have undefined values
};


#endif // #ifndef __FSPROPSETIMPL_H

