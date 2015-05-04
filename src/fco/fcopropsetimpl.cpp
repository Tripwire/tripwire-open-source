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
// propsetimpl.cpp

#include "stdfs.h"
#include "propsetimpl.h"

#include "core/debug.h"
#include "core/serializer.h"
#include "core/errorutil.h"
#include "fco/fcoundefprop.h"

 ///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cPropSetImpl::TraceContents(int dl) const
{
	if(dl < 0) 
		dl = cDebug::D_DEBUG;

	cDebug d("cPropSetImpl::TraceContents");

	TOSTRINGSTREAM ostr;
	ostr << _T("File Sysytem Prop Set: ");
	for(int i=0; i<GetNumProps(); i++)
	{
		if(mValidProps.ContainsItem(i))
		{
			ostr << _T("[") << i << _T("]") << GetPropName(i) << _T(" = ") << GetPropAt(i)->AsString().c_str() << _T(", ");
		}
	}
	d.Trace(dl, _T("%s\n"), ostr.str().c_str());

}

///////////////////////////////////////////////////////////////////////////////
// CopyProps
///////////////////////////////////////////////////////////////////////////////
void cPropSetImpl::CopyProps(const iFCOPropSet* pSrc, const cFCOPropVector& propsToCopy)
{
	ASSERT(pSrc->GetType() == CLASS_TYPE(cPropSetImpl));

	// first, modify my valid vector...
	mValidProps |= propsToCopy;

	for(int i=0; i < GetNumFSProps(); i++)
	{
		if(propsToCopy.ContainsItem(i))
		{
            ASSERT( pSrc->GetValidVector().ContainsItem( i ) );
            const iFCOProp* pProp = pSrc->GetPropAt(i);

            if (pProp->GetType() != cFCOUndefinedProp::GetInstance()->GetType())
            {
			    GetPropAt(i)->Copy( pProp );
                mUndefinedProps.RemoveItem(i);
            }
            else
                mUndefinedProps.AddItem(i);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Invalidate
///////////////////////////////////////////////////////////////////////////////
void cPropSetImpl::InvalidateProp(int index)
{
	ASSERT((index >= 0) && (index < GetNumProps()));
	mValidProps.RemoveItem(index);
}

void cPropSetImpl::InvalidateAll()
{
	mValidProps.Clear();
}

void cPropSetImpl::InvalidateProps(const cFCOPropVector& propsToInvalidate) 
{
	cFCOPropVector	inBoth	=  mValidProps;
	inBoth					&= propsToInvalidate;
	mValidProps				^= inBoth;
}

///////////////////////////////////////////////////////////////////////////////
// GetNumFSProps
///////////////////////////////////////////////////////////////////////////////
int cPropSetImpl::GetNumFSProps()
{
	return PROP_NUMITEMS;
}

///////////////////////////////////////////////////////////////////////////////
// ctors. dtor, operator=
///////////////////////////////////////////////////////////////////////////////
cPropSetImpl::cPropSetImpl() :
	mValidProps(cPropSetImpl::PROP_NUMITEMS),
    mUndefinedProps(cPropSetImpl::PROP_NUMITEMS)
{
	// TODO: do I want to zero out all the property values here?
}

cPropSetImpl::~cPropSetImpl()
{

}

cPropSetImpl::cPropSetImpl(const cPropSetImpl& rhs) :
	mValidProps(cPropSetImpl::PROP_NUMITEMS)
{
	*this = rhs;
}

const cPropSetImpl& cPropSetImpl::operator=(const cPropSetImpl& rhs)
{
	mValidProps = rhs.GetValidVector();
    mUndefinedProps = rhs.mUndefinedProps;

    for(int i=0; i < PROP_NUMITEMS; i++)
        if (mValidProps.ContainsItem(i) && !mUndefinedProps.ContainsItem(i))
	        GetPropAt(i)->Copy( ((cPropSetImpl&)rhs).GetPropAt(i) ); // call non-const GetPropAt for rhs
                                                                   // don't want it to assert ContainsItem
	return *this;
}

const cFCOPropVector& cPropSetImpl::GetValidVector() const 
{
	return mValidProps;
}

int cPropSetImpl::GetNumProps() const 
{
	return PROP_NUMITEMS;
}

/*
int cPropSetImpl::GetPropIndex(const TCHAR* name) const
{
	for(int i=0; i<PROP_NUMITEMS; i++)
	{
		if( _tcscmp( name, TSS_GetString( cFS, cPropSetImpl_PropNames[i]).c_str() ) == 0 )
			return i;
	}
	return iFCOPropSet::PROP_NOT_FOUND;
}

TSTRING cPropSetImpl::GetPropName(int index) const
{
	ASSERT((index >= 0) && (index < GetNumProps()));
	return TSS_GetString( cFS, cPropSetImpl_PropNames[index]);
}

  */
void cPropSetImpl::Read(iSerializer* pSerializer, int32 version)
{
	if (version > Version())
		ThrowAndAssert(eSerializerVersionMismatch(_T("Property Set Read")));

	mValidProps.Read(pSerializer);
    mUndefinedProps.Read(pSerializer);

    for (int i=0; i < PROP_NUMITEMS; i++)
    {
		if (mValidProps.ContainsItem(i) && !mUndefinedProps.ContainsItem(i))
			GetPropAt(i)->Read(pSerializer);
    }
}

void cPropSetImpl::Write(iSerializer* pSerializer) const
{
   mValidProps.Write(pSerializer);
   mUndefinedProps.Write(pSerializer);
 
   for (int i=0; i < PROP_NUMITEMS; i++)
   {
		if (mValidProps.ContainsItem(i) && !mUndefinedProps.ContainsItem(i))
			GetPropAt(i)->Write(pSerializer);
   }
}


