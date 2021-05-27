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
// fcopropcalc.h
//
// iFCOPropCalc -- abstract interface for object that calculates fco properties
#ifndef __FCOPROPCALC_H
#define __FCOPROPCALC_H

class cFCOPropVector;
class iFCOVisitor;
class iFCOSet;
class cErrorBucket;

// cErrorBucket error numbers...
/*  // the prop calculator owns all error numbers from 200-299
    enum ErrorNum
    {
        ERR_STAT_FAIL           = 200,
        ERR_GET_ACL_FAIL        = 201,
        ERR_NUM_ERRORS
    };
*/


class iFCOPropCalc
{
public:
    virtual void                  SetPropVector(const cFCOPropVector& pv) = 0;
    virtual const cFCOPropVector& GetPropVector() const                   = 0;
    // gets and sets the property vector that indicates what properties the
    // calculator will evaluate for each fco that it visits

    virtual iFCOVisitor*       GetVisitor()       = 0;
    virtual const iFCOVisitor* GetVisitor() const = 0;
    // returns 'this' if the class that implements this interface also implements
    // iFCOVisitor, which will almost always be the case. Otherwise, these methods
    // should return NULL

    virtual void                SetErrorBucket(cErrorBucket* pBucket) = 0;
    virtual const cErrorBucket* GetErrorBucket() const                = 0;

    // this enum lists all the ways that the prop calc can behave when calculating a property
    // that already has a valid value in an fco. The default behavior is PROP_LEAVE
    enum CollisionAction
    {
        PROP_OVERWRITE, // overwite the existing value with a newly calculated one
        PROP_LEAVE,     // leave the existing property value (default)
        PROP_NUMITEMS
    };
    virtual CollisionAction GetCollisionAction() const            = 0;
    virtual void            SetCollisionAction(CollisionAction a) = 0;
    // these methods get and set the collision action for the property calculator


    enum CalcFlags
    {
        DO_NOT_MODIFY_PROPERTIES = 0x00000001, // reset any properties that may have been altered due to measurement
        DIRECT_IO                = 0x00000002  // use direct i/o when scanning files
    };

    virtual int  GetCalcFlags() const = 0;
    virtual void SetCalcFlags(int i)  = 0;
    // any calculation flags needed for calculation.

    virtual ~iFCOPropCalc()
    {
    }
};


#endif //__FCOPROPCALC_H
