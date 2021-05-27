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
// iterproxy.h
//
// cIterProxy<> -- smart pointer for iterator classes used in tripwire. the
//      iterator must support being destroyed with a member function called
//      ReleaseIter()
#ifndef __ITERPROXY_H
#define __ITERPROXY_H

////////////////////////////////////////////////////
// cIterProxy -- a smart pointer class
//      for fco iterators. It can be dereferenced
//      and assigned to like a pointer, but it calls
//      mpIter->Release() when it is destroyed, unless
//      mpIter == NULL.
////////////////////////////////////////////////////

template<class TYPE> class cIterProxy
{
public:
    //TODO: Can these 2 constructors be made explicit?
    cIterProxy(TYPE* pIter = NULL) : mpIter(pIter){};
    cIterProxy(const TYPE* pIter) : mpIter((TYPE*)pIter){};
    ~cIterProxy()
    {
        if (mpIter)
            mpIter->DestroyIter();
    }
    operator TYPE*()
    {
        return mpIter;
    }
    operator const TYPE*() const
    {
        return mpIter;
    }
    TYPE* operator->()
    {
        return mpIter;
    }
    const TYPE* operator->() const
    {
        return mpIter;
    }
    TYPE* operator=(TYPE* rhs)
    {
        mpIter = rhs;
        return mpIter;
    }

private:
    TYPE* mpIter;
};


#endif
