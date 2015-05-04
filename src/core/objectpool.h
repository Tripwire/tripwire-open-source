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
// objectpool.h
//
// cBlockList		-- a linked list of blocks of memory
// cObjectPoolBase	-- a pool of equal-sized objects; constant time allocation
#ifndef __OBJECTPOOL_H
#define __OBJECTPOOL_H

class cBlockLink;

//-----------------------------------------------------------------------------
// cBlockList
//-----------------------------------------------------------------------------
class cBlockList
{
public:
	cBlockList();
	~cBlockList();

	void*	Allocate(int size);
	void	Clear();			// releases everything in the block list
private:
	cBlockLink* mpBlocks;		// linked list of blocks
};

//-----------------------------------------------------------------------------
// cObjectPoolBase
//-----------------------------------------------------------------------------
class cObjectPoolBase
{
public:
	cObjectPoolBase(int objSize, int chunkSize);
	~cObjectPoolBase();
		// NOTE -- dtor is not virtual; therefore it is potentially dangerous to delete a pointer to
		// this class unless you know for sure the dynamic class type has nothing to clean up.

	void*	Alloc	();
	void	Free	(void* pObj);
	void	Clear	();				
	//TODO -- add IsPointerValid() 
private:
	void	AllocNewChunk();	// get another chunk to use...

	struct cLink 
	{
		cLink* mpNext;
	};
	
	cBlockList	mBlockList;
	const int	mObjectSize;	// how big are the objects?
	const int	mChunkSize;		// how big are the chunks we are allocating?
	cLink*		mpNextFree;		// the next free object
	//int mInstanceCount;		// number of objects that are currently allocated but not freed.
};

//-----------------------------------------------------------------------------
// cObjectPool
// Note -- this template only works for classes that are constructed with their
//		default ctor; I don't know how to extend this model to work for non-default 
//		ctors...
//-----------------------------------------------------------------------------
template <class T>
class cObjectPool : public cObjectPoolBase
{
public:
	cObjectPool(int chunkSize) : cObjectPoolBase(sizeof(T), chunkSize) {}

	T*		New		()			{ return new(cObjectPoolBase::Alloc()) T(); }
	void	Delete	(T* pObj)	{ pObj->~T(); Free(pObj); }
};

//#############################################################################
// inline implementation
//#############################################################################
///////////////////////////////////////////////////////////////////////////////
// Alloc
///////////////////////////////////////////////////////////////////////////////
inline void* cObjectPoolBase::Alloc()
{
	if(! mpNextFree)
		AllocNewChunk();

	cLink* pRtn = mpNextFree;
	mpNextFree = mpNextFree->mpNext;
	return pRtn;
}

///////////////////////////////////////////////////////////////////////////////
// Free
///////////////////////////////////////////////////////////////////////////////
inline void cObjectPoolBase::Free(void* pObj)
{
	cLink* pNew = (cLink*)pObj;
	pNew->mpNext = mpNextFree;
	mpNextFree = pNew;
}

///////////////////////////////////////////////////////////////////////////////
// Clear
///////////////////////////////////////////////////////////////////////////////
inline void cObjectPoolBase::Clear()
{
	mBlockList.Clear();
	mpNextFree = 0;
}


#endif //__OBJECTPOOL_H

