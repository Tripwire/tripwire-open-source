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
// errorbucket.h
//

#ifndef __ERRORBUCKET_H
#define __ERRORBUCKET_H

#ifndef __TYPES_H
#include "types.h"
#endif
#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

class eError;

///////////////////////////////////////////////////////////////////////////////
// cErrorBucket -- contains an interface that handles error reporting, and
//      contains a link to a child bucket. Each concrete implementation of the
//      cErrorBucket interface will perform its own specific task related to the
//      error's occurence (print to stderr, store in a queue, etc) and then forward
//      the error on to its child link. The parent bucket does not own the destruction
//      of the pointer to the child bucket.
///////////////////////////////////////////////////////////////////////////////
class cErrorBucket
{
public:
    cErrorBucket();
    virtual ~cErrorBucket()
    {
    }

    virtual void AddError(const eError& error);
    // add an error to the bucket

    cErrorBucket* GetChild();
    // returns the bucket that the current bucket is chained to, or
    // NULL if nothing is attached to it.
    cErrorBucket* SetChild(cErrorBucket* pNewChild);
    // sets the child link of this bucket; returns the old link value

protected:
    virtual void HandleError(const eError& error) = 0;
    // override this to implement error handling functionality specific to
    // the derived class
    cErrorBucket* mpChild;
};

//#############################################################################
// inline implementation

///////////////////
// cErrorBucket
///////////////////
inline cErrorBucket::cErrorBucket() : mpChild(0)
{
}

inline cErrorBucket* cErrorBucket::GetChild()
{
    return mpChild;
}

inline cErrorBucket* cErrorBucket::SetChild(cErrorBucket* pNewChild)
{
    cErrorBucket* pOldChild = mpChild;
    mpChild                 = pNewChild;
    return pOldChild;
}

#endif
