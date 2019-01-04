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
// twfactory.h
//
//  iTWFactory -- an interface for classes that use the abstract factory pattern
//      to provide the appropriate instances of classes
#ifndef __TWFACTORY_H
#define __TWFACTORY_H

#ifndef __SREFCOUNTOBJ_H
#include "core/srefcountobj.h"
#endif

class iFCOPropCalc;
class iFCOSpec;
class iFCOSpecHelper;
class iFCOPropDisplayer;
class iParserGenreUtil;
class cFCOSetWS;
class iFCODataSourceIter;
class iFCONameInfo;
class cFCOPropVector;
class iFCONameTranslator;

// NOTE -- all the pointers returned from this factory should be deleted after usage
class iTWFactory
{
public:
    static void SetFactory(iTWFactory* pFact);
    // sets the single instance of the tripwire factory to what was passed it. The caller
    // is responsible for deleting the pointer. (probably just created on the stack.)
    static iTWFactory* GetInstance();
    // returns the global instance of the tripwire factory

    virtual iFCOPropCalc* CreatePropCalc() const = 0;
    // returns an object that can calculate properties for fcos produced by the
    // DataSource.
    virtual iFCOSpec* CreateSpec(const TSTRING& name, iFCOSpecHelper* pHelper = NULL) const = 0;
    // creates an appropriate FCOSpec and assignes it the name and helper specified.

    virtual iFCOPropDisplayer* CreatePropDisplayer() const = 0;
    // returns a pointer to an object which knows how to display human-readable representations of an FCO's properties

    virtual iSerRefCountObj::CreateFunc GetCreateFunc() const = 0;
    // returns a pointer to a function that creates instances of the currently appropriate FCO.

    virtual iParserGenreUtil* CreateParserGenreUtil() const = 0;
    // returns a pointer to an object that handles all genre-dependant parser functions for the currently appropriate FCO.

    virtual iFCODataSourceIter* CreateDataSourceIter() const = 0;

    virtual iFCONameInfo* GetNameInfo() const = 0;
    // returns a pointer to a class that provides characteristics of the FCO's names in the
    // current genre. Since this class will have no data, the object returned should be declared
    // statically in the cpp file that implements this method, so:
    // NOTE -- do _not_ delete the pointer you get back from calling this method!!

    virtual cFCOPropVector GetLooseDirMask() const = 0;
    // ok, it sucks that this interface is polluted with this method, but I can't really come up with a
    // better place for this.
    //
    // the property vector returned from this method indicates what properties should be _ignored_ for
    // an integrity check run in "loose directory" mode. This mask will be stripped off of all fcos comapred
    // where both return CAP_CAN_HAVE_CHILDREN from iFCO::GetCaps()

    virtual iFCONameTranslator* GetNameTranslator() const = 0;

    virtual ~iTWFactory()
    {
    }


private:
    static iTWFactory* mpInstance;
};

#endif //__TWFACTORY_H
