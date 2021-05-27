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
// growheap.h
//
#ifndef __GROWHEAP_H
#define __GROWHEAP_H

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

//-----------------------------------------------------------------------------
// cGrowHeap -- a heap that can grow but never shrink
//      All items alloced should be well below growBy in size
//-----------------------------------------------------------------------------
class cGrowHeap_i;

class cGrowHeap
{
public:
    cGrowHeap(size_t initialSize, size_t growby, const TCHAR* name);
    // creates a heap that is initially initialSize big, and increases the
    // size by growBy every time there is no more room.
    // initialSize and growby must be a multiple of BYTE_ALIGN
    ~cGrowHeap();

    void* Malloc(size_t size);
    void  Clear();
    // resets the grow heap's state.
    size_t TotalMemUsage() const;
    // returns the total memory usage of this heap
private:
    cGrowHeap_i* mpData;
};


#endif
