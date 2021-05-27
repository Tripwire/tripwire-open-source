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
// fcopropvector.cpp
//

#include "stdfco.h"
#include "fcopropvector.h"
#include "core/serializer.h"
#include "core/debug.h"
#include "core/errorutil.h"

int cFCOPropVector::msBitlength(sizeof(uint32_t) * 8);
//msBitlength is common to all objects of class.

///////////////////////////////////////////////////////////////////////////////
// Constructor -- Sets mSize.  Default = 32.
///////////////////////////////////////////////////////////////////////////////
cFCOPropVector::cFCOPropVector(int size) : iSerializable()
{
    mSize = 32;
    mMask = 0;
    mpBuf = NULL;
    SetSize(size);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor --
///////////////////////////////////////////////////////////////////////////////
cFCOPropVector::~cFCOPropVector()
{
    if (mpBuf != NULL)
        delete mpBuf;
}

///////////////////////////////////////////////////////////////////////////////
// Copy Constructor
///////////////////////////////////////////////////////////////////////////////
cFCOPropVector::cFCOPropVector(const cFCOPropVector& rhs) : iSerializable()
{
    mSize = rhs.mSize;
    mMask = rhs.mMask;
    if (rhs.mpBuf != NULL)
    {
        mpBuf  = new std::vector<uint32_t>;
        *mpBuf = *(rhs.mpBuf);
    }
    else
        mpBuf = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//  Overloaded Operators: ==, !=, = , &, |, &=, |=
///////////////////////////////////////////////////////////////////////////////
bool cFCOPropVector::operator==(const cFCOPropVector& rhs) const
{
    if (this != &rhs)
    {
        if ((mpBuf == NULL) && (rhs.mpBuf == NULL))
            return (mMask == rhs.mMask);
        else if ((mpBuf == NULL) && (rhs.mpBuf != NULL))
        {
            if (rhs.isExtended())
                return false;
            else
                return (mMask == ((*rhs.mpBuf)[0]));
        }
        else if ((mpBuf != NULL) && (rhs.mpBuf == NULL))
        {
            if ((*this).isExtended())
                return false;
            else
                return (((*mpBuf)[0]) == rhs.mMask);
        }
        else
            return (*mpBuf == *(rhs.mpBuf));
    }
    else
        return true;
}

bool cFCOPropVector::operator!=(const cFCOPropVector& rhs) const
{
    return (!(*this == rhs));
}


cFCOPropVector& cFCOPropVector::operator=(const cFCOPropVector& rhs)
{
    if (this != &rhs)
    {
        if ((rhs.mpBuf != NULL) && (mpBuf != NULL))
            *mpBuf = *(rhs.mpBuf);
        else if ((rhs.mpBuf != NULL) && (mpBuf == NULL))
        {
            mpBuf  = new std::vector<uint32_t>;
            *mpBuf = *(rhs.mpBuf);
        }
        else if ((rhs.mpBuf == NULL) && (mpBuf != NULL))
        {
            delete mpBuf;
            mpBuf = NULL;
        }
        mSize = rhs.mSize;
        mMask = rhs.mMask;
    } //end if
    return *this;
}

cFCOPropVector cFCOPropVector::operator&(const cFCOPropVector& rhs) const
{
    cFCOPropVector temp = *this;
    temp &= rhs;
    return temp;
}

cFCOPropVector cFCOPropVector::operator|(const cFCOPropVector& rhs) const
{
    cFCOPropVector temp = *this;
    temp |= rhs;
    return temp;
}

cFCOPropVector cFCOPropVector::operator^(const cFCOPropVector& rhs) const
{
    cFCOPropVector temp = *this;
    temp ^= rhs;
    return temp;
}


cFCOPropVector& cFCOPropVector::operator&=(const cFCOPropVector& rhs)
{
    // make sure I am big enough
    if (GetSize() < rhs.GetSize())
        SetSize(rhs.GetSize());
    if (mpBuf == 0)
    {
        ASSERT(GetSize() <= 32);
        mMask &= rhs.mMask;
    }
    else if (rhs.mpBuf == 0)
    {
        ASSERT(rhs.GetSize() <= 32);
        (*mpBuf)[0] &= rhs.mMask;
    }
    else
    {
        for (unsigned int i = 0; i < rhs.mpBuf->size(); ++i)
            (*mpBuf)[i] &= (*rhs.mpBuf)[i];
    }
    return *this;
}

cFCOPropVector& cFCOPropVector::operator|=(const cFCOPropVector& rhs)
{
    // make sure I am big enough
    if (GetSize() < rhs.GetSize())
        SetSize(rhs.GetSize());
    if (mpBuf == 0)
    {
        ASSERT(GetSize() <= 32);
        mMask |= rhs.mMask;
    }
    else if (rhs.mpBuf == 0)
    {
        ASSERT(rhs.GetSize() <= 32);
        (*mpBuf)[0] |= rhs.mMask;
    }
    else
    {
        for (unsigned int i = 0; i < rhs.mpBuf->size(); ++i)
            (*mpBuf)[i] |= (*rhs.mpBuf)[i];
    }
    return *this;
}

cFCOPropVector& cFCOPropVector::operator^=(const cFCOPropVector& rhs)
{
    // make sure I am big enough
    if (GetSize() < rhs.GetSize())
        SetSize(rhs.GetSize());
    if (mpBuf == 0)
    {
        ASSERT(GetSize() <= 32);
        mMask ^= rhs.mMask;
    }
    else if (rhs.mpBuf == 0)
    {
        ASSERT(rhs.GetSize() <= 32);
        (*mpBuf)[0] ^= rhs.mMask;
    }
    else
    {
        for (unsigned int i = 0; i < rhs.mpBuf->size(); ++i)
            (*mpBuf)[i] ^= (*rhs.mpBuf)[i];
    }
    return *this;
}


//END OPERATORS

///////////////////////////////////////////////////////////////////////////////
// GetSize -- returns the number of items the vector can hold
///////////////////////////////////////////////////////////////////////////////
int cFCOPropVector::GetSize(void) const
{
    return mSize;
}

///////////////////////////////////////////////////////////////////////////////
// SetSize -- Sets the maximum NUMBER of items that can be stored
// in vector.  **IMPORTANT** : If max is less than current size of mask+buff,
// the set is truncated and extra information is lost.
///////////////////////////////////////////////////////////////////////////////
int cFCOPropVector::SetSize(int max)
{
    if ((max <= msBitlength) && (mpBuf == NULL))
        return mSize = msBitlength;
    else if ((max <= msBitlength) && (mpBuf != NULL))
    {
        return mSize = (*mpBuf).capacity() * msBitlength;
        //new operation already performed, stick with mpBuf.
    }
    else if ((mpBuf == NULL) && (max > msBitlength))
    {
        mpBuf = new std::vector<uint32_t>;
        (*mpBuf).resize(((max / msBitlength) + 1), 0);
        (*mpBuf)[0]  = mMask;
        return mSize = ((*mpBuf).capacity() * msBitlength);
    }
    else //mpBuf!=NULL && max>msBitlength
    {
        if (mpBuf)
        {
            (*mpBuf).resize(((max / msBitlength) + 1), 0);
            mSize = ((*mpBuf).capacity() * msBitlength);
        }
        return mSize;
    }
}

///////////////////////////////////////////////////////////////////////////////
// AddItem -- Adds an item to the bitset by 'anding' it on. Behavior is
// undefined if i >= GetSize. Returns true if set contains item after change.
///////////////////////////////////////////////////////////////////////////////
bool cFCOPropVector::AddItem(int i)
{
    ASSERT(i < GetSize());

    if (mpBuf == NULL)
    {
        ASSERT(mSize <= 32);
        mMask |= 1 << (i);
    }
    else
    {
        ((*mpBuf)[(i) / msBitlength]) |= 1 << ((i) % msBitlength);
    }
    return ContainsItem(i);
}


///////////////////////////////////////////////////////////////////////////////
// AddItemAndGrow -- Like AddItem except that if i >= GetSize, resizes vector.
// Returns true if set contains item after change.
///////////////////////////////////////////////////////////////////////////////
bool cFCOPropVector::AddItemAndGrow(int i)
{
    if (i >= GetSize())
        SetSize(i);

    return AddItem(i);
}

///////////////////////////////////////////////////////////////////////////////
// RemoveItem -- Removes an item from the vector.  Vector is unchanged if
// i >= GetMaxItem.  Returns true if set does not contain item after change.
///////////////////////////////////////////////////////////////////////////////
bool cFCOPropVector::RemoveItem(int i)
{
    ASSERT(i < GetSize());

    if (!ContainsItem(i))
        return true;
    if (mpBuf == NULL)
        mMask ^= 1 << (i);
    else
    {
        ((*mpBuf)[(i) / msBitlength]) ^= 1 << ((i) % msBitlength);
    }
    return !ContainsItem(i);
}

///////////////////////////////////////////////////////////////////////////////
// ContainsItem -- Returns boolean value if bit value is in the vector.
// Fails if mMask+mBuf is < i.
///////////////////////////////////////////////////////////////////////////////
bool cFCOPropVector::ContainsItem(int i) const
{
    if (i > mSize - 1)
        return false;
    else if (mpBuf == NULL)
    {
        return ((mMask & (1 << (i))) != 0);
    }
    else
    {
        return ((((*mpBuf)[(i) / msBitlength]) & (1 << ((i) % msBitlength))) != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Clear -- removes all items from the vector
///////////////////////////////////////////////////////////////////////////////
void cFCOPropVector::Clear(void)
{
    mMask = 0;
    if (mpBuf)
    {
        std::fill(mpBuf->begin(), mpBuf->end(), 0);
    }
}


///////////////////////////////////////////////////////////////////////////////
// isExtended -- Returns true if Vector is USING mpBuf beyond [0].  It
// is assumed that {mpBuf [i] |i>0} is being used if i has non-zero value.
///////////////////////////////////////////////////////////////////////////////
bool cFCOPropVector::isExtended(void) const
{
    if (mpBuf == NULL)
        return false;
    else
    {
        if ((*mpBuf).size() <= 1)
            return false;
        else
        {
            uint32_t sum = 0;
            for (uint32_t i = (*mpBuf).size() - 1; i >= 2; i--)
                sum += ((*mpBuf)[i]);
            return (sum != 0);
        }
    }
}


// TO DO: temp function, will delete after testing... DA
void cFCOPropVector::check(cDebug& d) const
{
    d.TraceDetail("mSize = %i \n", mSize);
    d.TraceDetail("msBitlength = %i \n", msBitlength);
    d.TraceDetail("mMask = %u \n", mMask);
    if (mpBuf != NULL)
    {
        d.TraceDetail("*mpBuf has size %i \n", (*mpBuf).size());
        for (unsigned int i = 0; i < (*mpBuf).size(); ++i)
            d.TraceDetail("mpBuf element %i = %u \n", i, (*mpBuf)[i]);
    }
} //end check


void cFCOPropVector::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > 0)
        ThrowAndAssert(eSerializerVersionMismatch(_T("Property Vector Read")));

    int32_t newSize;
    pSerializer->ReadInt32(newSize);
    ASSERT(newSize > 0);

    SetSize(newSize);

    if (mpBuf == NULL)
    {
        int32_t mask;
        pSerializer->ReadInt32(mask);
        mMask = mask;
    }
    else
    {
        for (int i = 0; i <= mSize / msBitlength; ++i)
        {
            int32_t mask;
            pSerializer->ReadInt32(mask);
            (*mpBuf)[i] = mask;
        }
    }
}

void cFCOPropVector::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(mSize);

    if (mpBuf == NULL)
    {
        pSerializer->WriteInt32(mMask);
    }
    else
    {
        for (int i = 0; i <= mSize / msBitlength; ++i)
            pSerializer->WriteInt32((*mpBuf)[i]);
    }
}


///////////////////////////////////////////////////////////////////////////////
// TraceContents -- prints the contents of the vector to debug out
///////////////////////////////////////////////////////////////////////////////
void cFCOPropVector::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug         d("cFCOPropVector::TraceContents");
    TOSTRINGSTREAM ostr;
    for (int i = 0; i < GetSize(); i++)
    {
        if (ContainsItem(i))
            ostr << i << _T(", ");
    }

    tss_mkstr(contents, ostr);
    d.Trace(dl, _T("Size = %d Contents = %s\n"), GetSize(), contents.c_str());
}
